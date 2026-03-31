import bpy
import os

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

def clamp_byte(v):
    return max(0, min(255, int(v)))

def float_to_byte_norm(v):
    return clamp_byte((v * 0.5 + 0.5) * 255.0)

def float_to_byte_pos(v, minv, maxv):
    if maxv - minv < 1e-6:
        return 0
    return clamp_byte((v - minv) / (maxv - minv) * 255.0)

def float_to_byte_uv(v):
    return clamp_byte(v * 255.0)

# ------------------------------------------------------------
# Export a single mesh object → returns C++ block string
# ------------------------------------------------------------

def process_mesh_object(obj):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(depsgraph)
    mesh = eval_obj.to_mesh()

    # Ensure loop triangles, split normals, and tangents exist
    mesh.calc_loop_triangles()
    #mesh.calc_normals_split()
    mesh.calc_tangents()

    # Bounding box ranges
    xs = [v.co.x for v in mesh.vertices]
    ys = [v.co.y for v in mesh.vertices]
    zs = [v.co.z for v in mesh.vertices]

    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    minz, maxz = min(zs), max(zs)

    uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None

    # Deduplication
    vertex_dict = {}
    unique_vertices = []
    indices = []

    # Iterate triangulated loop triangles
    for tri in mesh.loop_triangles:
        for li in tri.loops:
            loop = mesh.loops[li]
            v = mesh.vertices[loop.vertex_index]

            # Position → 3 bytes
            px = float_to_byte_pos(v.co.x, minx, maxx)
            py = float_to_byte_pos(v.co.y, miny, maxy)
            pz = float_to_byte_pos(v.co.z, minz, maxz)

            # UV → 2 bytes
            if uv_layer:
                uv = uv_layer[li].uv
                uvx = float_to_byte_uv(uv.x)
                uvy = float_to_byte_uv(uv.y)
            else:
                uvx = 0
                uvy = 0

            # Normal → 3 bytes
            ln = getattr(loop, "normal", None)
            n = ln if ln is not None else v.normal
            nx = float_to_byte_norm(n.x)
            ny = float_to_byte_norm(n.y)
            nz = float_to_byte_norm(n.z)

            # Tangent → 3 bytes
            t = loop.tangent
            tx = float_to_byte_norm(t.x)
            ty = float_to_byte_norm(t.y)
            tz = float_to_byte_norm(t.z)

            # Padding → 1 byte
            pad = 0

            # 12‑byte vertex tuple
            vt = (px, pz, py, uvx, nx, ny, nz, uvy, tx, ty, tz, pad)

            # Deduplicate
            if vt in vertex_dict:
                idx = vertex_dict[vt]
            else:
                idx = len(unique_vertices)
                unique_vertices.append(vt)
                vertex_dict[vt] = idx

            indices.append(idx)

    # Clean up evaluated mesh
    eval_obj.to_mesh_clear()

    # --------------------------------------------------------
    # Build C++ code for this object
    # --------------------------------------------------------

    safe_name = obj.name.replace(" ", "_")

    cpp_lines = []

    cpp_lines.append("// ------------------------------------------------------------")
    cpp_lines.append(f"// Object: {obj.name}")
    cpp_lines.append("// ------------------------------------------------------------\n")

    cpp_lines.append(f"static const float g_{safe_name}_PosMin[3] = {{ {minx}f, {minz}f, {miny}f }};")
    cpp_lines.append(f"static const float g_{safe_name}_PosMax[3] = {{ {maxx}f, {maxz}f, {maxy}f }};\n")

    cpp_lines.append(f"static const uint32_t g_{safe_name}_VertexCount = {len(unique_vertices)};")
    cpp_lines.append(f"static const uint32_t g_{safe_name}_IndexCount  = {len(indices)};\n")

    # Vertex buffer
    cpp_lines.append(f"static const uint8_t g_{safe_name}_Vertices[] = {{")
    for (px, pz, py, uvx, nx, ny, nz, uvy, tx, ty, tz, pad) in unique_vertices:
        cpp_lines.append(
            f"    {px}, {pz}, {py}, {uvx}, {nx}, {ny}, {nz}, {uvy}, {tx}, {ty}, {tz}, {pad},"
        )
    cpp_lines.append("};\n")

    # Index buffer
    cpp_lines.append(f"static const uint16_t g_{safe_name}_Indices[] = {{")
    row = []
    for i, idx in enumerate(indices):
        row.append(str(idx))
        if (i + 1) % 12 == 0:
            cpp_lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        cpp_lines.append("    " + ", ".join(row) + ",")
    cpp_lines.append("};\n\n")

    return "\n".join(cpp_lines)

# ------------------------------------------------------------
# Main Exporter — iterates ALL mesh objects
# ------------------------------------------------------------

def export_all_meshes_header(filepath):
    with open(filepath, "w") as f:
        f.write("#pragma once\n#include <stdint.h>\n\n")

        for obj in bpy.context.scene.objects:
            if obj.type == 'MESH':
                block = process_mesh_object(obj)
                f.write(block)

    print(f"Exported header: {filepath}")

# ------------------------------------------------------------
# Run
# ------------------------------------------------------------

out_path = os.path.join(os.path.dirname(bpy.data.filepath), "all_meshes_export.h")
export_all_meshes_header(out_path)
