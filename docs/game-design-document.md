# The Last Street — Game Design Document

**Version:** 0.2 (the street as one body)
**Last updated:** 2026-11-08
**Status:** Pre-production. The simulation is built through prototype step 4. The game is moving to Unreal.

> *In a cold northern land at the turn of the century, one street must survive the winters together.*

---

## 1. Overview

| | |
|---|---|
| **Genre** | Survival / community builder with neighbour simulation |
| **Setting** | A fictional cold northern country, around 1900 |
| **Scale** | One single street: 12 plots, about 6 households (~25 people) still living there, the rest already empty |
| **Player role** | One household on the street |
| **Camera** | Strategy-style overhead view that zooms smoothly down to street level and into homes |
| **Look** | Realistic historical simulation (Unreal Engine 5), natural light, weathered materials |
| **Platforms** | Steam (PC + Mac) first; high-end iOS later if the game succeeds |
| **Business model** | Premium (Steam ~$14.99–19.99), DLC later. Mobile: free first season, one-time unlock |

### The hook

Most survival games are about you collecting enough stuff. In **The Last Street, nobody can survive alone, by design.** No household can cover everything it needs, so your neighbours are your main survival system, and every person who dies takes something away for good. The last train has stopped, and everything from the old world is running out. The street has to learn to make what it needs before it's gone.

---

## 2. Design Pillars

1. **Nobody survives alone.** Every household holds a piece of what the street needs. Your neighbours are your survival system, and they can refuse.
2. **Every loss is permanent.** When someone dies, what they knew and could do goes with them, unless someone learned it first.
3. **The old world runs out.** Salt, iron, matches, lamp oil and medicine only ever go down. The long game is replacing each one before it's gone.
4. **Winter is the boss.** The seasons are a clock. Every choice in spring, summer and autumn is tested in winter.
5. **Hard choices, no right answers.** Dilemmas trade your family's safety against your neighbours'. Consequences are remembered.
6. **One street, seen up close.** The camera moves from the whole street to a family by the stove. Zooming in should always show something happening.
7. **The street shows what happened.** Gardens grow, houses empty, some are burned for fuel, people are born and lost. The street's shape tells your story.

---

## 3. Setting

### Place
A **fictional northern country** inspired by cold rural regions (Scandinavia, Eastern Europe, Canada) without being any of them. The country, its towns and its traditions are invented, which gives freedom and avoids real-world politics.

### Era
Begins **around 1900.** No electricity on the street. Hand water pumps, wood stoves, oil lamps, horses and carts. A railway line passes nearby and links the street to the outside world, until the off-screen war stops the trains.

### The street
- 12 timber and stone houses facing each other along a muddy dirt road, about half still lived in when the game begins
- Fenced yards with vegetable gardens, sheds, chicken coops, woodpiles
- A shared **hand water pump** in the middle of the street (a natural gathering point)
- Birch and pine forest around the edges, fields beyond
- The **railway line** in the distance: trade goods and news arrive by train, until the **last train** stops coming

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

### Seeing inside houses (decided)
When a character goes indoors, the player must be able to see them from the top-down camera. Houses are therefore built as **cutaway models:**
- The **roof** (with the gables, chimney top and loft floor) is a separate piece that fades away when the camera is close to a house, or when a person inside it is selected or followed.
- The **upper part of the walls** can lower to a cutaway height, so the rooms read clearly from above, like a dollhouse.
- Every house has a **real interior:** rooms, the big stove under the chimney, table and benches, beds, storage, and a ladder to the loft. People inside are seen eating at the table, sleeping, by the stove or preserving food.
- From the high strategic view, roofs stay on and the street looks normal.

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
- **The family changes:** children grow into workers, people fall ill or die, an orphan may be taken in, a son is called to war before the last train. Losing a family member hurts but play continues while anyone in the household survives.

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
- **Street standing:** alongside individual relationships, each household (including the player's) has a street-wide reputation built from how it has treated the street as a whole: sharing in hard times, joining street projects, keeping promises, taking in the needy. Standing decides how much weight a household's voice carries in street decisions such as plot takeovers (§7.4), and how others react to its requests. Hoarding, cheating and taking advantage of others' misfortune lower it, and it recovers slowly.
- **Their own life:** routines, work, visits, arguments, celebrations, births and deaths, whether or not the player is watching.

**Dialogue** comes from written templates filled in from simulation state, not from a live language model. That keeps it offline, consistent and cheap. For example: *"After what you did in the flood, [name] won't open the door."*

### 7.3 Trading
- A barter economy: food, firewood, tools, cloth, medicine, labour and favours
- Value depends on season and scarcity (firewood is cheap in summer and precious in January)
- Neighbours' willingness and prices depend on relationship, traits and their own needs
- The train brings outside goods for coin, while it runs. **After the last train, coin is worthless,** and the street's currency is goods and **favours that people remember** (§7.11)

### 7.4 Building
The street already exists when the game starts. The player does not lay out a town; they **improve their own plot, keep it standing, and build with the street.** Building is about survival, not decoration.

#### Your plot
- The player's household owns one plot: the house, a yard, a garden and space at the back.
- Outbuildings and gardens are **placed on a snapping grid** within the plot.
- The plot can only grow by taking over another plot, and that is **never free.** It needs leverage and the street's consent (see *Taking over a plot* below).

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
This is **the last street.** Every house that goes cold is one less family, one less pair of hands, one step closer to the street dying. The street feels that, and so should the player.

When a neighbour household dies out or leaves, its house stands empty. The choices:
- Take in a **refugee family,** if one comes down the road (new neighbours with their own traits, skills and history). The street usually prefers this, but it means more mouths.
- **Take over** the plot to expand your own (hard; see below)
- **Salvage** it for materials, or **burn it for fuel in a hard winter** (needs the street's consent, just like a takeover). Every empty home is both firewood and a grave, and the street visibly shrinks as you consume it
- Leave it to decay as a visible scar on the street

#### Taking over a plot
A player **cannot simply claim** a plot. It needs both a **claim** and the **street's consent.**

**1. A claim: a legitimate reason it should be yours.** At least one of:

| Claim | How it's earned |
|---|---|
| **Purchase** | A family that is leaving agrees to sell. They must trust you, and you need coin or goods they value. |
| **Debt** | The household owes you heavily. Collecting this way is legal but harsh, and the street will judge it. |
| **Kinship** | A family tie: an orphan from that house you took in, a marriage between households, an elder who names you their heir. |
| **Caretaking** | You kept the empty house standing (heated, repaired, protected) for at least a season, with the street's knowledge. |
| **Street grant** | The street gives it to you, for example in thanks for getting them through a disaster. |

**2. Consent: the street must accept it.**
- The remaining households weigh in, based on their relationship with you, your **street standing** (§7.2) and their traits. A proud or grudge-holding neighbour may object on principle; a family you saved will back you.
- A majority must accept. Elders' opinions count for more.
- Even with consent, a takeover **costs standing with some households.** Expanding while others struggle is noticed.

**3. No claim or no consent means no takeover.** There is no way to simply grab land. Pushing for it anyway (pressing a weak claim, calling in a debt from a grieving family) is a dilemma in its own right, with lasting consequences.

**Why it's built this way:** in a dying street, land is not the scarce resource. **People** are. Expansion should feel rare, earned and morally weighted, and the game should constantly ask: *is this house worth more to you, or to a family who could live in it?*

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
15. The seed grain is all that's left. Eat it now, or go hungry to have a harvest next year?
16. The last sack of salt: who gets it, and who loses their winter meat?
17. The last good axe head breaks, or nearly does. Who gets the one that's left?
18. A family's house has the fever. Board them in to save everyone else?
19. The stove has died next door. Take them in, with their stores and their grudges?
20. Tear down a house to make a firebreak while the street burns. Whose?
21. Deserters at the end of the street, hungry and armed. Feed them, turn them away, or give them a place?
22. Someone has been stealing from the common store, and you've known them for ten winters.
23. Food is short, and two young people want to marry. Do you spend grain on a wedding?
24. A privy is fouling the well. The family who owns it refuses to move it.

### 7.8 Winning and losing
**You win when the street no longer needs the outside world and a child born there grows up** (§7.12 and §8). Then a train finally arrives, and the street decides whether to let it in.

The street can also fail: too few people or skills remain for anyone to get through another winter.

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
- **Long (this year and beyond):** "Build the root cellar." "Earn the Petrovs' trust." "Find a salt route before the sacks run out."

Shown in a simple season goals panel.

#### Difficulty curve
- Year 1: a mild winter to learn from
- Years 2–3: proper winters and the first disasters
- Story acts raise pressure (the last train, then shortages, then sickness)
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

### 7.10 The street as one body
No household can cover everything. Each one holds a piece of what the street needs, and a new game reshuffles who holds what, so replays play differently.

| Kind of thing held | Examples |
|---|---|
| **Places** | The only good well, the best cellar, the smokehouse |
| **Animals** | The cow, the horse, the breeding sheep |
| **Stocks** | The seed grain, the salt, the last tools |
| **Skills** | Mending iron, carpentry, midwifery, fevers and herbs, weaving, hunting |

**Losing someone costs the street what they held.** If the only person who can mend iron dies, broken tools stay broken. If one old woman is the only healer, then when she goes, so does what she knows.

**Knowledge can be passed on, at a price.** An apprentice costs labour you can't spare today. Sending your daughter to learn from the old healer means one less pair of hands on the woodpile, and it only pays off if she survives.

A person's value to the street is always visible on the relationship map, so the player can see who the street can't afford to lose.

### 7.11 Favours and the pump
- Money dies with the last train. **A ledger of favours, kept by each household,** replaces it. Who owes whom, for what, and whether it was paid back.
- **The shared works** need everyone: the firebreak, the ice house, the common grain store, the night watch. Someone always shirks.
- **Street meetings at the pump** decide shared questions: rationing, quarantine, who gets the last of something, whether to let strangers stay. The player has one voice, and its weight comes from street standing (§7.2). You persuade; you never command.

### 7.12 Run out, then replace
The last train starts a countdown on everything the street can't make. Each item has a way to replace it, and each replacement needs a skill, an animal, a plot of land or a journey.

| From the old world | What replaces it |
|---|---|
| Lamp oil | Tallow candles, which need animals |
| Matches | Keeping fires alive, and flint and steel |
| Iron and nails | Charcoal and a working forge, reusing scrap; wooden pegs |
| Salt | A salt route, which means a dangerous journey and trade with strangers |
| Cloth | Flax and sheep, spinning and weaving |
| Medicine | Herbs, and someone who learned how to use them |
| Seed | Saving your own, every year, however hungry you are |
| Glass | Shutters and oiled cloth |

The street's self-sufficiency is a visible checklist. **Filling it is the long goal of the game.**

### 7.13 What survival really demands
These join food and firewood as simulated pressures.

| Need | The real problem | The hard choice |
|---|---|---|
| **Heat** | Winter's wood must be cut and dried in summer | Cut wood or tend crops? |
| **Water** | Privies foul wells, and boiling burns firewood | Whose well is safe? Who moves their privy? |
| **Food** | Harvests come once a year; stores rot or get rats | Eat the seed grain, or starve now to eat next year? |
| **Salt** | Nobody can make it, and without it there's no meat for winter | Who gets the last sack? |
| **Tools** | Every break is permanent without a smith | Who gets the last good axe head? |
| **Medicine and births** | Childbirth and fever are the big killers | The only quinine, your child or theirs? |
| **Fire** | A wooden street can burn in one night | Tear down a house to make a firebreak? |
| **Safety** | Deserters and refugees arrive hungry | Feed, turn away, or take them in? |
| **Hope** | Despair stops people working | Spend scarce food on a wedding? |

---

## 8. Story Mode: The Five Acts

The war is **fictional** and **always off-screen.** It's the reason the trains stop, and it never comes onto the street as a battle. No combat systems: threats are settled by the watch, bargains and standoffs.

The arc is **running out, then replacing it** (§7.12).

| Act | Years (in-game) | What changes |
|---|---|---|
| **1. The Last Train** | 1–2 | The train comes less and less, then stops. Learn the seasons and the neighbours. It's the last chance to buy salt, nails and medicine with coin. A mild first winter. |
| **2. Running Down** | 3–5 | Money is worthless. Stocks run out one at a time. The first deaths take skills with them. Deserters and refugees come down the road. |
| **3. The Sickness** | 6–7 | Fever comes, and the wells may be why. Quarantine or nurse? The hardest winter of the game. |
| **4. Making Do** | 8–11 | The street replaces what it lost, or doesn't: candles, charcoal, flax and sheep, a salt route. |
| **5. The Train** | 12+ | The first child born after the last train grows up. Then a train finally arrives, and the street decides whether to let it in. |

Target length: roughly 15–25 hours. Later, quieter years can pass faster so the ending stays within reach (see §14).

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
| Engine | **Unreal Engine 5,** newest stable 5.x (Lumen lighting, Nanite geometry), developed on Windows |
| Gameplay code | **C++,** with game rules kept in the engine-free `core/`. Blueprints only for thin visual hookups. Unreal 6 is reported to drop Blueprints and Actors, so the less game logic depends on them, the better. |
| Game data | Plain text (JSON/CSV): neighbours, traits, events, items, seasons, balance values |
| Level building | **Python editor scripts** to generate terrain, place houses and scatter foliage |
| Architecture | Simulation logic kept separate from visuals, so it can be built and balanced on its own |
| Version control | Git with Git LFS for large binary assets |

**Development machine:** the Windows PC (i5-13600KF, RX 6700 XT, 16 GB RAM, more RAM later if the editor struggles). Epic Games Launcher + UE5 needs about 100 GB free disk.

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

### Phase 1: One year on the street
Prove the fun with **one full year through one real winter:** six households in Unreal, placeholder people where needed.
- 1 player household (family of four) + 5 neighbour households, about 25 people
- Household control: daily plan, jobs, direct orders (§6)
- Needs, traits, memory, relationships and street standing
- Each household holding a piece of what the street needs, and permanent loss (§7.10)
- Favours, barter, and meetings at the pump (§7.11)
- Water, salt, tools and the finite stocks, starting with the last train (§7.12, §7.13)
- Building: a few house upgrades and outbuildings, repairs, one shared work (§7.4)
- 10 dilemma events, fire, fever and a basic storyteller
- Season goals panel and an end-of-winter summary (§7.9)

**Not in the prototype:** the replacement chains beyond one example, passing on skills beyond one example, the later acts, endless mode, carry-over unlocks, difficulty settings.

**Success:** when that winter ends, people argue about whether they did the right thing. If it doesn't do that, more content won't save it.

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
- [x] Development machine for Unreal 5: the Windows PC (§10)
- [ ] How quickly years pass, so a child can grow up within the target length (§8)
- [ ] The age at which a child born on the street counts as grown (§7.8)
- [ ] The fifth neighbour household, and who holds what by default (§7.10)

### Name shortlist
**Country:** Varnholm · Kaldria · Norrvik · Velmark · Hollvaen
**Street:** Birch Row · Pump Lane · Railway Street · Ember Lane · Kettle Row
