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
# Axis conversion Blender → DirectX
# Blender:  X +right, Y +forward, Z +up
# DirectX:  X +right, Y +up,      Z +forward
# We swap Y and Z.
# ------------------------------------------------------------

def flip_yz(v):
    return (v.x, v.z, v.y)

# ------------------------------------------------------------
# Export a single mesh object → returns C++ block string
# ------------------------------------------------------------

def process_mesh_object(obj):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(depsgraph)
    mesh = eval_obj.to_mesh()

    mesh.calc_loop_triangles()
    mesh.calc_tangents()

    # Bounding box in Blender space
    xs = [v.co.x for v in mesh.vertices]
    ys = [v.co.y for v in mesh.vertices]
    zs = [v.co.z for v in mesh.vertices]

    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    minz, maxz = min(zs), max(zs)

    uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None

    vertex_dict = {}
    unique_vertices = []
    indices = []

    for tri in mesh.loop_triangles:
        for li in tri.loops:
            loop = mesh.loops[li]
            v = mesh.vertices[loop.vertex_index]

            # --------------------------------------------------------
            # Position (flip Y/Z)
            # --------------------------------------------------------
            px, pz, py = flip_yz(v.co)

            px = float_to_byte_pos(px, minx, maxx)
            py = float_to_byte_pos(py, miny, maxy)
            pz = float_to_byte_pos(pz, minz, maxz)

            # --------------------------------------------------------
            # UV
            # --------------------------------------------------------
            if uv_layer:
                uv = uv_layer[li].uv
                uvx = float_to_byte_uv(uv.x)
                uvy = float_to_byte_uv(uv.y)
            else:
                uvx = uvy = 0

            # --------------------------------------------------------
            # Normal (flip Y/Z)
            # --------------------------------------------------------
            n = loop.normal
            nx, nz, ny = flip_yz(n)
            nx = float_to_byte_norm(nx)
            ny = float_to_byte_norm(ny)
            nz = float_to_byte_norm(nz)

            # --------------------------------------------------------
            # Tangent (flip Y/Z)
            # --------------------------------------------------------
            t = loop.tangent
            tx, tz, ty = flip_yz(t)
            tx = float_to_byte_norm(tx)
            ty = float_to_byte_norm(ty)
            tz = float_to_byte_norm(tz)

            # --------------------------------------------------------
            # Bitangent (reconstructed using handedness)
            # Blender tangent.w = handedness (+1 or -1)
            # B = cross(N, T) * w
            # --------------------------------------------------------
            w = loop.bitangent_sign
            # We don't store bitangent, but we need it for TBN correctness.
            # If you ever want to store it, flip Y/Z here too.

            pad = 0

            vt = (px, pz, py, uvx, nx, ny, nz, uvy, tx, ty, tz, pad)

            if vt in vertex_dict:
                idx = vertex_dict[vt]
            else:
                idx = len(unique_vertices)
                unique_vertices.append(vt)
                vertex_dict[vt] = idx

            indices.append(idx)

    eval_obj.to_mesh_clear()

    # --------------------------------------------------------
    # Build C++ output
    # --------------------------------------------------------

    safe_name = obj.name.replace(" ", "_")
    cpp = []

    cpp.append("// ------------------------------------------------------------")
    cpp.append(f"// Object: {obj.name}")
    cpp.append("// ------------------------------------------------------------\n")

    # Note: PosMin/PosMax also flipped Y/Z
    cpp.append(f"static const float g_{safe_name}_PosMin[3] = {{ {minx}f, {minz}f, {miny}f }};")
    cpp.append(f"static const float g_{safe_name}_PosMax[3] = {{ {maxx}f, {maxz}f, {maxy}f }};\n")

    cpp.append(f"static const uint32_t g_{safe_name}_VertexCount = {len(unique_vertices)};")
    cpp.append(f"static const uint32_t g_{safe_name}_IndexCount  = {len(indices)};\n")

    cpp.append(f"static const uint8_t g_{safe_name}_Vertices[] = {{")
    for vt in unique_vertices:
        cpp.append("    " + ", ".join(str(x) for x in vt) + ",")
    cpp.append("};\n")

    cpp.append(f"static const uint16_t g_{safe_name}_Indices[] = {{")
    row = []
    for i, idx in enumerate(indices):
        row.append(str(idx))
        if (i + 1) % 12 == 0:
            cpp.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        cpp.append("    " + ", ".join(row) + ",")
    cpp.append("};\n\n")

    return "\n".join(cpp)

# ------------------------------------------------------------
# Main Exporter
# ------------------------------------------------------------

def export_all_meshes_header(filepath):
    with open(filepath, "w") as f:
        f.write("#pragma once\n#include <stdint.h>\n\n")
        for obj in bpy.context.scene.objects:
            if obj.type == 'MESH':
                f.write(process_mesh_object(obj))
    print(f"Exported header: {filepath}")

# ------------------------------------------------------------
# Run
# ------------------------------------------------------------

out_path = os.path.join(os.path.dirname(bpy.data.filepath), "all_meshes_export.h")
export_all_meshes_header(out_path)
