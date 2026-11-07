"""Builds a house for The Last Street in Blender, entirely from code.

Run from the project folder:

    /Applications/Blender.app/Contents/MacOS/Blender --background \
        --python tools/blender/build_house.py -- --house lind

It saves assets/blender/<house>.blend and preview renders in
assets/renders/<house>/.

The house is built in separate collections so the game can open it up
from above (design document, "Seeing inside houses"):

    Shell          foundation, porch, door
    Walls_Full     log walls at full height
    Walls_Cutaway  the same walls cut low, for looking inside
    Windows        frames, glass, curtains
    Roof           roof, gables, chimney top, loft floor
    Interior       floor, partitions, stove, furniture

Sizes are real metres. X runs along the front of the house, Y from the
front (negative) to the back, Z up. The front door faces -Y, the street.
"""

import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Euler, Matrix, Vector

# ---------------------------------------------------------------------------
# House settings. Neighbours' houses will be new entries in this table.
# ---------------------------------------------------------------------------

HOUSES = {
    "lind": {
        "width": 10.0,             # along the street
        "depth": 8.0,
        "foundation": 0.6,         # stone plinth height
        "log_diameter": 0.27,
        "log_rows": 11,
        "corner_overhang": 0.28,   # log ends sticking out past the corners
        "roof_pitch_deg": 36.0,
        "roof_overhang": 0.45,
        "gable_overhang": 0.4,
        "chimney_x": -1.0,
        "chimney_y": -0.45,        # just in front of the ridge, as in the reference
        "front_windows": [-3.5, -2.0, 2.0, 3.5],
        "back_windows": [-3.5, -2.0, 2.0, 3.5],
        "side_windows": [-1.3, 1.3],
        "door_x": 0.0,
        "seed": 1900,
    },
}

WINDOW_W = 0.9
WINDOW_H = 1.25
WINDOW_SILL = 0.85   # above the floor
DOOR_W = 1.0
DOOR_H = 2.05
CUTAWAY_ROWS = 5     # log rows kept in the cutaway walls

# ---------------------------------------------------------------------------
# Materials (procedural, so nothing has to be downloaded)
# ---------------------------------------------------------------------------


def _new_material(name):
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    out = nodes.new("ShaderNodeOutputMaterial")
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    mat.node_tree.links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    return mat, nodes, mat.node_tree.links, bsdf


def _ramp(nodes, c1, c2, pos1=0.3, pos2=0.7):
    ramp = nodes.new("ShaderNodeValToRGB")
    ramp.color_ramp.elements[0].position = pos1
    ramp.color_ramp.elements[0].color = (*c1, 1.0)
    ramp.color_ramp.elements[1].position = pos2
    ramp.color_ramp.elements[1].color = (*c2, 1.0)
    return ramp


def _coords(nodes, links, scale, projection="object"):
    """Texture coordinates. 'wall' wraps a texture around vertical faces facing any
    way (bricks, stones); 'roof' lays it along sloped roofs."""
    tc = nodes.new("ShaderNodeTexCoord")
    mapping = nodes.new("ShaderNodeMapping")
    mapping.inputs["Scale"].default_value = scale
    if projection == "object":
        links.new(tc.outputs["Object"], mapping.inputs["Vector"])
        return mapping
    sep = nodes.new("ShaderNodeSeparateXYZ")
    links.new(tc.outputs["Object"], sep.inputs["Vector"])
    add = nodes.new("ShaderNodeMath")
    add.operation = "ADD"
    comb = nodes.new("ShaderNodeCombineXYZ")
    if projection == "wall":
        links.new(sep.outputs["X"], add.inputs[0])
        links.new(sep.outputs["Y"], add.inputs[1])
        links.new(add.outputs["Value"], comb.inputs["X"])
        links.new(sep.outputs["Z"], comb.inputs["Y"])
    else:  # roof
        links.new(sep.outputs["X"], comb.inputs["X"])
        links.new(sep.outputs["Y"], add.inputs[0])
        links.new(sep.outputs["Z"], add.inputs[1])
        links.new(add.outputs["Value"], comb.inputs["Y"])
    links.new(comb.outputs["Vector"], mapping.inputs["Vector"])
    return mapping


def _bump(nodes, links, bsdf, height_socket, strength):
    bump = nodes.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = strength
    links.new(height_socket, bump.inputs["Height"])
    links.new(bump.outputs["Normal"], bsdf.inputs["Normal"])


def mat_noise(name, c1, c2, scale=(1, 1, 1), noise_scale=4.0, rough=0.8, bump=0.2, detail=8.0):
    """Mottled surface: weathered wood, stone, paint."""
    mat, nodes, links, bsdf = _new_material(name)
    mapping = _coords(nodes, links, scale)
    noise = nodes.new("ShaderNodeTexNoise")
    noise.inputs["Scale"].default_value = noise_scale
    noise.inputs["Detail"].default_value = detail
    links.new(mapping.outputs["Vector"], noise.inputs["Vector"])
    ramp = _ramp(nodes, c1, c2)
    links.new(noise.outputs["Fac"], ramp.inputs["Fac"])
    links.new(ramp.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = rough
    if bump > 0:
        _bump(nodes, links, bsdf, noise.outputs["Fac"], bump)
    return mat


def mat_stone(name, c1, c2, mortar, rough=0.9, bump=0.8):
    """Fieldstone: irregular cells with dark gaps between them."""
    mat, nodes, links, bsdf = _new_material(name)
    # Voronoi cells are 3D, so they wrap every face without stretching.
    mapping = _coords(nodes, links, (1, 1, 1))
    cells = nodes.new("ShaderNodeTexVoronoi")
    cells.feature = "DISTANCE_TO_EDGE"
    cells.inputs["Scale"].default_value = 3.2
    cells.inputs["Randomness"].default_value = 0.8
    links.new(mapping.outputs["Vector"], cells.inputs["Vector"])
    colour = nodes.new("ShaderNodeTexVoronoi")
    colour.inputs["Scale"].default_value = 3.2
    colour.inputs["Randomness"].default_value = 0.8
    links.new(mapping.outputs["Vector"], colour.inputs["Vector"])
    tone = _ramp(nodes, c1, c2, 0.0, 1.0)
    links.new(colour.outputs["Color"], tone.inputs["Fac"])
    gap = _ramp(nodes, mortar, (1, 1, 1), 0.05, 0.14)
    links.new(cells.outputs["Distance"], gap.inputs["Fac"])
    mix = nodes.new("ShaderNodeMix")
    mix.data_type = "RGBA"
    mix.blend_type = "MULTIPLY"
    mix.inputs["Factor"].default_value = 1.0
    links.new(tone.outputs["Color"], mix.inputs["A"])
    links.new(gap.outputs["Color"], mix.inputs["B"])
    links.new(mix.outputs["Result"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = rough
    _bump(nodes, links, bsdf, cells.outputs["Distance"], bump)
    return mat


def mat_bands(name, c1, c2, direction, scale=1.0, rough=0.8, bump=0.25):
    """Boards and planks: bands along one axis, roughened with noise."""
    mat, nodes, links, bsdf = _new_material(name)
    mapping = _coords(nodes, links, (1, 1, 1))
    wave = nodes.new("ShaderNodeTexWave")
    wave.wave_type = "BANDS"
    wave.bands_direction = direction
    wave.inputs["Scale"].default_value = scale
    wave.inputs["Distortion"].default_value = 2.0
    wave.inputs["Detail"].default_value = 6.0
    links.new(mapping.outputs["Vector"], wave.inputs["Vector"])
    ramp = _ramp(nodes, c1, c2, 0.2, 0.8)
    links.new(wave.outputs["Fac"], ramp.inputs["Fac"])
    links.new(ramp.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = rough
    _bump(nodes, links, bsdf, wave.outputs["Fac"], bump)
    return mat


def mat_brick(name, c1, c2, mortar, brick_w, row_h, rough=0.85, bump=0.4, axis_scale=(1, 1, 1),
              projection="wall"):
    """Bricks, tiles and roof shingles."""
    mat, nodes, links, bsdf = _new_material(name)
    mapping = _coords(nodes, links, axis_scale, projection)
    brick = nodes.new("ShaderNodeTexBrick")
    brick.inputs["Scale"].default_value = 1.0
    brick.inputs["Brick Width"].default_value = brick_w
    brick.inputs["Row Height"].default_value = row_h
    brick.inputs["Mortar Size"].default_value = 0.012
    brick.inputs["Color1"].default_value = (*c1, 1)
    brick.inputs["Color2"].default_value = (*c2, 1)
    brick.inputs["Mortar"].default_value = (*mortar, 1)
    links.new(mapping.outputs["Vector"], brick.inputs["Vector"])
    links.new(brick.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = rough
    _bump(nodes, links, bsdf, brick.outputs["Fac"], bump)
    return mat


TEXTURES = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "assets", "textures"))


def _image(nodes, path, colour, mapping, links):
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(path, check_existing=True)
    tex.image.colorspace_settings.name = "sRGB" if colour else "Non-Color"
    # Box projection wraps the photo around any shape without UV maps.
    tex.projection = "BOX"
    tex.projection_blend = 0.25
    links.new(mapping.outputs["Vector"], tex.inputs["Vector"])
    return tex


def _add_weathering(nodes, links, colour, kind):
    """Darkens and greens a surface the way weather does, and deepens creases.

    kind 'wall': damp and grime creeping up from the ground.
    kind 'roof': moss patches and darker shingles towards the eaves.
    Every kind also gets ambient occlusion, so corners and gaps go darker."""
    tc = nodes.new("ShaderNodeTexCoord")
    sep = nodes.new("ShaderNodeSeparateXYZ")
    links.new(tc.outputs["Object"], sep.inputs["Vector"])
    patches = nodes.new("ShaderNodeTexNoise")
    patches.inputs["Scale"].default_value = 1.4
    patches.inputs["Detail"].default_value = 6.0
    links.new(tc.outputs["Object"], patches.inputs["Vector"])

    if kind == "wall":
        # 1 at ground level, fading to 0 about a metre up.
        height = nodes.new("ShaderNodeMapRange")
        height.inputs["From Min"].default_value = 0.4
        height.inputs["From Max"].default_value = 1.6
        height.inputs["To Min"].default_value = 1.0
        height.inputs["To Max"].default_value = 0.0
        links.new(sep.outputs["Z"], height.inputs["Value"])
        amount = nodes.new("ShaderNodeMath")
        amount.operation = "MULTIPLY"
        links.new(height.outputs["Result"], amount.inputs[0])
        links.new(patches.outputs["Fac"], amount.inputs[1])
        grime = nodes.new("ShaderNodeMix")
        grime.data_type = "RGBA"
        grime.blend_type = "MULTIPLY"
        grime.inputs["B"].default_value = (0.42, 0.45, 0.36, 1)
        links.new(amount.outputs["Value"], grime.inputs["Factor"])
        links.new(colour, grime.inputs["A"])
        colour = grime.outputs["Result"]
    elif kind == "roof":
        moss_mask = _ramp(nodes, (0, 0, 0), (1, 1, 1), 0.52, 0.68)
        links.new(patches.outputs["Fac"], moss_mask.inputs["Fac"])
        moss = nodes.new("ShaderNodeMix")
        moss.data_type = "RGBA"
        moss.inputs["B"].default_value = (0.11, 0.14, 0.05, 1)
        links.new(moss_mask.outputs["Color"], moss.inputs["Factor"])
        links.new(colour, moss.inputs["A"])
        colour = moss.outputs["Result"]

    ao = nodes.new("ShaderNodeAmbientOcclusion")
    ao.inputs["Distance"].default_value = 0.35
    links.new(colour, ao.inputs["Color"])
    soften = nodes.new("ShaderNodeMix")
    soften.data_type = "RGBA"
    soften.inputs["Factor"].default_value = 0.65
    links.new(colour, soften.inputs["A"])
    links.new(ao.outputs["Color"], soften.inputs["B"])
    return soften.outputs["Result"]


def mat_photo(name, texture, size_m, ramp=None, value=1.0, saturation=1.0, rough_scale=1.0, normal=1.0,
              weather=None):
    """A Poly Haven photo texture at its real size.

    ramp: optional (dark, light) colours to repaint it, keeping only its light
    and shade, for painted wood in our own colours.
    weather: 'wall' or 'roof' grime (see _add_weathering); any other value
    still adds soft shadows in creases."""
    folder = os.path.join(TEXTURES, texture)
    if not os.path.isdir(folder):
        return None
    mat, nodes, links, bsdf = _new_material(name)
    mapping = _coords(nodes, links, (1 / size_m,) * 3)
    diff = _image(nodes, os.path.join(folder, f"{texture}_diff_2k.jpg"), True, mapping, links)
    rough = _image(nodes, os.path.join(folder, f"{texture}_rough_2k.jpg"), False, mapping, links)
    nor = _image(nodes, os.path.join(folder, f"{texture}_nor_gl_2k.jpg"), False, mapping, links)

    colour = diff.outputs["Color"]
    if ramp:
        bw = nodes.new("ShaderNodeRGBToBW")
        links.new(colour, bw.inputs["Color"])
        r = _ramp(nodes, ramp[0], ramp[1], 0.15, 0.75)
        links.new(bw.outputs["Val"], r.inputs["Fac"])
        colour = r.outputs["Color"]
    hsv = nodes.new("ShaderNodeHueSaturation")
    hsv.inputs["Saturation"].default_value = saturation
    hsv.inputs["Value"].default_value = value
    links.new(colour, hsv.inputs["Color"])
    links.new(_add_weathering(nodes, links, hsv.outputs["Color"], weather), bsdf.inputs["Base Color"])

    rmul = nodes.new("ShaderNodeMath")
    rmul.operation = "MULTIPLY"
    rmul.inputs[1].default_value = rough_scale
    links.new(rough.outputs["Color"], rmul.inputs[0])
    links.new(rmul.outputs["Value"], bsdf.inputs["Roughness"])

    nmap = nodes.new("ShaderNodeNormalMap")
    nmap.inputs["Strength"].default_value = normal
    links.new(nor.outputs["Color"], nmap.inputs["Color"])
    links.new(nmap.outputs["Normal"], bsdf.inputs["Normal"])
    return mat


def mat_plain(name, color, rough=0.6, alpha=1.0, metallic=0.0):
    mat, nodes, links, bsdf = _new_material(name)
    bsdf.inputs["Base Color"].default_value = (*color, 1)
    bsdf.inputs["Roughness"].default_value = rough
    bsdf.inputs["Metallic"].default_value = metallic
    if alpha < 1.0:
        bsdf.inputs["Alpha"].default_value = alpha
        if hasattr(mat, "surface_render_method"):
            mat.surface_render_method = "BLENDED"
    return mat


def build_materials():
    mats = procedural_materials()
    # Photo textures from Poly Haven (CC0) replace the procedural ones where
    # they've been downloaded into assets/textures/.
    photo = {
        "log_x": mat_photo("Logs (photo)", "weathered_brown_planks", 2.4, value=0.75, saturation=0.7,
                           weather="wall"),
        "shingle": mat_photo("Shingles (photo)", "roof_09", 3.2, value=0.7, saturation=0.45, weather="roof"),
        "shingle_dark": mat_photo("Shingles dark (photo)", "roof_09", 3.0, value=0.6, saturation=0.42,
                                  weather="roof"),
        "shingle_light": mat_photo("Shingles light (photo)", "roof_09", 3.4, value=0.85, saturation=0.5,
                                   weather="roof"),
        "stone": mat_photo("Fieldstone (photo)", "rustic_stone_wall", 1.3, value=0.8, saturation=0.6,
                           weather="wall"),
        "brick": mat_photo("Chimney brick (photo)", "red_brick_03", 1.0, value=0.85, weather="soot"),
        "red_boards": mat_photo("Red boards (photo)", "distressed_painted_planks", 1.6,
                                ramp=((0.10, 0.02, 0.015), (0.30, 0.06, 0.045)), weather="paint"),
        "red_door": mat_photo("Red door (photo)", "distressed_painted_planks", 1.2,
                              ramp=((0.11, 0.02, 0.016), (0.32, 0.07, 0.05)), weather="wall"),
        "ochre": mat_photo("Ochre paint (photo)", "distressed_painted_planks", 1.0,
                           ramp=((0.18, 0.10, 0.03), (0.44, 0.29, 0.10)), weather="paint"),
        "trim": mat_photo("Trim (photo)", "weathered_brown_planks", 2.0, value=0.6, saturation=0.6,
                          weather="wall"),
        "floor": mat_photo("Floor (photo)", "old_planks_02", 2.0, value=0.8, weather="floor"),
        "planks": mat_photo("Planks (photo)", "old_planks_02", 1.6, value=0.9, weather="floor"),
    }
    if photo["log_x"]:
        photo["log_y"] = photo["log_x"]
    for key, mat in photo.items():
        if mat is not None:
            mats[key] = mat
    return mats


def procedural_materials():
    return {
        "log_x": mat_noise("Log along X", (0.07, 0.06, 0.05), (0.20, 0.17, 0.14),
                           scale=(0.25, 6.0, 6.0), noise_scale=3.0, bump=0.35),
        "log_y": mat_noise("Log along Y", (0.07, 0.06, 0.05), (0.20, 0.17, 0.14),
                           scale=(6.0, 0.25, 6.0), noise_scale=3.0, bump=0.35),
        "log_end": mat_noise("Log end", (0.13, 0.10, 0.07), (0.22, 0.17, 0.12), noise_scale=12.0),
        "stone": mat_stone("Fieldstone", (0.12, 0.12, 0.11), (0.25, 0.24, 0.22), (0.35, 0.35, 0.35)),
        "shingle": mat_brick("Shingles", (0.13, 0.12, 0.10), (0.21, 0.19, 0.16), (0.05, 0.04, 0.04),
                             brick_w=0.18, row_h=0.22, bump=0.5, projection="roof"),
        "shingle_dark": mat_brick("Shingles dark", (0.09, 0.085, 0.075), (0.15, 0.14, 0.12), (0.03, 0.03, 0.03),
                                  brick_w=0.16, row_h=0.22, bump=0.5, projection="roof"),
        "shingle_light": mat_brick("Shingles light", (0.18, 0.17, 0.15), (0.26, 0.24, 0.21), (0.06, 0.05, 0.05),
                                   brick_w=0.2, row_h=0.22, bump=0.5, projection="roof"),
        "brick": mat_brick("Chimney brick", (0.22, 0.08, 0.05), (0.30, 0.12, 0.08), (0.30, 0.28, 0.26),
                           brick_w=0.22, row_h=0.07, bump=0.3),
        "ochre": mat_noise("Ochre paint", (0.30, 0.18, 0.06), (0.42, 0.27, 0.10), noise_scale=9.0, rough=0.7, bump=0.1),
        "red_boards": mat_bands("Red boards", (0.12, 0.025, 0.02), (0.20, 0.04, 0.03), "Y", scale=6.0),
        "red_door": mat_bands("Red door", (0.13, 0.03, 0.025), (0.21, 0.05, 0.04), "X", scale=7.0),
        "trim": mat_noise("Grey trim", (0.11, 0.10, 0.08), (0.20, 0.18, 0.15), noise_scale=8.0),
        # Partly see-through, so the lace curtains show behind the old glass.
        "glass": mat_plain("Glass", (0.04, 0.05, 0.06), rough=0.06, alpha=0.72),
        "lace": mat_plain("Lace", (0.70, 0.68, 0.63), rough=0.9, alpha=0.5),
        "floor": mat_bands("Floor planks", (0.14, 0.10, 0.06), (0.24, 0.18, 0.11), "Y", scale=9.0),
        "planks": mat_bands("Interior planks", (0.18, 0.13, 0.08), (0.28, 0.21, 0.13), "X", scale=8.0),
        "tile": mat_brick("Stove tiles", (0.86, 0.84, 0.78), (0.80, 0.78, 0.72), (0.60, 0.58, 0.54),
                          brick_w=0.2, row_h=0.2, rough=0.35, bump=0.2),
        "iron": mat_plain("Iron", (0.06, 0.06, 0.06), rough=0.5, metallic=0.8),
        "wool_red": mat_noise("Wool red", (0.35, 0.08, 0.06), (0.50, 0.14, 0.10), noise_scale=30.0, rough=1.0),
        "wool_blue": mat_noise("Wool blue", (0.10, 0.15, 0.25), (0.18, 0.25, 0.38), noise_scale=30.0, rough=1.0),
        "wool_green": mat_noise("Wool green", (0.15, 0.22, 0.12), (0.24, 0.33, 0.18), noise_scale=30.0, rough=1.0),
        "linen": mat_noise("Linen", (0.75, 0.72, 0.64), (0.86, 0.83, 0.76), noise_scale=40.0, rough=1.0),
        "moss": mat_noise("Moss", (0.07, 0.09, 0.03), (0.16, 0.20, 0.07), noise_scale=25.0, rough=1.0, bump=0.6),
        "pottery": mat_noise("Pottery", (0.45, 0.26, 0.16), (0.60, 0.38, 0.24), noise_scale=20.0, rough=0.6),
        "ground": mat_noise("Ground", (0.24, 0.27, 0.15), (0.38, 0.36, 0.22), noise_scale=0.6, rough=1.0, bump=0.3),
        "backdrop": mat_plain("Backdrop", (0.80, 0.80, 0.80), rough=1.0),
    }


# ---------------------------------------------------------------------------
# Mesh building: many pieces go into one mesh per part, so the model stays light
# ---------------------------------------------------------------------------


class MeshBuilder:
    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.materials = []

    def _mat_index(self, mat):
        if mat not in self.materials:
            self.materials.append(mat)
        return self.materials.index(mat)

    @staticmethod
    def _faces_of(result):
        """Every face touching the vertices a bmesh operator just created."""
        return {f for v in result["verts"] for f in v.link_faces}

    def _paint(self, faces, mat, smooth=False):
        idx = self._mat_index(mat)
        for f in faces:
            f.material_index = idx
            f.smooth = smooth and len(f.verts) == 4

    def box(self, center, size, mat, rot=(0, 0, 0)):
        m = Matrix.LocRotScale(Vector(center), Euler(rot).to_quaternion(), Vector(size))
        result = bmesh.ops.create_cube(self.bm, size=1.0, matrix=m)
        self._paint(self._faces_of(result), mat)

    def cylinder(self, center, radius, length, axis, mat, segments=10, radius2=None, cap_mat=None):
        rot = {"X": (0, math.pi / 2, 0), "Y": (math.pi / 2, 0, 0), "Z": (0, 0, 0)}[axis]
        m = Matrix.LocRotScale(Vector(center), Euler(rot).to_quaternion(), Vector((1, 1, 1)))
        result = bmesh.ops.create_cone(self.bm, cap_ends=True, cap_tris=False, segments=segments,
                                       radius1=radius, radius2=radius if radius2 is None else radius2,
                                       depth=length, matrix=m)
        faces = self._faces_of(result)
        self._paint(faces, mat, smooth=True)
        if cap_mat is not None:
            # The round ends are the only faces with more than four corners.
            self._paint([f for f in faces if len(f.verts) > 4], cap_mat)

    def blob(self, center, size, mat, rot=(0, 0, 0)):
        """A squashed ball: moss tufts, stones, sacks."""
        m = Matrix.LocRotScale(Vector(center), Euler(rot).to_quaternion(), Vector(size))
        result = bmesh.ops.create_icosphere(self.bm, subdivisions=2, radius=0.5, matrix=m)
        self._paint(self._faces_of(result), mat, smooth=True)

    def poly(self, verts, faces, mat):
        """Any shape from a vertex list and faces given as index tuples."""
        vs = [self.bm.verts.new(Vector(v)) for v in verts]
        made = [self.bm.faces.new([vs[i] for i in f]) for f in faces]
        self._paint(made, mat)

    def prism(self, profile, x0, x1, mat):
        """Extrude a 2D (y, z) outline along X from x0 to x1."""
        n = len(profile)
        verts = [(x0, y, z) for y, z in profile] + [(x1, y, z) for y, z in profile]
        faces = [tuple(range(n))[::-1], tuple(range(n, 2 * n))]
        for i in range(n):
            j = (i + 1) % n
            faces.append((i, j, n + j, n + i))
        self.poly(verts, faces, mat)

    def finish(self, collection):
        mesh = bpy.data.meshes.new(self.name)
        # Hand-built shapes can come out inside-out; point every face outward.
        bmesh.ops.recalc_face_normals(self.bm, faces=self.bm.faces)
        self.bm.normal_update()
        self.bm.to_mesh(mesh)
        self.bm.free()
        for mat in self.materials:
            mesh.materials.append(mat)
        obj = bpy.data.objects.new(self.name, mesh)
        collection.objects.link(obj)
        return obj


# ---------------------------------------------------------------------------
# The house
# ---------------------------------------------------------------------------


def openings_on(wall, h):
    """Door and window gaps on a wall as (from, to, bottom, top), along the wall."""
    z0 = h["foundation"]
    windows = {"front": h["front_windows"], "back": h["back_windows"],
               "left": h["side_windows"], "right": h["side_windows"]}[wall]
    gaps = [(c - WINDOW_W / 2, c + WINDOW_W / 2, z0 + WINDOW_SILL, z0 + WINDOW_SILL + WINDOW_H) for c in windows]
    if wall == "front":
        gaps.append((h["door_x"] - DOOR_W / 2, h["door_x"] + DOOR_W / 2, z0, z0 + DOOR_H))
    return gaps


def log_spans(start, end, z, r, gaps):
    """Split one log row into pieces that go around any openings it crosses."""
    spans = [(start, end)]
    for g0, g1, bottom, top in gaps:
        if z + r * 0.3 < bottom or z - r * 0.3 > top:
            continue
        new = []
        for s0, s1 in spans:
            if g1 <= s0 or g0 >= s1:
                new.append((s0, s1))
                continue
            if g0 > s0:
                new.append((s0, g0))
            if g1 < s1:
                new.append((g1, s1))
        spans = new
    return [s for s in spans if s[1] - s[0] > 0.05]


def build_walls(h, mats, collection, rows, name):
    b = MeshBuilder(name)
    rng = random.Random(h["seed"])
    d = h["log_diameter"]
    r = d / 2
    hw, hd = h["width"] / 2, h["depth"] / 2
    over = h["corner_overhang"]
    z0 = h["foundation"]

    for i in range(rows):
        z = z0 + r + i * d
        wobble = lambda: rng.uniform(-0.012, 0.012)
        # Front and back walls run along X; side walls along Y, sitting half a log higher
        # so the corners interlock.
        for wall, y in (("front", -hd + r), ("back", hd - r)):
            gaps = openings_on(wall, h)
            for s0, s1 in log_spans(-hw - over, hw + over, z, r, gaps):
                b.cylinder(((s0 + s1) / 2, y + wobble(), z), r * rng.uniform(0.96, 1.04), s1 - s0, "X",
                           mats["log_x"], segments=12, cap_mat=mats["log_end"])
        zs = z + r * 0.5
        for wall, x in (("left", -hw + r), ("right", hw - r)):
            # Side walls are measured along Y; their windows use the same numbers.
            gaps = [(g0, g1, lo, hi) for g0, g1, lo, hi in openings_on(wall, h)]
            for s0, s1 in log_spans(-hd - over, hd + over, zs, r, gaps):
                b.cylinder((x + wobble(), (s0 + s1) / 2, zs), r * rng.uniform(0.96, 1.04), s1 - s0, "Y",
                           mats["log_y"], segments=12, cap_mat=mats["log_end"])
    # Inner walls go with the outer ones, so the cutaway lowers both.
    add_partitions(b, h, mats, min(2.6, rows * d))
    return b.finish(collection)


PARTITION_X = 1.2


def add_partitions(b, h, mats, height):
    """Inner plank walls: the main room on the left (stove, table, the grandmother's
    bed), a bedroom at the back right and a pantry at the front right, with doorways
    from the main room into both."""
    hw, hd = h["width"] / 2, h["depth"] / 2
    z0 = h["foundation"]
    inner = h["log_diameter"]
    yi0, yi1 = -hd + inner, hd - inner
    xi1 = hw - inner
    # Along Y at PARTITION_X, with two doorways.
    doors = [(-1.6, -0.7), (0.9, 1.8)]
    edges = [yi0] + [v for d in doors for v in d] + [yi1]
    for i in range(0, len(edges), 2):
        y0, y1 = edges[i], edges[i + 1]
        b.box((PARTITION_X, (y0 + y1) / 2, z0 + height / 2), (0.1, y1 - y0, height), mats["planks"])
    # Along X between the pantry and the bedroom.
    b.box(((PARTITION_X + xi1) / 2, 0.0, z0 + height / 2), (xi1 - PARTITION_X, 0.1, height), mats["planks"])


def build_window_on(b, mats, center, facing, z_bottom, width, height):
    """A window with an ochre carved frame, glass, mullions and lace curtains.

    facing: outward direction as (dx, dy)."""
    dx, dy = facing
    # Local axes: u runs along the wall, n points outward.
    def at(u, n, z):
        return (center[0] + u * (-dy) + n * dx, center[1] + u * dx + n * dy, z)

    rot = (0, 0, math.atan2(dy, dx) + math.pi / 2)
    zc = z_bottom + height / 2
    t = 0.09  # frame board width

    def carved(outline, n0, n1, mat):
        """A board cut to an outline given as (u, z) points, n0..n1 out from the wall."""
        n = len(outline)
        verts = [at(u, n0, z) for u, z in outline] + [at(u, n1, z) for u, z in outline]
        faces = [tuple(range(n)), tuple(range(2 * n - 1, n - 1, -1))]
        for i in range(n):
            j = (i + 1) % n
            faces.append((i, n + i, n + j, j))
        b.poly(verts, faces, mat)

    # Frame boards
    b.box(at(-width / 2 - t / 2, 0.06, zc), (t, 0.06, height + 2 * t), mats["ochre"], rot)
    b.box(at(width / 2 + t / 2, 0.06, zc), (t, 0.06, height + 2 * t), mats["ochre"], rot)

    # Carved crown: a bell-shaped pediment with a finial and flicked-up ears.
    top = z_bottom + height
    cw = width + 0.36
    upper = []
    steps = 40
    for i in range(steps + 1):
        u = cw / 2 - cw * i / steps
        bell = math.cos(math.pi * u / cw) ** 2
        ears = 0.06 * math.exp(-(((abs(u) - cw / 2 + 0.05) / 0.035) ** 2))
        finial = 0.07 * math.exp(-((u / 0.035) ** 2))
        upper.append((u, top + 0.12 + 0.17 * bell * width + ears + finial))
    carved([(-cw / 2, top), (cw / 2, top)] + upper, 0.04, 0.11, mats["ochre"])

    # A second, smaller carved layer in front, so the crown has depth.
    iw = cw * 0.68
    inner = []
    for i in range(steps + 1):
        u = iw / 2 - iw * i / steps
        bell = math.cos(math.pi * u / iw) ** 2
        inner.append((u, top + 0.1 + 0.12 * bell * width + 0.05 * math.exp(-((u / 0.03) ** 2))))
    carved([(-iw / 2, top + 0.03), (iw / 2, top + 0.03)] + inner, 0.11, 0.145, mats["ochre"])

    # Carved drops hanging under the ends of the crown.
    axis = "X" if dx != 0 else "Y"
    for side in (-1, 1):
        b.cylinder(at(side * (cw / 2 - 0.05), 0.08, top - 0.045), 0.026, 0.09, "Z", mats["ochre"],
                   segments=8, radius2=0.006)
        # A raised bead down the middle of each side board.
        b.box(at(side * (width / 2 + t / 2), 0.095, zc), (0.026, 0.024, height), mats["ochre"], rot)
        # A small round boss where the side board meets the sill.
        b.cylinder(at(side * (width / 2 + t / 2), 0.1, z_bottom + 0.02), 0.04, 0.03, axis, mats["ochre"],
                   segments=10)

    # Sill, and a scalloped apron below it.
    b.box(at(0, 0.1, z_bottom - 0.05), (width + 0.3, 0.14, 0.07), mats["ochre"], rot)
    aw = width * 0.9
    lower = []
    for i in range(steps + 1):
        u = -aw / 2 + aw * i / steps
        taper = 1.0 - (2 * u / aw) ** 2
        lower.append((u, z_bottom - 0.12 - 0.12 * abs(math.sin(3 * math.pi * u / aw)) * taper))
    carved([(aw / 2, z_bottom - 0.08), (-aw / 2, z_bottom - 0.08)] + lower, 0.03, 0.08, mats["ochre"])
    # Glass, set back into the wall, with a cross of mullions
    b.box(at(0, -0.08, zc), (width, 0.02, height), mats["glass"], rot)
    b.box(at(0, -0.06, zc), (0.05, 0.04, height), mats["ochre"], rot)
    b.box(at(0, -0.06, z_bottom + height * 0.62), (width, 0.04, 0.05), mats["ochre"], rot)
    # Lace curtains just inside, drawn to each side
    for side in (-1, 1):
        b.box(at(side * width * 0.3, -0.16, zc - 0.05), (width * 0.38, 0.01, height * 0.9), mats["lace"], rot)


def build_windows(h, mats, collection):
    b = MeshBuilder("Windows")
    hw, hd = h["width"] / 2, h["depth"] / 2
    z = h["foundation"] + WINDOW_SILL
    for x in h["front_windows"]:
        build_window_on(b, mats, (x, -hd), (0, -1), z, WINDOW_W, WINDOW_H)
    for x in h["back_windows"]:
        build_window_on(b, mats, (x, hd), (0, 1), z, WINDOW_W, WINDOW_H)
    for y in h["side_windows"]:
        build_window_on(b, mats, (-hw, y), (-1, 0), z, WINDOW_W, WINDOW_H)
        build_window_on(b, mats, (hw, y), (1, 0), z, WINDOW_W, WINDOW_H)
    return b.finish(collection)


def roof_geometry(h):
    hd = h["depth"] / 2
    eave = h["foundation"] + h["log_rows"] * h["log_diameter"] + 0.1
    pitch = math.radians(h["roof_pitch_deg"])
    rise = hd * math.tan(pitch)
    return eave, pitch, rise, eave + rise


def build_roof(h, mats, collection):
    b = MeshBuilder("Roof")
    rng = random.Random(h["seed"] + 1)
    hw, hd = h["width"] / 2, h["depth"] / 2
    eave, pitch, rise, ridge = roof_geometry(h)
    over = h["roof_overhang"]
    gover = h["gable_overhang"]
    x0, x1 = -hw - gover, hw + gover
    slope_len = (hd + over) / math.cos(pitch)

    # Each side: a sloped deck, then rows of shingles laid on it from the eave up.
    for side in (-1, 1):  # -1 = front slope, +1 = back slope
        tilt = -side * pitch
        deck_mid_y = side * (hd + over) / 2
        deck_mid_z = ridge - (hd + over) / 2 * math.tan(pitch)
        b.box((0, deck_mid_y, deck_mid_z), (x1 - x0, slope_len, 0.08), mats["trim"], (tilt, 0, 0))

        # Each row is laid in short runs of slightly different shingles, so the
        # roof looks patched and weathered rather than machine-made.
        shingle_mats = [mats["shingle"], mats["shingle"], mats["shingle_dark"], mats["shingle_light"]]
        row = 0.22
        n_rows = int(slope_len / row)

        def sag(x):
            # The roof dips a few centimetres towards the middle with age.
            return -0.04 * math.sin(math.pi * (x - x0) / (x1 - x0))

        for i in range(n_rows):
            dist = slope_len - (i + 0.5) * row          # distance down the slope from the ridge
            y = side * dist * math.cos(pitch)
            z = ridge - dist * math.sin(pitch) + 0.06
            if i == n_rows - 1:
                # The eave row: individual shingles of different widths, a few
                # missing, some cocked up by the wind.
                sx = x0 + rng.uniform(-0.05, 0.02)
                while sx < x1:
                    w = rng.uniform(0.11, 0.24)
                    if rng.random() > 0.05:
                        b.box((sx + w / 2, y, z + sag(sx) + rng.uniform(-0.01, 0.02)), (w - 0.012, row * 1.3, 0.03),
                              rng.choice(shingle_mats),
                              (tilt - side * rng.uniform(0.04, 0.12), rng.uniform(-0.05, 0.05), rng.uniform(-0.04, 0.04)))
                    sx += w
                continue
            # Other rows: short runs, with ragged ends at the gables.
            sx = x0 + rng.uniform(-0.06, 0.03)
            end = x1 + rng.uniform(-0.03, 0.06)
            while sx < end - 0.05:
                run = min(rng.uniform(0.8, 2.6), end - sx)
                lift = rng.uniform(-0.008, 0.012)
                b.box((sx + run / 2, y, z + lift + sag(sx + run / 2)), (run, row * 1.25, 0.035),
                      rng.choice(shingle_mats), (tilt - side * rng.uniform(0.05, 0.08), 0, 0))
                sx += run

        # Moss tufts gathering on the lower, shadier part of the slope.
        for _ in range(14 if side > 0 else 9):
            dist = slope_len * rng.uniform(0.55, 0.95)
            mx = rng.uniform(x0 + 0.3, x1 - 0.3)
            my = side * dist * math.cos(pitch)
            mz = ridge - dist * math.sin(pitch) + 0.09 + sag(mx)
            b.blob((mx, my, mz), (rng.uniform(0.15, 0.4), rng.uniform(0.12, 0.25), 0.05), mats["moss"],
                   (tilt, 0, rng.uniform(0, math.pi)))

    # Ridge cap: two weathered boards meeting over the ridge.
    for side in (-1, 1):
        b.box((0, side * 0.08, ridge + 0.1), (x1 - x0 + 0.05, 0.2, 0.04), mats["trim"], (-side * pitch, 0, 0))

    # Gables: triangles of red vertical boards on both ends, with grey barge boards.
    for side in (-1, 1):
        x = side * (hw + 0.02)
        gx0, gx1 = (x - 0.06, x) if side < 0 else (x, x + 0.06)
        b.prism([(-hd, eave), (hd, eave), (0, ridge)], gx0, gx1, mats["red_boards"])
        for slope in (-1, 1):
            mid_y = slope * (hd + over) / 2
            mid_z = ridge - (hd + over) / 2 * math.tan(pitch) + 0.05
            b.box((side * (hw + gover - 0.04), mid_y, mid_z), (0.08, slope_len, 0.22), mats["trim"], (-slope * pitch, 0, 0))

    # Attic windows in the gables.
    for side in (-1, 1):
        build_window_on(b, mats, (side * (hw + 0.06), 0.0), (side, 0), eave + 0.7, 0.55, 0.75)

    # Loft floor (hidden with the roof when looking inside).
    b.box((0, 0, eave - 0.05), (h["width"] - 0.6, h["depth"] - 0.6, 0.1), mats["floor"])

    # Chimney top, from the loft up through the front slope near the ridge.
    cx, cy = h["chimney_x"], h["chimney_y"]
    top = ridge + 0.45
    b.box((cx, cy, (eave + top) / 2), (0.62, 0.62, top - eave), mats["brick"])
    b.box((cx, cy, top + 0.06), (0.76, 0.76, 0.12), mats["brick"])
    return b.finish(collection)


def build_shell(h, mats, collection):
    b = MeshBuilder("Shell")
    rng = random.Random(h["seed"] + 2)
    hw, hd = h["width"] / 2, h["depth"] / 2
    z0 = h["foundation"]

    # Fieldstone foundation: a plinth with stones bulging out of it.
    b.box((0, 0, z0 / 2), (h["width"] + 0.25, h["depth"] + 0.25, z0), mats["stone"])
    for wall in range(4):
        length = h["width"] if wall < 2 else h["depth"]
        u = -length / 2
        while u < length / 2:
            w = rng.uniform(0.3, 0.6)
            for row in range(2):
                hgt = z0 / 2 * rng.uniform(0.85, 1.0)
                zc = hgt / 2 + row * z0 / 2
                if wall == 0:
                    c, s = (u + w / 2, -hd - 0.14, zc), (w * 0.95, 0.1, hgt)
                elif wall == 1:
                    c, s = (u + w / 2, hd + 0.14, zc), (w * 0.95, 0.1, hgt)
                elif wall == 2:
                    c, s = (-hw - 0.14, u + w / 2, zc), (0.1, w * 0.95, hgt)
                else:
                    c, s = (hw + 0.14, u + w / 2, zc), (0.1, w * 0.95, hgt)
                b.box(c, s, mats["stone"], (rng.uniform(-0.04, 0.04),) * 3)
            u += w

    # Door: red planks with a small glazed top, set into the opening.
    dx = h["door_x"]
    b.box((dx, -hd + 0.12, z0 + DOOR_H / 2), (DOOR_W - 0.06, 0.06, DOOR_H - 0.02), mats["red_door"])
    b.box((dx, -hd + 0.08, z0 + DOOR_H - 0.45), (0.5, 0.02, 0.45), mats["glass"])
    b.box((dx, -hd + 0.07, z0 + DOOR_H - 0.45), (0.04, 0.03, 0.45), mats["ochre"])
    b.box((dx, -hd + 0.07, z0 + DOOR_H - 0.45), (0.5, 0.03, 0.04), mats["ochre"])
    b.box((dx + 0.35, -hd + 0.06, z0 + 1.0), (0.04, 0.06, 0.12), mats["iron"])
    # Door frame
    for side in (-1, 1):
        b.box((dx + side * (DOOR_W / 2 + 0.05), -hd + 0.03, z0 + DOOR_H / 2), (0.1, 0.1, DOOR_H + 0.1), mats["trim"])
    b.box((dx, -hd + 0.03, z0 + DOOR_H + 0.06), (DOOR_W + 0.3, 0.1, 0.14), mats["trim"])

    # Porch: deck, step, two carved posts and a small gabled roof.
    porch_w, porch_d = 2.4, 1.3
    deck_top = z0 - 0.05
    b.box((dx, -hd - porch_d / 2, deck_top - 0.08), (porch_w, porch_d, 0.16), mats["trim"])
    b.box((dx, -hd - porch_d - 0.3, deck_top - 0.3), (1.4, 0.6, 0.18), mats["stone"])
    post_top = z0 + DOOR_H + 0.35
    for side in (-1, 1):
        px, py = dx + side * (porch_w / 2 - 0.15), -hd - porch_d + 0.15
        b.box((px, py, (deck_top + post_top) / 2), (0.14, 0.14, post_top - deck_top), mats["trim"])
        # Carving: a stack of swellings up the post.
        z = deck_top + 0.5
        while z < post_top - 0.4:
            b.cylinder((px, py, z), 0.1, 0.16, "Z", mats["trim"], segments=8, radius2=0.07)
            z += 0.22
    # Porch roof: it runs out from the wall, so its ridge is along Y and the
    # slopes fall to the left and right.
    pr_eave = post_top + 0.05
    pr_pitch = math.radians(34)
    half = porch_w / 2 + 0.25
    pr_rise = half * math.tan(pr_pitch)
    ry0, ry1 = -hd + 0.1, -hd - porch_d - 0.3
    depth = ry0 - ry1
    mid_y = (ry0 + ry1) / 2
    slope_len = half / math.cos(pr_pitch)
    shingle_mats = [mats["shingle"], mats["shingle_dark"], mats["shingle_light"]]
    for side in (-1, 1):
        tilt = side * pr_pitch
        # Deck under the shingles.
        b.box((dx + side * half / 2, mid_y, pr_eave + pr_rise / 2), (slope_len, depth, 0.05), mats["trim"], (0, tilt, 0))
        # Shingle rows parallel to the ridge, from the eave up.
        row = 0.2
        n_rows = int(slope_len / row)
        for i in range(n_rows):
            dist = slope_len - (i + 0.5) * row
            x = dx + side * dist * math.cos(pr_pitch)
            z = pr_eave + pr_rise - dist * math.sin(pr_pitch) + 0.05
            b.box((x, mid_y, z), (row * 1.25, depth + 0.04, 0.03), rng.choice(shingle_mats), (0, tilt + side * 0.06, 0))
        # Barge board along the front edge of each slope.
        b.box((dx + side * half / 2, ry1 - 0.02, pr_eave + pr_rise / 2 + 0.05), (slope_len + 0.1, 0.05, 0.2),
              mats["trim"], (0, tilt, 0))
    b.box((dx, mid_y, pr_eave + pr_rise + 0.08), (0.16, depth + 0.06, 0.1), mats["trim"])
    # The gable under it, set back a little, with a small carved sunburst.
    gy = ry1 + 0.12
    tri = [(-half + 0.1, pr_eave), (half - 0.1, pr_eave), (0, pr_eave + pr_rise - 0.08)]
    b.poly([(dx + gx, gy, gz) for gx, gz in tri] + [(dx + gx, gy + 0.04, gz) for gx, gz in tri],
           [(0, 1, 2), (5, 4, 3), (0, 3, 4, 1), (1, 4, 5, 2), (2, 5, 3, 0)], mats["red_boards"])
    sun_z = pr_eave + pr_rise * 0.35
    for k in range(7):
        a = math.pi * (k + 0.5) / 7
        b.box((dx + math.cos(a) * 0.22, gy - 0.02, sun_z + math.sin(a) * 0.22), (0.05, 0.02, 0.24),
              mats["ochre"], (0, -(a - math.pi / 2), 0))
    b.cylinder((dx, gy - 0.02, sun_z), 0.09, 0.03, "Y", mats["ochre"], segments=12)
    return b.finish(collection)


def build_interior(h, mats, collection):
    b = MeshBuilder("Interior")
    hw, hd = h["width"] / 2, h["depth"] / 2
    z0 = h["foundation"]
    inner = h["log_diameter"]
    xi0, xi1 = -hw + inner, hw - inner
    yi0, yi1 = -hd + inner, hd - inner

    # Plank floor.
    b.box((0, 0, z0 + 0.02), (xi1 - xi0, yi1 - yi0, 0.04), mats["floor"])

    # The big tiled stove under the chimney, with a cooking ledge and a flue.
    cx, cy = h["chimney_x"], h["chimney_y"]
    b.box((cx, cy, z0 + 0.95), (1.5, 1.3, 1.9), mats["tile"])
    b.box((cx, cy - 0.75, z0 + 0.85), (1.5, 0.3, 0.06), mats["iron"])
    b.box((cx, cy - 0.66, z0 + 0.5), (0.45, 0.05, 0.35), mats["iron"])
    b.box((cx, cy, z0 + 2.4), (0.6, 0.6, 1.1), mats["brick"])
    # A sleeping bench beside the stove, where Grandmother Ingrid sleeps warm.
    b.box((cx - 1.45, cy + 0.3, z0 + 0.25), (1.2, 1.9, 0.5), mats["planks"])
    b.box((cx - 1.45, cy + 0.3, z0 + 0.53), (1.1, 1.8, 0.08), mats["wool_red"])

    # Table and benches by the front windows.
    tx, ty = -3.2, -2.3
    b.box((tx, ty, z0 + 0.74), (1.7, 0.9, 0.06), mats["planks"])
    for lx in (-0.7, 0.7):
        for ly in (-0.35, 0.35):
            b.box((tx + lx, ty + ly, z0 + 0.36), (0.08, 0.08, 0.72), mats["planks"])
    for side in (-1, 1):
        b.box((tx, ty + side * 0.75, z0 + 0.42), (1.6, 0.3, 0.05), mats["planks"])
        b.box((tx, ty + side * 0.75, z0 + 0.2), (1.5, 0.08, 0.4), mats["planks"])
    # A bowl, a loaf and a lamp on the table.
    b.cylinder((tx - 0.3, ty, z0 + 0.81), 0.13, 0.08, "Z", mats["pottery"], segments=12)
    b.box((tx + 0.3, ty + 0.1, z0 + 0.82), (0.28, 0.16, 0.1), mats["ochre"])
    b.cylinder((tx, ty - 0.2, z0 + 0.88), 0.05, 0.22, "Z", mats["iron"], segments=8)

    # Cupboard and chest along the back wall, a spinning wheel by the window.
    b.box((-3.6, yi1 - 0.25, z0 + 0.9), (1.1, 0.45, 1.8), mats["red_door"])
    b.box((-2.2, yi1 - 0.3, z0 + 0.3), (1.0, 0.55, 0.6), mats["wool_green"])
    b.cylinder((-0.2, -2.6, z0 + 0.75), 0.4, 0.06, "X", mats["planks"], segments=16)
    b.box((-0.2, -2.6, z0 + 0.35), (0.5, 0.25, 0.08), mats["planks"])

    # Ladder up to the loft.
    lx, ly = cx + 1.1, 1.4
    eave, _, _, _ = roof_geometry(h)
    for side in (-0.25, 0.25):
        b.box((lx + side, ly, (z0 + eave) / 2), (0.06, 0.06, eave - z0), mats["planks"])
    z = z0 + 0.3
    while z < eave - 0.1:
        b.box((lx, ly, z), (0.5, 0.05, 0.04), mats["planks"])
        z += 0.3

    # Rag rugs.
    b.box((-2.0, -0.6, z0 + 0.045), (1.4, 0.8, 0.01), mats["wool_blue"])
    b.box((3.0, 2.0, z0 + 0.045), (1.0, 0.7, 0.01), mats["wool_red"])

    # Bedroom: the parents' bed and Elin's small bed.
    b.box((2.6, yi1 - 1.0, z0 + 0.3), (1.4, 1.95, 0.6), mats["planks"])
    b.box((2.6, yi1 - 1.0, z0 + 0.63), (1.3, 1.85, 0.1), mats["wool_blue"])
    b.box((2.6, yi1 - 0.18, z0 + 0.7), (1.3, 0.3, 0.12), mats["linen"])
    b.box((4.0, 0.9, z0 + 0.25), (0.8, 1.5, 0.5), mats["planks"])
    b.box((4.0, 0.9, z0 + 0.53), (0.72, 1.4, 0.08), mats["wool_green"])
    b.box((4.0, 1.5, z0 + 0.6), (0.7, 0.25, 0.1), mats["linen"])

    # Pantry: shelves with jars, sacks and a barrel.
    for level in range(3):
        b.box((xi1 - 0.25, -1.8, z0 + 0.5 + level * 0.55), (0.4, 2.6, 0.04), mats["planks"])
        for j in range(5):
            b.cylinder((xi1 - 0.25, -2.9 + j * 0.5, z0 + 0.6 + level * 0.55), 0.08, 0.18, "Z", mats["pottery"], segments=8)
    b.cylinder((2.0, -2.9, z0 + 0.4), 0.3, 0.8, "Z", mats["planks"], segments=14)
    b.box((2.9, -3.0, z0 + 0.3), (0.5, 0.4, 0.6), mats["linen"])
    b.box((2.4, -2.2, z0 + 0.25), (0.45, 0.35, 0.5), mats["linen"])
    return b.finish(collection)


# ---------------------------------------------------------------------------
# Scene, cameras and renders
# ---------------------------------------------------------------------------


def soften_edges(obj, width=0.012):
    """Rounds every sharp edge a little, so corners catch the light like real
    timber and stone instead of looking machine-cut."""
    bevel = obj.modifiers.new("Soft edges", "BEVEL")
    bevel.width = width
    bevel.segments = 2
    bevel.limit_method = "ANGLE"
    bevel.angle_limit = math.radians(40)
    bevel.use_clamp_overlap = True
    bevel.harden_normals = False


def make_collection(name):
    col = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(col)
    return col


def clear_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def setup_render(scene, samples=48):
    engines = [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties["engine"].enum_items]
    for preferred in ("BLENDER_EEVEE", "BLENDER_EEVEE_NEXT", "CYCLES"):
        if preferred in engines:
            scene.render.engine = preferred
            break
    if scene.render.engine.startswith("BLENDER_EEVEE") and hasattr(scene.eevee, "taa_render_samples"):
        scene.eevee.taa_render_samples = samples
    scene.render.resolution_x = 1280
    scene.render.resolution_y = 720
    scene.render.image_settings.file_format = "PNG"
    scene.view_settings.view_transform = "AgX" if "AgX" in [
        i.identifier for i in scene.view_settings.bl_rna.properties["view_transform"].enum_items] else "Filmic"

    scene.view_settings.exposure = -0.4

    world = bpy.data.worlds.new("World")
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    bg.inputs["Color"].default_value = (0.78, 0.79, 0.80, 1)
    bg.inputs["Strength"].default_value = 0.7
    scene.world = world

    sun_data = bpy.data.lights.new("Sun", "SUN")
    sun_data.energy = 3.0
    sun_data.angle = math.radians(12)
    sun = bpy.data.objects.new("Sun", sun_data)
    sun.rotation_euler = Euler((math.radians(50), math.radians(-10), math.radians(35)))
    scene.collection.objects.link(sun)


def add_camera(scene, name, location, target, ortho_scale=None, lens=45):
    cam_data = bpy.data.cameras.new(name)
    cam_data.lens = lens
    if ortho_scale:
        cam_data.type = "ORTHO"
        cam_data.ortho_scale = ortho_scale
    cam = bpy.data.objects.new(name, cam_data)
    scene.collection.objects.link(cam)
    cam.location = Vector(location)
    direction = Vector(target) - cam.location
    cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    return cam


def set_visible(collections, visible):
    for col in collections:
        col.hide_render = not visible
        col.hide_viewport = not visible


def render(scene, camera, path):
    scene.camera = camera
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)


def export_for_engine(root, key, parts):
    """Saves the house as assets/models/<key>_house.glb for the game engine.

    Each part stays a separate mesh (Shell, Walls_Full, Walls_Cutaway, Windows,
    Roof, Interior) so the game can lift the roof and lower the walls. Only
    material names are exported: the engine builds its own materials from the
    same photo textures, since Blender's node setups don't carry over."""
    out_dir = os.path.join(root, "assets", "models")
    os.makedirs(out_dir, exist_ok=True)
    # Every part must be visible to be exported, including the cutaway walls.
    for col in bpy.data.collections:
        col.hide_viewport = False
        col.hide_render = False
    for layer in bpy.context.view_layer.layer_collection.children:
        layer.exclude = False
        layer.hide_viewport = False
    for obj in bpy.context.scene.objects:
        obj.hide_set(False)
        obj.select_set(obj in parts)
    bpy.ops.export_scene.gltf(
        filepath=os.path.join(out_dir, f"{key}_house.glb"),
        export_format="GLB",
        use_selection=True,
        export_apply=True,
        export_materials="EXPORT",  # the engine matches materials by these names
        export_image_format="NONE",  # it uses its own copies of the textures
        export_yup=True,
    )


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    key = argv[argv.index("--house") + 1] if "--house" in argv else "lind"
    no_render = "--no-render" in argv
    h = HOUSES[key]

    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    blend_dir = os.path.join(root, "assets", "blender")
    render_dir = os.path.join(root, "assets", "renders", key)
    os.makedirs(blend_dir, exist_ok=True)
    os.makedirs(render_dir, exist_ok=True)

    clear_scene()
    scene = bpy.context.scene
    mats = build_materials()

    cols = {name: make_collection(name) for name in
            ("Shell", "Walls_Full", "Walls_Cutaway", "Windows", "Roof", "Interior", "Stage")}
    parts = [
        build_shell(h, mats, cols["Shell"]),
        build_walls(h, mats, cols["Walls_Full"], h["log_rows"], "Walls_Full"),
        build_walls(h, mats, cols["Walls_Cutaway"], CUTAWAY_ROWS, "Walls_Cutaway"),
        build_windows(h, mats, cols["Windows"]),
        build_roof(h, mats, cols["Roof"]),
        build_interior(h, mats, cols["Interior"]),
    ]
    for part in parts:
        soften_edges(part)

    # A stage to stand on: plain grey to compare with the references, or ground for game views.
    stage = MeshBuilder("Backdrop")
    stage.box((0, 0, -0.01), (200, 200, 0.02), mats["backdrop"])
    backdrop = stage.finish(cols["Stage"])
    ground_b = MeshBuilder("Ground")
    ground_b.box((0, 0, 0.0), (60, 60, 0.02), mats["ground"])
    ground = ground_b.finish(cols["Stage"])

    setup_render(scene)
    eave, _, _, ridge = roof_geometry(h)
    centre = (0, 0, ridge * 0.45)

    cams = {
        "compare_three_quarter": add_camera(scene, "Cam_ThreeQuarter", (14, -17, 6.5), centre, lens=50),
        "compare_front": add_camera(scene, "Cam_Front", (0, -30, ridge * 0.5), (0, 0, ridge * 0.5), ortho_scale=14),
        "game_overhead": add_camera(scene, "Cam_Overhead", (9, -16, 22), (0, 0, 1), lens=45),
        "game_inside": add_camera(scene, "Cam_Inside", (3, -8, 15), (0, 0.3, 0.5), lens=40),
    }

    cutaway = [cols["Walls_Cutaway"]]
    full = [cols["Walls_Full"], cols["Windows"], cols["Roof"]]

    # Default state saved in the file: the closed house, shown with its
    # materials (not Blender's plain grey Solid view) when opened.
    set_visible(cutaway, False)
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == "VIEW_3D":
                for space in area.spaces:
                    if space.type == "VIEW_3D":
                        space.shading.type = "MATERIAL"
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(blend_dir, f"{key}.blend"))

    if "--export" in argv:
        export_for_engine(root, key, parts)

    if no_render:
        return

    # Compare with the references on a plain grey stage.
    ground.hide_render = True
    backdrop.hide_render = False
    render(scene, cams["compare_three_quarter"], os.path.join(render_dir, "compare_three_quarter.png"))
    render(scene, cams["compare_front"], os.path.join(render_dir, "compare_front.png"))

    # How it looks from the game camera, on the ground.
    ground.hide_render = False
    backdrop.hide_render = True
    render(scene, cams["game_overhead"], os.path.join(render_dir, "game_overhead.png"))

    # Opened up: roof off, walls cut low, so the rooms show.
    set_visible(full, False)
    set_visible(cutaway, True)
    render(scene, cams["game_inside"], os.path.join(render_dir, "game_inside.png"))

    set_visible(full, True)
    set_visible(cutaway, False)


main()
