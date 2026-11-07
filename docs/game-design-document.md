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
- Upgrade your house: insulation, stove, roof, root cellar, well, workshop
- Build outbuildings: woodshed, coop, greenhouse, smokehouse
- Repair damage after disasters
- **Shared projects with neighbours:** a second well, a watch rota, a shared greenhouse, rebuilding a burnt house

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
- 1 player household + 4 neighbour households
- Needs, traits, memory and relationships
- 4 seasons and one hard winter
- Bartering
- 10 dilemma events
- A basic storyteller

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
