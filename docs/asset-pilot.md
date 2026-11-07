# Asset Pilot: One House

A small test of the whole asset pipeline on a single house, before spending money on assets. It runs alongside the gameplay prototype.

**The question it answers:** can a ChatGPT reference image become a clean 3D house that looks like our concept art, and how much cleanup does that take?

## The pipeline

| Step | Who | What |
|---|---|---|
| 1 | You | Generate reference images in ChatGPT with the prompts below |
| 2 | You | Turn the best image into a 3D model with a free trial of **Tripo** or **Meshy** |
| 3 | You | Save the downloaded model in `assets/pilot/raw/` |
| 4 | Claude | Clean it up in Blender with scripts: scale, orientation, mesh, materials |
| 5 | Claude | Make a snowy-roof variant and render preview images |
| 6 | Together | Compare with the concept art and decide if the pipeline works |

---

## Step 1: Reference images

### Rules that matter for 3D generation
- **One object only.** No people, fences, gardens, trees or ground clutter.
- **Plain light-grey background** so the tools can separate the house.
- **Soft, even lighting** with no strong shadows. Shadows get baked into the textures.
- **The whole house in frame,** nothing cut off.
- **No snow, rain or night.** We add weather later; the base model must be clean.

### The house (used in every prompt)

> a one-and-a-half storey log house from a fictional cold northern country around 1900, about 10 metres wide and 9 metres deep, weathered dark timber log walls, a low grey fieldstone foundation, a steep gable roof covered in weathered wooden shingles, one red-brown brick chimney, a small covered porch with two carved wooden posts at the front door, four small multi-pane windows on the front with carved wooden window frames painted faded ochre, lace curtains inside, deep red painted boards under the gable, a small attic window in the gable, realistic, weathered and lived-in

### Prompt A: three-quarter view (the main one)
Most image-to-3D tools work best from this angle. Generate this first and keep generating until you have one you love.

```
Three-quarter front view of [THE HOUSE], isolated single object centered on a plain light grey background, soft even studio lighting with no harsh shadows, entire building visible with space around it, no people, no fence, no trees, no snow, photorealistic 3D asset reference, high detail, sharp focus
```

### Prompt B: straight front view
After you have your favourite from Prompt A, attach it and ask:

```
Show this exact same house from straight in front, orthographic view, same materials, colours and details, isolated on the same plain light grey background, soft even lighting, entire building visible, no people, no fence, no trees, no snow
```

### Prompt C: side view
```
Show this exact same house from its left side, orthographic view, same materials, colours and details, isolated on the same plain light grey background, soft even lighting, entire building visible, no people, no fence, no trees, no snow
```

### Prompt D: back view
```
Show this exact same house from directly behind, orthographic view, same materials, colours and details, isolated on the same plain light grey background, soft even lighting, entire building visible, no people, no fence, no trees, no snow
```

### Tips
- Do Prompts B–D **in the same ChatGPT conversation** as your favourite Prompt A image, so it keeps the house consistent.
- If the views don't match each other closely, just use the Prompt A image on its own. Single-image generation works fine.
- Ask for widescreen or square, whichever it gives best; it doesn't matter here.
- Save the images in `assets/pilot/reference/`.

---

## Step 2: Image to 3D

Use a free trial of **Tripo** (tripo3d.ai) or **Meshy** (meshy.ai):

1. Upload the Prompt A image. If the tool offers **multi-view**, upload A–D together instead.
2. Settings, where offered:
   - **Textures / PBR materials:** on
   - **Topology:** quad, if available
   - **Polygon count:** medium (around 20,000–50,000 faces)
   - **Style:** realistic
3. Generate. If the first result is poor, try once or twice more.
4. Download as **GLB** (preferred) or **FBX**.
5. Save it in `assets/pilot/raw/`, then tell Claude.

Note which tool and settings you used, and roughly how long it took. That's part of what we're measuring.

---

## Step 3 onwards: Claude's part

Once the model is in `assets/pilot/raw/`, Claude will:
- Import it into Blender and fix the scale (real metres) and orientation
- Check and simplify the mesh; check the textures
- Make a **snowy-roof variant**
- Render preview images from the game's camera angles (high overhead, mid, close)
- Export a version ready for Unreal

Blender has to be installed first. It's free from blender.org. Without admin rights on this Mac, drag it into an `Applications` folder inside your home folder instead of the main one.

## Success test

- From the high and mid camera angles, it reads as **our** house: it matches the concept art
- The mesh is clean enough to use, with an hour or less of cleanup
- The textures look natural, without blurry or melted areas
