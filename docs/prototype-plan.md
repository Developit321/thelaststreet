# Prototype Plan

The gameplay prototype from the design document (§12, Phase 1), built in small steps. Each step ends with something you can see and try.

## How each step works

1. **Claude builds the step,** compiles it and commits it.
2. **Claude hands it over** with how to launch it and a short list of what to test.
3. **You test it** and report back: anything broken, anything that feels wrong.
4. **Claude fixes what you found.**
5. **Only when you say the step passes** do we move to the next one.

## Technical ground rules (so the core moves into Unreal later)

- The simulation lives in `core/`, in plain C++ with no graphics code.
- **Commands in, events out:** the game sends the simulation commands ("Father: chop wood"); the simulation reports events ("6 logs added"). The window only listens and draws.
- The simulation knows **places, not pixels** ("at the woodpile", not screen coordinates).
- No C++ exceptions and no hidden global state in `core/`; the simulation advances in fixed steps, so the same inputs always give the same results.
- Game data lives in text files under `data/`. Only a small loader outside `core/` reads them.
- The window (raylib) lives in `app/` and is throwaway: Unreal replaces it later.

## Steps

### Step 1: The street on screen
**Builds:** the project, the build script and a window showing the street from above: the road, about 5 house plots as boxes, the pump, fences and the forest edge. Camera pan and zoom.
**You test:**
- It builds and opens on your Mac with one command
- Pan (drag or arrow keys) and zoom (scroll) feel smooth
- The layout reads as one street with houses on both sides

### Step 2: Time and seasons
**Builds:** the game clock (days, seasons, years), pause and speed controls, temperature through the year, and the ground changing colour with the seasons (green, gold, brown, white).
**You test:**
- Pause, normal, fast and fastest speeds work
- Seasons pass in order, and the date and temperature display are right
- Winter is clearly colder, and the ground turns white

### Step 3: Your household
**Builds:** your family of four as dots with names, the household's needs (food, firewood, warmth, health, morale), the stove burning firewood, daily food use, and simple daily routines (people move between house, garden and woodpile).
**You test:**
- Clicking a person shows who they are and how they're doing
- Food and firewood go down every day, and faster in winter
- If the firewood runs out, warmth and then health drop

### Step 4: Jobs and direct orders
**Builds:** the jobs panel (assign each person to firewood, garden, repair or rest), direct orders (select a person, then click a target), and planting and harvesting.
**You test:**
- Assigning jobs changes what people do and what gets produced
- Direct orders work: select, click, they go and do it
- With good planning you can survive the first (mild) winter

### Step 5: Neighbours
**Builds:** 4 neighbour households with their own families, traits, needs and routines; relationships between every household; memory of what was done to them; and an inspect panel.
**You test:**
- Neighbours live their own lives without you doing anything
- Each household feels different, because the traits show
- A neighbour can run short of food or wood on their own

### Step 6: Trading
**Builds:** the barter screen at a neighbour's gate, prices that change with season and scarcity, and relationships and traits affecting deals. Neighbours can also come to you.
**You test:**
- Trades make sense, and firewood is precious in winter
- Liked households give better deals; disliked ones refuse
- Neighbours remember a fair deal, or a cheat

### Step 7: Building and repairs
**Builds:** 4–5 house upgrades, 3 yard buildings placed on a grid, materials and labour, building stages, house condition dropping over time, and repairs.
**You test:**
- Upgrades visibly matter (for example, insulation saves firewood)
- Building takes materials, people and time, and slows in bad weather
- Neglected houses decay, and repairs work

### Step 8: Dilemmas, standing and the storyteller
**Builds:** 10 dilemma events as cards with choices, consequences remembered by neighbours, street standing, and the basic storyteller adjusting pressure.
**You test:**
- Events show up at sensible moments and the choices feel hard
- Neighbours react later to what you chose
- When you're struggling, the game eases off; when you're comfortable, it pushes

### Step 9: The first year and the summary
**Builds:** onboarding through the friendly neighbour, the season goals panel, one street project, a mild first winter, a hard second winter, and the end-of-winter summary.
**You test:**
- A new player understands what to do without reading a manual
- There's always a clear next goal
- The summary tells the story of the winter

### Step 10: Playable build and tuning
**Builds:** a save and load system, a balancing pass on the numbers, and fixes from your play sessions.
**You test:**
- Play two full years start to finish
- **The real test: do you want to play another year?**
