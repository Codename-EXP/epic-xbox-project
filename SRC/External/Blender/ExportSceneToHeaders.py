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
# Export a single mesh object → returns dict of C++ strings
# ------------------------------------------------------------

def process_mesh_object(obj):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(depsgraph)
    mesh = eval_obj.to_mesh()

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

    # Build vertices + indices
    for poly in mesh.polygons:
        for li in poly.loop_indices:
            loop = mesh.loops[li]
            v = mesh.vertices[loop.vertex_index]

            px = float_to_byte_pos(v.co.x, minx, maxx)
            py = float_to_byte_pos(v.co.y, miny, maxy)
            pz = float_to_byte_pos(v.co.z, minz, maxz)

            if uv_layer:
                uv = uv_layer[li].uv
                uvx = float_to_byte_uv(uv.x)
                uvy = float_to_byte_uv(uv.y)
            else:
                uvx = 0
                uvy = 0

            n = loop.normal
            nx = float_to_byte_norm(n.x)
            ny = float_to_byte_norm(n.y)
            nz = float_to_byte_norm(n.z)

            vt = (px, py, pz, uvx, nx, ny, nz, uvy)

            if vt in vertex_dict:
                idx = vertex_dict[vt]
            else:
                idx = len(unique_vertices)
                unique_vertices.append(vt)
                vertex_dict[vt] = idx

            indices.append(idx)

    eval_obj.to_mesh_clear()

    # --------------------------------------------------------
    # Build C++ code for this object
    # --------------------------------------------------------

    safe_name = obj.name.replace(" ", "_")

    cpp = []

    cpp.append(f"// ------------------------------------------------------------")
    cpp.append(f"// Object: {obj.name}")
    cpp.append(f"// ------------------------------------------------------------\n")

    cpp.append(f"static const float g_{safe_name}_PosMin[3] = {{ {minx}f, {miny}f, {minz}f }};")
    cpp.append(f"static const float g_{safe_name}_PosMax[3] = {{ {maxx}f, {maxy}f, {maxz}f }};\n")

    cpp.append(f"static const uint32_t g_{safe_name}_VertexCount = {len(unique_vertices)};")
    cpp.append(f"static const uint32_t g_{safe_name}_IndexCount  = {len(indices)};\n")

    # Vertex buffer
    cpp.append(f"static const uint8_t g_{safe_name}_Vertices[] = {{")
    for (px, py, pz, uvx, nx, ny, nz, uvy) in unique_vertices:
        cpp.append(f"    {px}, {py}, {pz}, {uvx}, {nx}, {ny}, {nz}, {uvy},")
    cpp.append("};\n")

    # Index buffer
    cpp.append(f"static const uint16_t g_{safe_name}_Indices[] = {{")
    for i in range(0, len(indices), 12):
        chunk = indices[i:i+12]
        cpp.append("    " + ", ".join(str(x) for x in chunk) + ",")
    cpp.append("};\n\n")

    return "\n".join(cpp)

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
