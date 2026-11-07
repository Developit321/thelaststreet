# The Last Street

*In a cold northern land at the turn of the century, one street must survive the winters together.*

A survival and community builder set on a single street in a fictional northern country around 1900. Build and repair your home, trade with neighbours who remember what you did, and prepare for winters that test every choice.

**Status:** gameplay prototype, step 4 of 10 (see [`docs/prototype-plan.md`](docs/prototype-plan.md)).

## Run the 3D prototype (Godot)

Needs Godot 4.7 in `~/Applications`, Blender in `/Applications` and SCons (`python3 -m pip install --user scons`). From this folder:

```bash
make godot-run
```

That builds the simulation library, makes the house model and fetches the textures if they're missing, imports the project and opens the game. You can also open the folder in the Godot editor after running `make godot` once.

## Run the 2D prototype (raylib)

```bash
make run
```

## Project layout

- `core/`: the simulation, in plain C++ with no graphics. Godot (and later Unreal, if we move) only display it.
- `loader/`: reads the files in `data/` into core types.
- `data/`: game data as text files.
- `project.godot` and `godot/`: the 3D prototype. `godot/extension/` is the C++ bridge (`StreetSim`); `godot/scripts/` builds the world, houses, people, camera and interface.
- `app/`: the original 2D prototype window (raylib).
- `tools/blender/`: the house generator.
- `assets/`: textures (downloaded), models (generated), references.
- `third_party/`: raylib 5.5, nlohmann/json 3.11.3 and godot-cpp 4.5, with their licences.

## Contents

- [`docs/game-design-document.md`](docs/game-design-document.md): the full design: setting, systems, story acts, tech, asset pipeline and plan
- [`docs/prototype-plan.md`](docs/prototype-plan.md): the step-by-step plan for the gameplay prototype
- [`docs/asset-prompts.md`](docs/asset-prompts.md): reference image prompts for every asset
- [`docs/asset-pilot.md`](docs/asset-pilot.md): the one-house asset pilot
- [`reference/concept-art/`](reference/concept-art/): AI-generated concept art used as the visual target (internal reference only)

## Next steps

1. Settle the open decisions in the design document (§14)
2. Install Unreal Engine 5
3. Phase 0: the look test
