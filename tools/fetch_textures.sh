#!/bin/sh
# Downloads the Poly Haven (CC0) textures the asset generators use into
# assets/textures/. The images aren't kept in git; run this after cloning.
#
#   sh tools/fetch_textures.sh

set -e
cd "$(dirname "$0")/.."

TEXTURES="weathered_brown_planks roof_09 rustic_stone_wall red_brick_03 distressed_painted_planks old_planks_02"
AGENT="TheLastStreet-asset-pipeline"

for id in $TEXTURES; do
    mkdir -p "assets/textures/$id"
    for map in diff nor_gl rough; do
        out="assets/textures/$id/${id}_${map}_2k.jpg"
        [ -f "$out" ] && continue
        echo "Downloading $out"
        curl -fsSL -A "$AGENT" "https://dl.polyhaven.org/file/ph-assets/Textures/jpg/2k/$id/${id}_${map}_2k.jpg" -o "$out"
    done
done
echo "Textures are in assets/textures/"
