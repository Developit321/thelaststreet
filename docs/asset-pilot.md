# Asset Pilot: One House

A small test of the asset pipeline on a single house, before making the rest of the kit. It runs alongside the gameplay prototype.

**The question it answers:** can Claude build a house in Blender with code that looks like our concept art and is clean enough for Unreal?

## The approach

Buildings and props are **built with code** in Blender: Claude writes Python that constructs the model piece by piece (log walls, notched corners, stone foundation, shingle roof, windows with carved frames, porch, chimney) and applies free CC0 textures from Poly Haven. Claude renders preview images, checks them, and adjusts.

Why code instead of image-to-3D for buildings:
- **Exact sizes** that match the plots in `data/street.json`
- **Clean meshes** from the start
- **Matching variants** from one generator: draughty or sealed walls, leaking or repaired roof, damaged, snowy, with or without an extension
- **Every house on the street** can come from the same generator with different settings
- **Free:** Blender and CC0 textures

People and animals still go through image-to-3D and Mixamo (see `docs/asset-prompts.md`, priorities 5 and 6).

## Houses must open up

Players see inside a house from the top-down camera when someone goes in (design document §5). So every house is built in separate parts:
- **Roof:** roof, gables, chimney top and loft floor, which the game fades away up close
- **Walls:** full height, plus a cutaway version cut low so the rooms read from above
- **Interior:** floor, room partitions, the stove, furniture
- **Shell:** foundation, porch, door and windows

## Steps

| Step | Who | What |
|---|---|---|
| 1 | You | Install Blender (blender.org, Apple Silicon) into `~/Applications` |
| 2 | You | Generate the Lind house references with prompts **1.1, 1.6 and 1.7** in `docs/asset-prompts.md`, plus the front, side and back views |
| 3 | You | Save them in `assets/reference/houses/` |
| 4 | Claude | Write the house generator and build the Lind house to match |
| 5 | Claude | Render previews from the game's camera angles: high overhead, mid, close |
| 6 | Together | Compare with the references and concept art; adjust |
| 7 | Claude | Add variants: snowy roof, draughty vs repaired, damaged |
| 8 | Claude | Export a version ready for Unreal |

## Optional: a live link to Blender

**BlenderMCP** lets Claude control a running Blender window you can watch, and fetch Poly Haven textures directly. Without it, Claude runs Blender in the background and sends you renders, which works fine too. Setup (no admin rights needed):

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```
```bash
uvx blender-mcp install-addon
```
Then enable "Interface: Blender MCP" in Blender's preferences, and connect it:
```bash
claude mcp add blender uvx blender-mcp
```

It's a third-party open-source project: [github.com/ahujasid/blender-mcp](https://github.com/ahujasid/blender-mcp).

## Success test

- From the high and mid camera angles, it reads as **our** house and matches the references
- The variants (snow, repaired, damaged) clearly read as the same house
- It's light enough to use many times on the street
