# The Last Street

*In a cold northern land at the turn of the century, one street must survive the winters together.*

A survival and community builder set on a single street in a fictional northern country around 1900. Build and repair your home, trade with neighbours who remember what you did, and prepare for winters that test every choice.

**Status:** gameplay prototype, step 3 of 10 (see [`docs/prototype-plan.md`](docs/prototype-plan.md)).

## Run the prototype

On a Mac with Xcode installed, from this folder:

```bash
make run
```

The first build compiles raylib and takes a minute; later builds are quick.

## Project layout

- `core/`: the simulation, in plain C++ with no graphics. This moves into Unreal later.
- `loader/`: reads the files in `data/` into core types.
- `app/`: the throwaway prototype window (raylib).
- `data/`: game data as text files.
- `third_party/`: raylib 5.5 and nlohmann/json 3.11.3, with their licences.

## Contents

- [`docs/game-design-document.md`](docs/game-design-document.md): the full design: setting, systems, story acts, tech, asset pipeline and plan
- [`docs/prototype-plan.md`](docs/prototype-plan.md): the step-by-step plan for the gameplay prototype
- [`reference/concept-art/`](reference/concept-art/): AI-generated concept art used as the visual target (internal reference only)

## Next steps

1. Settle the open decisions in the design document (§14)
2. Install Unreal Engine 5
3. Phase 0: the look test
