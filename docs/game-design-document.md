# The Last Street — Game Design Document

**Version:** 0.1 (planning)
**Last updated:** 2026-11-07
**Status:** Pre-production. Nothing built yet.

> *In a cold northern land at the turn of the century, one street must survive the winters together.*

---

## 1. Overview

| | |
|---|---|
| **Genre** | Survival / community builder with neighbour simulation |
| **Setting** | A fictional cold northern country, around 1900 |
| **Scale** | One single street, ~12–15 households |
| **Player role** | One household on the street |
| **Camera** | Strategy-style overhead view that zooms smoothly down to street level and into homes |
| **Look** | Realistic historical simulation (Unreal Engine 5), natural light, weathered materials |
| **Platforms** | Steam (PC + Mac) first; high-end iOS later if the game succeeds |
| **Business model** | Premium (Steam ~$14.99–19.99), DLC later. Mobile: free first season, one-time unlock |

### The hook

Most survival builders put you alone in the wilderness. **The Last Street** puts you on one street with neighbours you know by name. The seasons and disasters are the pressure. The relationships are the game.

---

## 2. Design Pillars

1. **The neighbours are the game.** Every household has needs, personality and memory. They remember what you did.
2. **Winter is the boss.** The seasons are a clock. Every choice in spring, summer and autumn is tested in winter.
3. **Hard choices, no right answers.** Dilemmas trade your family's safety against your neighbours'. Consequences are remembered.
4. **One street, seen up close.** The camera moves from the whole street to a family by the stove. Zooming in should always show something happening.
5. **The street changes over time.** Gardens grow, houses empty, people are born and lost. After twenty years the street tells your story.

---

## 3. Setting

### Place
A **fictional northern country** inspired by cold rural regions (Scandinavia, Eastern Europe, Canada) without being any of them. The country, its towns and its traditions are invented, which gives freedom and avoids real-world politics.

### Era
Begins **around 1900.** No electricity on the street. Hand water pumps, wood stoves, oil lamps, horses and carts. A railway line passes nearby and links the street to the outside world.

### The street
- ~12–15 timber and stone houses facing each other along a muddy dirt road
- Fenced yards with vegetable gardens, sheds, chicken coops, woodpiles
- A shared **hand water pump** in the middle of the street (a natural gathering point)
- Birch and pine forest around the edges, fields beyond
- The **railway line** in the distance: trade goods arrive by train, and the train is how news (and the war) reaches the street

### Architecture (decided)
A **blend of northern influences**, so the country feels like its own place rather than a Russian village. Keep the log houses and carved window frames from the concept art, and mix in painted wood (deep red, ochre), stone foundations, shingle or turf roofs, and a few invented decorative details unique to this country.

### Mood (decided)
**Hard but hopeful.** Losses hurt and winters are dangerous, but kindness pays off and the street can come through together.

---

## 4. Art Direction

### Reference images
Saved in [`reference/concept-art/`](../reference/concept-art/):

| File | Shows | Use as reference for |
|---|---|---|
| `05-summer-aerial.jpg` | The street in late summer, busy, muddy, gardens full | Strategic camera, summer palette, street layout |
| `01-winter-aerial.jpg` | The same street in deep winter, few chimneys smoking | Winter palette, snow coverage, mood |
| `03-autumn-trade-at-gate.jpg` | Neighbours trading potatoes for firewood at a gate | Close camera, characters, clothing, autumn palette |
| `04-spring-flood.jpg` | Neighbours sandbagging during a flood | Disaster events, community moments |
| `02-key-art-split.jpg` | Summer/winter split with title | Marketing direction (AI-generated, see §13) |

**Still needed:** a family huddled by the stove at night (interior close-up), and clean front/side/close references of single houses for 3D generation.

### Visual rules
- **Lighting does most of the work:** soft sun, long shadows, golden hour, believable sky.
- **The ground changes:** grass wears into mud paths where people walk; puddles after rain; snow builds up over days rather than switching on.
- **Nothing looks new:** worn wood, damp stone, sagging roofs.
- **Muted, earthy palette;** colour comes from light, seasons and clothing.
- **Atmosphere:** mist, fog, chimney smoke drifting with the wind, visible breath in winter.
- **Readable state:** a cold chimney means no fuel; closed curtains mean sickness or low morale; a black ribbon on a door means a death.

### Season palettes
| Season | Feel |
|---|---|
| Spring | Wet, grey-green, mud, snowmelt, first shoots |
| Summer | Warm gold light, deep greens, busy street |
| Autumn | Orange birch leaves, overcast, harvest, woodpiles growing |
| Winter | Cold blue, white, warm orange windows as the only colour |

---

## 5. Camera

Three zoom levels that blend into each other with no loading screens:

| Level | View | Shows |
|---|---|---|
| **Strategic** (high) | The whole street and surroundings | Building placement, resource overlays, weather fronts, relationship map |
| **Neighbourhood** (mid) | A few houses | People walking and working, trades, smoke, animals |
| **Intimate** (close) | One house or person, and through windows into interiors | Faces, conversations, families by the stove, snow on windowsills |

Details increase as the camera gets closer: depth of field, ambient sound and character detail all change with zoom.

---

## 6. Core Loop

### Player control (decided)
The player controls **their own household** (3–6 people), not one character and not the whole street. Neighbours are independent AI and can only be influenced through trade, help, conversation and dilemmas. Neighbours must never be directly controllable, because their ability to refuse, remember and betray is what makes them people.

Three levels of control, from hands-off to hands-on:

| Level | What it is | Example |
|---|---|---|
| **1. Daily plan** (automatic) | Each family member has a role and routine and carries on sensibly if left alone | Father chops wood mornings and repairs afternoons; grandmother cooks and minds the child |
| **2. Jobs** (main decisions) | Reassign people at the start of a day or any time | "Everyone on firewood this week." "Send Anna to help the Petrovs with their roof." |
| **3. Direct orders** (urgent moments) | Select a family member, then a target | "Trade at the Petrovs' gate." "Grab a bucket, the shed is on fire!" |

- **Dilemmas** appear as event cards with choices. The game pauses and the camera moves in on the moment.
- **Pause anytime** to give orders; speed controls for quiet stretches.
- **Input** is select-then-target, which works for both mouse and touch.
- **Follow camera:** any person (family or neighbour) can be followed up close.
- **The family changes:** children grow into workers, people fall ill or die, an orphan may be taken in, a son is called to war in Act 3. Losing a family member hurts but play continues while anyone in the household survives.

#### A typical minute
1. Zoomed out: the woodpile is low, a storm is forecast in 3 days, the Petrovs' chimney is cold.
2. Assign two family members to firewood and one to check on the Petrovs.
3. Speed up time and watch the street.
4. An event: the Petrov grandfather is sick and they're out of wood. Give, trade, or walk away?
5. Zoom in to watch the choice play out. The Petrovs remember.
6. Zoom out and get back to preparing for the storm.

### Season loop
- **Spring:** repair winter damage, plant, trade surplus, welcome or lose neighbours. Threat: floods.
- **Summer:** build, upgrade, stockpile. Threats: drought, heat, fire.
- **Autumn:** harvest, preserve food, gather and chop firewood, insulate, make final trades.
- **Winter:** the test. Fuel and food run down; cold, sickness and isolation set in. Your preparation is judged.

### Moment-to-moment
1. Check your household's needs and stores
2. Assign family members to tasks (garden, chop wood, repair, trade, help a neighbour)
3. Respond to events and dilemmas
4. Build or upgrade
5. Watch the street live and react

### Time
- One in-game day is roughly 3–5 real minutes at normal speed (also suits mobile later)
- Pause anytime; speed controls; constant autosave

---

## 7. Systems

### 7.1 Household needs
Every household (the player's and each neighbour's) tracks:
- **Food:** stored and daily consumption
- **Warmth:** fuel stock, house insulation, outside temperature
- **Health:** sickness, injury, age
- **Safety:** house condition, disaster exposure
- **Morale:** hope, grief, conflict

### 7.2 Neighbours
Each neighbour household has:
- **Members:** names, ages, roles (parents, children, elderly)
- **Traits:** 2–3 per adult, for example *generous, proud, paranoid, lazy, devout, grudge-holder, skilled carpenter, healer, gossip, kind to children*. Traits affect how they trade, help and react.
- **Memory:** a log of significant things the player (and other neighbours) did. *"Gave us wood during the blizzard." "Charged triple when our child was sick." "Refused to help sandbag."* Memories fade slowly, but big ones last forever.
- **Relationship score with every other household,** not just the player, so neighbours can fall out with each other too.
- **Their own life:** routines, work, visits, arguments, celebrations, births and deaths, whether or not the player is watching.

**Dialogue** comes from written templates filled in from simulation state, not from a live language model. That keeps it offline, consistent and cheap. For example: *"After what you did in the flood, [name] won't open the door."*

### 7.3 Trading
- A barter economy: food, firewood, tools, cloth, medicine, labour and favours
- Value depends on season and scarcity (firewood is cheap in summer and precious in January)
- Neighbours' willingness and prices depend on relationship, traits and their own needs
- The train brings outside goods for coin, while it runs

### 7.4 Building
The street already exists when the game starts. The player does not lay out a town; they **improve their own plot, keep it standing, and build with the street.** Building is about survival, not decoration.

#### Your plot
- The player's household owns one plot: the house, a yard, a garden and space at the back.
- Outbuildings and gardens are **placed on a snapping grid** within the plot.
- The plot can **grow** by buying or claiming a neighbouring plot (for example when a family leaves). Usually this comes through a dilemma.

#### House upgrades
The house itself improves through an **upgrade tree, not freeform room building** (far cheaper to build and to animate, and readable from every zoom level). Each upgrade changes the model visibly.

| Part | Upgrades | Effect |
|---|---|---|
| Walls | Draughty logs → chinked logs → insulated (moss, clay, boards) | Less heat loss |
| Roof | Leaking shingles → repaired → tight double layer | Less heat loss, survives storms and snow load |
| Stove | Open hearth → brick stove → large tiled stove | Heat per log of firewood |
| Windows | Bare → shutters → double glazing (glass from the train) | Heat loss, light, morale |
| Cellar | None → root cellar → lined cellar | Food storage, less spoilage |
| Extension | One room → second room → upper loft | Space for more family members or a taken-in neighbour |

#### Outbuildings (placed in the yard)
Woodshed (keeps firewood dry), chicken coop, smokehouse (preserves meat and fish), workshop (repairs and tools), greenhouse (early and late crops), well (private water), stable (if you own a horse).

#### How construction works
- **Materials:** timber and logs (from the forest edge, which visibly recedes if over-cut), stone (fields and the river), clay and moss, plus nails, glass and iron tools that only come by **train or trade.**
- **Labour:** assign family members. Each project needs a number of work-hours, with faster progress for skilled workers.
- **Time and weather:** frozen ground blocks foundations; rain and mud slow work; deep winter allows only indoor and emergency work. Building happens mainly from spring to autumn.
- **Visible stages:** foundation → frame → walls → roof → finished, so progress shows on the street.
- **Neighbour help:** with good relationships, neighbours join in (a barn-raising), which speeds work. Helping them build earns the same favour back.

#### Condition and repair
- Every house, the player's and the neighbours', tracks condition per part (roof, walls, stove and chimney, windows).
- Condition **drops over time and from disasters:** snow load, storms, floods, chimney fires.
- Poor condition means more heat loss, leaks, fire risk and sickness.
- Repairs cost materials and labour. Visibly decaying neighbour houses tell the player who is struggling.

#### Empty houses
When a neighbour household dies out or leaves, its house stands empty. That creates choices:
- Help a **new family** move in (a new neighbour with their own traits and history)
- **Take over** the plot to expand your own
- **Salvage** it for materials (others on the street will have opinions)
- Leave it to decay as a visible scar on the street

#### Street projects (shared)
Built on common land by several households pooling materials and labour: a second well, a street greenhouse, a firebreak, a flood wall, a watch hut, a meeting hall, rebuilding a burnt house. Each needs other households to commit, which depends on trust. Some neighbours will contribute less than they promised.

### 7.5 Seasons and weather
- Temperature, snowfall and rain simulated over the year
- Snow accumulates gradually; mud forms from rain and foot traffic
- Crops depend on season, weather and care
- Fuel use rises with cold and falls with insulation

### 7.6 The storyteller
A director system that paces events based on how the street is doing:
- When things are comfortable, it brings pressure (an early winter, a fire)
- When things are desperate, it eases off or brings luck (a neighbour with surplus, a mild week)
- It aims for tension followed by relief, like episodes of a TV series
- **Disasters are always announced ahead** where plausible ("a storm in 3 days") so they feel fair

### 7.7 Dilemmas (starting list)
1. A neighbour knocks at night during a blizzard with no food. Let them in?
2. Enough medicine for your child or the neighbour's child, not both.
3. The street pools grain for winter; you catch someone taking extra.
4. A flood is coming: sandbag the neighbour's house or reinforce your own?
5. An orphaned child is left alone after a death. Take them in?
6. A neighbour asks to borrow your horse; theirs died. You need yours for firewood.
7. A stranger arrives off the train asking for shelter. Is it safe?
8. The old man at the end of the street hasn't been seen in days.
9. Sickness in one house: quarantine them or nurse them?
10. A neighbour offers to sell you their land cheap because they're leaving.
11. Someone's chimney fire spreads toward your roof.
12. You find a neighbour stealing firewood from your shed.
13. The street wants to send a young man to the city for supplies through the snow.
14. A letter arrives for a neighbour who is away. It's bad news. Do you open it?

### 7.8 Losing
Family members can die. If the player's household is lost (decided):
- **Story mode:** the game ends with a **summary of the story:** who helped, who betrayed you, what the street remembers
- **Endless mode:** the player continues as a surviving neighbour

### 7.9 Progression
For a first-time player, the game starts small and opens up gradually, so there is always a next goal and each year the household is stronger than the last.

#### A gentle first year (onboarding)
Systems arrive with the seasons, taught **by the neighbours** rather than pop-up boxes.

| When | What the player learns | How it's introduced |
|---|---|---|
| Spring, year 1 | Moving, needs, planting | A friendly old neighbour shows you around |
| Summer | Chopping wood, stockpiling, simple building | "Winter is long here. Start your woodpile now." |
| Autumn | Trading, preserving food | The first trade at the gate |
| Winter, year 1 | Fuel, warmth, sickness | A *mild* first winter: felt, but survivable |
| Year 2 | Dilemmas, the storyteller, relationships | The first real dilemma: a knock at the door at night |

#### Layers of progress
- **The house:** visible upgrades from draughty cabin to a warm, sturdy home (§7.4)
- **Family skills:** people improve at what they do often (wood, trade, healing, building); the grandmother can teach remedies
- **Relationships:** stranger → acquaintance → friend → family-like. Deeper levels unlock shared tools, borrowing a horse, help in disasters
- **The street:** shared projects visibly improve the street (§7.4)
- **Player knowledge:** players learn the signs (geese leaving early means a hard winter; a red sky means a storm)

#### Goals at three scales
- **Short (today):** "Finish the woodshed roof before the rain."
- **Medium (this season):** "Stockpile 40 days of firewood before winter."
- **Long (this year and beyond):** "Build the root cellar." "Earn the Petrovs' trust." "Get everyone through the war."

Shown in a simple season goals panel.

#### Difficulty curve
- Year 1: a mild winter to learn from
- Years 2–3: proper winters and the first disasters
- Story acts raise pressure (rumours → war → sickness)
- The storyteller adjusts: breathing room for struggling players, tests for comfortable ones

Target feeling: *"Last year I barely made it. This year I'm ready."* Then something new arrives.

#### Rewards
- The **end-of-winter summary:** who survived, what was built, who helped, how the street changed
- The street visibly changing over the years: gardens, repairs, new families, children growing up
- Small moments: a gift left at the door after helping someone; children playing in the snow when morale is high

#### Carry-over between games (mainly endless mode)
- Unlock new starting conditions: a different family, a harsher region, a poorer start
- Unlock new neighbour types and events
- A **street chronicle:** a shareable record of past streets and their stories

#### Never lost, never unfairly punished
- Warnings before trouble: "Firewood will last 9 more days at this rate."
- Losses that teach: the summary explains why ("the stove went cold on day 12")
- Difficulty settings: **Story** (gentler, narrative-focused) and **Harsh** (for survival fans)

---

## 8. Story Mode: The Five Acts

The war is **fictional** and **always off-screen.** It happens *to* the street, never *on* it. No combat systems.

| Act | Years (in-game) | What changes |
|---|---|---|
| **1. The Good Years** | ~1900–1905 | Peaceful but hard winters. Learn farming, building, trading, the neighbours. |
| **2. Rumours** | ~1906–1910 | Newspapers speak of tension abroad. Prices rise; the train comes less often. A young neighbour enlists. |
| **3. The War Years** | — | Men are called away; households left with women, elderly and children. Rationing, scarcity, letters from the front, some with bad news. |
| **4. The Sickness** | — | A pandemic sweeps through. Quarantine your family or nurse your neighbours? The hardest winter of the game. |
| **5. Whoever Is Left** | — | The war ends. Survivors come home, changed. The street rebuilds, or doesn't, depending on everything that came before. |

Target length: roughly 10–20 hours.

### Endless mode
A sandbox with random seasons, disasters and neighbour generation, for replayability (in the spirit of long-running colony sims).

---

## 9. Platforms and Business

- **Steam (Windows + Mac) first.** Premium price ~$14.99–19.99. Possible Early Access.
- **Steam Deck** support as a stretch goal.
- **iOS (high-end devices) later,** only if the Steam launch succeeds: free first season, one-time unlock (~$6.99–9.99).
- **No** energy timers, loot boxes or pay-to-survive.
- **DLC ideas:** new streets or regions, new disaster packs, new story chapters.

---

## 10. Technology

| Area | Choice |
|---|---|
| Engine | **Unreal Engine 5** (Lumen lighting, Nanite geometry) |
| Gameplay code | **C++.** Blueprints only for thin visual hookups. |
| Game data | Plain text (JSON/CSV): neighbours, traits, events, items, seasons, balance values |
| Level building | **Python editor scripts** to generate terrain, place houses and scatter foliage |
| Architecture | Simulation logic kept separate from visuals, so it can be built and balanced on its own |
| Version control | Git with Git LFS for large binary assets |

**Hardware/space needed:** Epic Games Launcher + UE5 (~100 GB free disk).

---

## 11. Asset Pipeline

| Asset type | Source |
|---|---|
| Ground, rocks, trees, grass, foliage | **Fab** photo-scanned assets |
| Houses, sheds, fences, props, furniture | **AI 3D generation** (Tripo for game-ready meshes; Rodin for hero detail; Meshy as alternative) from reference images. Static props go straight into Nanite with little cleanup. |
| Villagers (models) | AI-generated base + auto-rigging; a freelancer later for hero close-up characters |
| Villager animation | Phone video → **Move.ai / Rokoko Vision** → cleanup in **Cascadeur** |
| Concept art | AI image generation (Midjourney / GPT Image / Flux) as reference |

Meshy and Tripo have APIs, so generation → download → Unreal import can be scripted.

**Budget estimate (early):** ~$15–60/month in AI tool subscriptions plus ~$100–500 in Fab assets for the look test.

---

## 12. Development Plan

### Phase 0: Look test (~1–2 months)
Prove the look and camera in Unreal 5:
- One street with 3–4 houses (Fab + AI assets)
- The zoom camera from strategic down to street level
- Day–night cycle
- Summer-to-winter transition with snow building up
- A few villagers walking around

**Success:** it looks like the concept art from above, and the zoom feels great.

### Phase 1: Gameplay prototype (~4–6 weeks)
**Simple shapes only** (boxes for houses, capsules for people). Prove the fun:
- 1 player household (family of four) + 4 neighbour households
- Household control: daily plan, jobs, direct orders (§6)
- Needs, traits, memory and relationships, with relationship levels
- 4 seasons, a mild first winter and a hard second winter
- Bartering
- Building: 4–5 house upgrades, 3 outbuildings, repairs and condition, one street project (§7.4)
- 10 dilemma events
- A basic storyteller
- First-year onboarding through a neighbour, season goals panel, end-of-winter summary (§7.9)

**Not in the prototype:** family skills, carry-over unlocks, difficulty settings, empty-house choices, story acts.

**Success:** playing it, you want "one more winter." If it isn't fun with boxes, realistic art won't fix it.

### Phase 2: Vertical slice (~3 months)
One full year on the street at near-final quality. Steam page goes live; devlog clips begin.

### Phase 3: Content (~6+ months)
All systems, the five acts, endless mode, a Steam Next Fest demo.

### Phase 4: Launch
Steam (possibly Early Access). Mobile 6–12 months later if it succeeds.

---

## 13. Marketing

- **Signature shot:** a time-lapse from bright summer to winter, chimneys going dark one by one, then the camera zooms into one window: a family around a single candle.
- **Steam page early;** collect wishlists. ~7,000–10,000+ at launch is a healthy target.
- **Weekly devlog clips** for TikTok, YouTube Shorts and Reddit. The zoom camera is built for short video.
- **Steam Next Fest** with a polished one-winter demo.
- **AI disclosure:** Steam requires disclosing AI-generated content players see or hear, including store art. Prefer real in-game screenshots or an artist-made capsule for the store; keep AI art as internal reference.

### Tagline (decided)
**"No one is coming. We only have each other."**

Alternatives for trailers and other uses:
- *Survive the winter. Together, or not at all.*
- *One street. Every winter.*
- *One street. Twenty years. A war that never reaches it, and changes everything.*

---

## 14. Open Decisions

- [x] Architecture: a blend of northern influences (§3)
- [ ] Name of the fictional country and of the street (shortlist below)
- [x] Tagline: "No one is coming. We only have each other." (§13)
- [x] Mood: hard but hopeful (§3)
- [x] Losing: story mode ends with a summary; endless mode continues as a neighbour (§7.8)
- [ ] Trademark check on "The Last Street" before launch
- [ ] Development machine for Unreal 5. The current MacBook Air (M1, 8 GB RAM, ~54 GB free, no admin rights) is not enough for UE5.

### Name shortlist
**Country:** Varnholm · Kaldria · Norrvik · Velmark · Hollvaen
**Street:** Birch Row · Pump Lane · Railway Street · Ember Lane · Kettle Row
