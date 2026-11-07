#include "tls/social.h"

namespace tls {

namespace {
struct TraitInfo {
    Trait trait;
    const char* id;
    const char* name;
};
constexpr TraitInfo kTraits[kTraitCount] = {
    {Trait::Hardworking, "hardworking", "Hardworking"},
    {Trait::Lazy, "lazy", "Lazy"},
    {Trait::Generous, "generous", "Generous"},
    {Trait::Stingy, "stingy", "Stingy"},
    {Trait::Kind, "kind", "Kind to children"},
    {Trait::Proud, "proud", "Proud"},
    {Trait::Sociable, "sociable", "Sociable"},
    {Trait::Private, "private", "Private"},
    {Trait::Quarrelsome, "quarrelsome", "Quarrelsome"},
    {Trait::GrudgeHolder, "grudge_holder", "Holds grudges"},
    {Trait::Forgiving, "forgiving", "Forgiving"},
};
}  // namespace

const char* TraitName(Trait trait) {
    for (const TraitInfo& t : kTraits) {
        if (t.trait == trait) return t.name;
    }
    return "?";
}

bool TraitFromName(const std::string& name, Trait& out) {
    for (const TraitInfo& t : kTraits) {
        if (name == t.id) {
            out = t.trait;
            return true;
        }
    }
    return false;
}

const char* MemoryText(MemoryKind kind) {
    switch (kind) {
        case MemoryKind::HelpedUs: return "helped us when we were short of";
        case MemoryKind::WeHelped: return "we helped them with";
        case MemoryKind::RefusedUs: return "refused to share their";
        case MemoryKind::Quarrel: return "quarrelled with us";
    }
    return "?";
}

Feeling FeelingFor(float opinion) {
    if (opinion <= -50.0f) return Feeling::Enemies;
    if (opinion <= -15.0f) return Feeling::Cold;
    if (opinion < 15.0f) return Feeling::Neutral;
    if (opinion < 50.0f) return Feeling::Friendly;
    return Feeling::Close;
}

const char* FeelingName(Feeling feeling) {
    switch (feeling) {
        case Feeling::Enemies: return "Enemies";
        case Feeling::Cold: return "Cold";
        case Feeling::Neutral: return "Neutral";
        case Feeling::Friendly: return "Friendly";
        case Feeling::Close: return "Close";
    }
    return "?";
}

}  // namespace tls
