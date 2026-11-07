# Asset Reference Prompts

Prompts for generating **reference images** in ChatGPT for every asset in The Last Street. These images are not the assets themselves:

- **Buildings and props** are built by Claude in Blender with code, using these images as the visual target.
- **People and animals** go through image-to-3D (Tripo, Meshy or Rodin) and Mixamo, so their images must be clean character sheets.
- **Ground, rocks and trees** come from Fab, so they need no prompts here.

Save each image in `assets/reference/<category>/` with a short name, for example `assets/reference/houses/lind-house-34.png`.

---

## How to use these prompts

1. Copy a prompt and replace `[STYLE]` with the style block below.
2. Generate until you have one you love. Then, **in the same ChatGPT conversation,** ask for the other views listed under the prompt, so they stay consistent.
3. Keep the rules: **one object, plain light-grey background, soft even light, nothing else in frame.**

### The style block (paste wherever a prompt says [STYLE])

```
in the style of a fictional cold northern country around 1900, a blend of northern European rural traditions: weathered log and timber construction, carved wooden trim, faded painted wood in deep red, ochre and muted green, grey fieldstone, wooden shingles, hand-forged iron, handmade and lived-in, realistic, muted earthy colours, isolated single object centred on a plain light grey background, soft even studio lighting with no harsh shadows, entire object visible, no people, no snow, photorealistic reference image, high detail, sharp focus
```

### Views to ask for after the first image

- **Buildings:** "Show this exact same building from straight in front, then from the left side, then from behind, orthographic views, same materials and details, same background."
- **Props:** "Show this exact same object from the front and from the side, same background."
- **Characters:** see the character section; they use a turnaround sheet instead.

---

## Priority 1: The houses (needed first)

The street has five households. Each house shares the same building tradition but differs, so the street feels real. Generate the **pilot house first** (it's the Linds', the player's home).

### 1.1 The Lind house (the player's home: the pilot)
```
Three-quarter front view of a modest one-and-a-half storey log house, about 10 metres wide and 9 metres deep, weathered dark timber log walls with notched corners, a low grey fieldstone foundation, a steep gable roof covered in weathered wooden shingles, one red-brown brick chimney, a small covered porch with two carved wooden posts at the front door, four small multi-pane windows on the front with carved wooden window frames painted faded ochre, lace curtains inside, deep red painted boards under the gable, a small attic window in the gable, [STYLE]
```

### 1.2 The Halden house (larger, better off)
```
Three-quarter front view of a sturdy two-storey log house, about 11 metres wide, well-kept dark timber walls, a tall grey fieldstone foundation with a cellar door, a steep gable roof with neat wooden shingles, a large brick chimney, a wide covered porch with a carved railing, six multi-pane windows with ornate carved frames painted deep green, a carved sunburst pattern under the gable, [STYLE]
```

### 1.3 The Voss house (poor and worn)
```
Three-quarter front view of a small single-storey log cabin, about 8 metres wide, old grey weathered logs with gaps stuffed with moss, a sagging roof with some missing and mossy wooden shingles, a crooked stone chimney, a plain door with no porch, two small windows with cracked panes and a board over one corner, simple unpainted frames, a lean-to added on one side, [STYLE]
```

### 1.4 The Petrov house (painted and proud)
```
Three-quarter front view of a one-and-a-half storey timber house, about 10 metres wide, walls clad in vertical boards painted deep red with white corner boards and trim, a grey fieldstone foundation, a steep gable roof with dark wooden shingles, a brick chimney, a small glazed porch with many small panes, multi-pane windows with white carved frames, a small carved weathervane on the roof ridge, [STYLE]
```

### 1.5 The Marek house (old and odd)
```
Three-quarter front view of an old one-and-a-half storey log house, about 10 metres wide, dark aged logs, a later stone extension on one side, a steep roof with turf and grass growing on part of it and wooden shingles on the rest, a large stone chimney, a heavy plank door with iron hinges, small deep-set windows with carved frames painted faded blue, a bench by the door, [STYLE]
```

### 1.6 House detail sheet (helps Claude model the carvings)
```
A reference sheet of architectural details from a 1900s northern log house, arranged in a grid on a plain light grey background: a carved wooden window frame with a decorative top, a notched log corner joint, a section of wooden shingle roof, a carved porch post, a carved gable board pattern, a plank front door with iron hinges, a brick chimney top, a stone foundation corner, labelled clearly, soft even lighting, realistic, high detail
```

### 1.7 Condition reference (helps Claude model upgrades and damage)
```
A reference sheet showing the same small log house wall and roof corner in four conditions side by side: 1. draughty, with gaps between the logs and missing shingles, 2. repaired, with moss and clay packed into the gaps and patched shingles, 3. well insulated, with boards over the logs and a tight double layer of shingles, 4. storm damaged, with torn-off shingles and a broken window, plain light grey background, soft even lighting, realistic, high detail
```

---

## Priority 2: The yard (needed for the look test)

### 2.1 Woodshed
```
Three-quarter view of a small open-fronted woodshed, about 4 metres wide, a timber frame with a sloped plank roof covered in wooden shingles, back and side walls of weathered vertical boards, neatly stacked split firewood filling it halfway, a chopping block with an axe stuck in it beside it, [STYLE]
```

### 2.2 Woodpile and chopping block
```
Three-quarter view of an outdoor woodpile of split firewood logs stacked in neat rows between two wooden posts, about 3 metres long and 1.5 metres high, a round tree-stump chopping block with an axe and scattered wood chips beside it, [STYLE]
```

### 2.3 Fence and gate
```
Front view of a section of a rustic wooden picket fence about 4 metres long, weathered grey hand-split pickets on two horizontal rails and round posts, with a matching small swinging gate in the middle with iron hinges and a simple wooden latch, [STYLE]
```

### 2.4 Hand water pump
```
Three-quarter view of a cast-iron hand water pump on a square grey stone base with a small stone trough, a long curved iron handle, a spout with a wooden bucket hanging from a hook, painted dark green iron with some rust, [STYLE]
```

### 2.5 Vegetable garden bed
```
Three-quarter view of a small rectangular vegetable garden bed about 6 metres by 4 metres, dark tilled soil in neat rows of cabbages, potatoes, carrots and beans, bordered by low wooden planks, a wooden rake and a watering can at the edge, [STYLE]
```

### 2.6 Chicken coop
```
Three-quarter view of a small wooden chicken coop on short stilts, about 2 metres wide, weathered boards painted faded red, a small ramp to a little door, a nesting box on the side with a hinged lid, a sloped shingle roof, a small fenced run of woven willow beside it, [STYLE]
```

### 2.7 Smokehouse
```
Three-quarter view of a small square smokehouse about 2.5 metres wide, dark soot-stained log walls, a low heavy plank door, a steep shingle roof with a small smoke vent at the top, a stone fire pit at the base on one side, [STYLE]
```

### 2.8 Root cellar entrance
```
Three-quarter view of a root cellar entrance built into a small grassy mound, a stone archway framing a heavy plank door with iron hinges, a short timber roof over the door, stone steps leading down, [STYLE]
```

### 2.9 Workshop
```
Three-quarter view of a small timber workshop shed about 4 metres wide, plank walls, a shingle roof, a wide double door standing open showing a workbench with hand tools hanging on the wall, a grindstone wheel outside the door, [STYLE]
```

### 2.10 Greenhouse
```
Three-quarter view of a small 1900s greenhouse about 4 metres long, a wooden frame with many small glass panes, a low brick base wall, a sloped glass roof, wooden shelves with seedlings in clay pots inside, [STYLE]
```

### 2.11 Stable
```
Three-quarter view of a small log stable for one horse, about 5 metres wide, a split stable door with the top half open, a hay loft opening under the gable, a shingle roof, a hitching rail and a water trough in front, [STYLE]
```

### 2.12 Outhouse
```
Three-quarter view of a small wooden outhouse, weathered vertical boards, a sloped shingle roof, a plank door with a small heart-shaped cutout, [STYLE]
```

---

## Priority 3: Street and household props

### 3.1 Props sheet: work and storage
```
A reference sheet of 1900s rural household props arranged in a grid on a plain light grey background: a wooden barrel, a wooden crate, a burlap sack of potatoes, a wicker basket, a wooden bucket, a wooden sled loaded with firewood, a two-wheeled hand cart, a wooden wheelbarrow, an axe, a saw, a shovel, a hay fork, a sickle, an oil lantern, separated clearly, soft even lighting, realistic, high detail
```

### 3.2 Horse cart
```
Three-quarter view of a simple wooden four-wheeled farm cart, weathered planks, iron-rimmed spoked wooden wheels, two wooden shafts for a horse, empty, [STYLE]
```

### 3.3 Street details sheet
```
A reference sheet of 1900s village street details on a plain light grey background: a wooden bench, a washing line on two posts with linen hanging, a wooden signpost, a stack of sandbags, a stone wall section, a small wooden footbridge, a wooden water trough, a rain barrel, separated clearly, soft even lighting, realistic, high detail
```

---

## Priority 4: Interiors (for the close-up zoom)

### 4.1 The main room
```
Interior reference of the main room of a small 1900s northern log house, seen from a high corner: a large white tiled stove with a cooking surface, a heavy wooden table with benches, a cupboard with painted plates, a spinning wheel, a wooden bed with wool blankets in an alcove, a rag rug on a plank floor, small windows with lace curtains, oil lamps, soft warm light, realistic, high detail
```

### 4.2 Furniture sheet
```
A reference sheet of 1900s rural northern furniture on a plain light grey background: a cast-iron wood stove, a white tiled corner stove, a heavy wooden table, a wooden bench, a ladder-back chair, a wooden bed with a straw mattress and wool blankets, a painted wooden chest, a hanging wall cupboard, a cradle, a spinning wheel, separated clearly, soft even lighting, realistic, high detail
```

### 4.3 Kitchen items sheet
```
A reference sheet of 1900s rural kitchen items on a plain light grey background: cast-iron pots and pans, a kettle, earthenware jars, a bread board with a loaf, wooden spoons and bowls, strings of dried fish and mushrooms, a butter churn, a flour sack, jars of preserves, an oil lamp, separated clearly, soft even lighting, realistic, high detail
```

---

## Priority 5: People (for image-to-3D and Mixamo)

Character sheets need a **front and side view in an A-pose** (arms slightly away from the body), the **whole body visible**, a **neutral expression** and **plain clothing without loose flowing parts.** These are the images the 3D tools work best from.

### The character block (paste wherever a prompt says [CHARACTER])
```
character turnaround reference sheet, front view and side view side by side, standing in a relaxed A-pose with arms slightly away from the body, full body visible from head to feet, neutral expression, realistic proportions, clothing of a poor rural family in a fictional cold northern country around 1900, plain light grey background, soft even lighting, photorealistic, high detail
```

### 5.1 The Linds
```
Anders, a 38-year-old working man, lean and strong, short beard, flat wool cap, a heavy brown wool coat over a collarless shirt and waistcoat, wool trousers, worn leather boots, [CHARACTER]
```
```
Maja, a 35-year-old woman, tired but kind face, hair tied back under a headscarf, a long dark wool skirt, a knitted shawl over a blouse, an apron, sturdy boots, [CHARACTER]
```
```
Elin, a 10-year-old girl, braided hair, a knitted striped hat and scarf, a patched wool coat, a long skirt over wool stockings, small boots, [CHARACTER]
```
```
Grandmother Ingrid, a 71-year-old woman, small and bent, a deeply lined face, a black headscarf, a long dark dress, a thick knitted shawl, a walking stick, [CHARACTER]
```

### 5.2 Seasonal clothing
For each person, after the base sheet, ask: *"Show this exact same person dressed for deep winter: a heavy fur-lined coat, a fur hat or thick wool headscarf, mittens and felt boots, same pose and background."*

### 5.3 The neighbours
Use the same `[CHARACTER]` block for each neighbour family once they're written in step 5 of the prototype.

---

## Priority 6: Animals (image-to-3D, with animation from Fab packs)

```
A reference sheet of farm animals for a 1900s northern village, each shown from the side on a plain light grey background: a sturdy brown work horse, a brown and white cow, a goat, a hen and a rooster, a shaggy farm dog, separated clearly, soft even lighting, realistic, high detail
```

---

## Not needed here

- **Trees, grass, rocks, ground, mud:** Fab photo-scanned assets
- **Textures** (wood, stone, shingles, bark): Poly Haven, free and CC0
- **Snow, rain, smoke, fire:** effects in Unreal

## A note on AI images

These are internal references only. Steam requires disclosing AI-generated content that players see, hear or read. That clearly covers AI images shown in the game or on the store page, and it very likely covers models built by Claude or generated by image-to-3D tools too. Plan to tick Steam's "pre-generated AI content" box and describe how AI was used. Fab and Poly Haven assets are not AI-generated.
