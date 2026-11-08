# Prototype Plan

The gameplay prototype from the design document (§12, Phase 1), built in small steps. Each step ends with something you can see and try.

**Status (2026-11-08):** steps 1 to 4 passed. Steps 5 onward follow the updated design: the street as one body, permanent loss, and things running out (design document §7.10 to §7.13).

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
- The raylib window in `app/` and the Godot project were stepping stones. Unreal, on the Windows PC, is the real front end from here.

## Placeholders

- **Country:** Norrvik. **Street:** Birch Row. Easy to change later.
- **Neighbour households:** Claude writes the five families (names, members, traits, what each one holds) in `data/`. Change anything you don't like.

## Not in the prototype

To keep it small, these wait until after the prototype proves the fun:
- The replacement chains beyond one example (candles from tallow)
- Passing on skills beyond one example (the healer's apprentice)
- The later story acts and the ending
- Endless mode
- Difficulty settings
- Carry-over unlocks and the street chronicle
- Final art, animation and sound

## Steps

### Step 1: The street on screen
**Builds:** the project, the build script and a window showing the street from above: the road, about 5 house plots as boxes, the pump, fences and the forest edge. Camera pan and zoom.
**You test:**
- It builds and opens on your Mac with one command
- Pan (drag or arrow keys) and zoom (scroll) feel smooth
- The layout reads as one street with houses on both sides

### Step 2: Time and seasons
**Builds:** the game clock (days, seasons, years), pause and speed controls, temperature through the year, and the ground changing colour with the seasons (green, gold, brown, white). Also the **debug panel** (hidden behind a key) that grows with every step: jump to any season, and later add or remove resources, trigger any event, and show the hidden numbers (relationships, standing, the storyteller's pressure).
**You test:**
- Pause, normal, fast and fastest speeds work
- Seasons pass in order, and the date and temperature display are right
- Winter is clearly colder, and the ground turns white
- The debug panel opens and jumping to a season works

### Step 3: Your household
**Builds:** your family of four as dots with names, the household's needs (food, firewood, warmth, health, morale), the stove burning firewood, daily food use, and simple daily routines (people move between house, garden and woodpile). Also:
- **Warnings strip:** "Firewood will last 9 more days at this rate", "Food is running low"
- **Event log:** a running record of what happened and when
- **Sickness and death:** a family member whose health reaches zero dies, and the family carries on without them
- **Losing the household:** if everyone dies, the game ends with a simple summary of what happened
- Debug panel: add or remove food and firewood
**You test:**
- Clicking a person shows who they are and how they're doing
- Food and firewood go down every day, and faster in winter
- Warnings appear before things run out, and the log records events
- If the firewood runs out, warmth and then health drop, and people can die
- Losing everyone ends the game with a summary

### Step 4: Jobs and direct orders
**Builds:** the jobs panel (assign each person to firewood, garden, preserve food or rest; repairing comes with house damage in step 7), direct orders (select a person, then click a target), planting and harvesting, and **food storage:** fresh food spoils over time, preserving (drying, salting, smoking) makes it last, and storage space limits how much you can keep.
**You test:**
- Assigning jobs changes what people do and what gets produced
- Direct orders work: select, click, they go and do it
- Fresh food spoils, and preserving it makes it last through winter
- With good planning you can survive the first (mild) winter

### Step 5: Neighbours who hold what you need
**Builds:** 5 neighbour households with their own families, traits, needs and routines; each one **holding a piece of what the street needs** (well, cow, seed, iron-mending, midwifery and so on), reshuffled on a new game; relationships between every household; memory of what was done to them; neighbours helping, visiting and falling out without you; an inspect panel; the follow camera. **When someone dies, what they held is lost.** Debug panel: show and change relationships; kill a person.
**You test:**
- Neighbours live their own lives without you doing anything
- Each household feels different, and you can see who holds what
- Losing someone visibly costs the street something
- A new game gives a different spread of who holds what

### Step 6: Favours and barter
**Builds:** the barter screen at a neighbour's gate, prices that change with season and scarcity, relationships and traits affecting deals, and the **ledger of favours:** who owes whom, for what, and whether it was paid back. Neighbours come to you and trade with each other.
**You test:**
- Trades make sense, and firewood is precious in winter
- A favour you did gets remembered and can be called in
- Liked households give better deals; disliked ones refuse
- Neighbours trade among themselves (visible in the event log)

### Step 7: Running out
**Builds:** the **last train** and the finite stocks (salt, iron, lamp oil, matches, medicine); **water** (wells, privies that foul them, boiling that burns firewood); **salt needed to preserve meat; tools that wear out and break;** seed grain you can eat or save; and one replacement chain (tallow candles). Debug panel: set any stock; stop the train.
**You test:**
- You can see each stock going down and roughly when it will run out
- A fouled well makes people sick, and you can work out why
- Running out of salt or tools really changes how you play
- Eating the seed grain feels like a terrible idea you might still have to do

### Step 8: Building, repairs and fire
**Builds:** a few house upgrades, yard buildings on a grid, materials and labour, building stages, condition and repairs, **fire that can spread down the street,** a firebreak, and **salvaging or burning an empty house** with the street's consent.
**You test:**
- Upgrades visibly matter (insulation saves firewood)
- Building takes materials, people and time
- A fire is frightening, and neighbours helping makes a real difference
- Burning an empty house for fuel feels like a cost

### Step 9: The pump, dilemmas and the storyteller
**Builds:** street meetings at the pump, one shared work that needs everyone (with someone shirking), 10 dilemma cards with remembered consequences, street standing, the storyteller, **fever and childbirth,** one apprentice example, deserters, and disasters announced ahead. Debug panel: trigger any event; show standing and the storyteller's pressure.
**You test:**
- The choices feel hard, and neighbours react later to what you chose
- You can lose a vote at the pump, and it matters
- Disasters are warned about, and preparing makes a difference
- When you're struggling, the game eases off; when you're comfortable, it pushes

### Step 10: One year, one real winter
**Builds:** the opening with the last train, onboarding through a neighbour, the season goals panel, the end-of-winter summary, save and load, a balancing pass, and fixes from your play sessions.
**You test:**
- A new player understands what to do without reading a manual
- There's always a clear next goal
- The summary tells the story of the winter
- **The real test: when the winter ends, do you argue with yourself about whether you did the right thing?**
