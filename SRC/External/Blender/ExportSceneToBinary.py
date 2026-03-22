import bpy
import struct
import os
import mathutils

# ----------------------------
# Helpers
# ----------------------------

def clamp_byte(v):
    return max(0, min(255, int(v)))

def float_to_byte_norm(v):
    # v is -1..1 → 0..255
    return clamp_byte((v * 0.5 + 0.5) * 255.0)

def float_to_byte_pos(v, minv, maxv):
    # Normalize position to 0..1 range, then to byte
    if maxv - minv < 1e-6:
        return 0
    return clamp_byte((v - minv) / (maxv - minv) * 255.0)

def float_to_byte_uv(v):
    # UVs assumed 0..1
    return clamp_byte(v * 255.0)

# ----------------------------
# Export Function
# ----------------------------

def export_packed_mesh(obj, filepath):
    mesh = obj.to_mesh()
    mesh.calc_normals_split()

    # Compute bounding box for position normalization
    xs = [v.co.x for v in mesh.vertices]
    ys = [v.co.y for v in mesh.vertices]
    zs = [v.co.z for v in mesh.vertices]

    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    minz, maxz = min(zs), max(zs)

    # Prepare vertex data
    loop_normals = [ln for ln in mesh.loops]
    uv_layer = mesh.uv_layers.active.data if mesh.uv_layers.active else None

    vertices_out = []
    indices_out = []

    # Blender uses loops → we flatten to per-loop vertices
    for poly in mesh.polygons:
        for li in poly.loop_indices:
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
            n = loop.normal
            nx = float_to_byte_norm(n.x)
            ny = float_to_byte_norm(n.y)
            nz = float_to_byte_norm(n.z)

            # Pack into your 8‑byte struct
            vertex_bytes = struct.pack(
                "BBBBBBBB",
                px, py, pz,
                uvx,
                nx, ny, nz,
                uvy
            )

            vertices_out.append(vertex_bytes)
            indices_out.append(len(indices_out))  # sequential 16‑bit indices

    # Write binary file
    with open(filepath, "wb") as f:
        # Write vertex count and index count (optional)
        f.write(struct.pack("I", len(vertices_out)))
        f.write(struct.pack("I", len(indices_out)))

        # Write vertices
        for vb in vertices_out:
            f.write(vb)

        # Write 16‑bit indices
        for idx in indices_out:
            f.write(struct.pack("H", idx))

    print(f"Exported {len(vertices_out)} vertices and {len(indices_out)} indices to {filepath}")

# ----------------------------
# Run Export
# ----------------------------

obj = bpy.context.active_object
if obj and obj.type == 'MESH':
    out_path = os.path.join(os.path.dirname(bpy.data.filepath), "mesh.bin")
    export_packed_mesh(obj, out_path)
else:
    print("Select a mesh object first.")
