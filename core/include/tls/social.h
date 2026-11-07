#pragma once

#include <cstdint>
#include <string>

namespace tls {

// Personality. Each adult has two or three; they change how a household
// works, helps, visits and holds grudges.
enum class Trait {
    Hardworking,   // works faster
    Lazy,          // works slower
    Generous,      // quick to help
    Stingy,        // slow to help
    Kind,          // helps families with children
    Proud,         // won't ask for help until desperate
    Sociable,      // visits often
    Private,       // keeps to themselves
    Quarrelsome,   // starts arguments
    GrudgeHolder,  // remembers wrongs for a long time
    Forgiving,     // lets things go quickly
};
constexpr int kTraitCount = 11;

const char* TraitName(Trait trait);
bool TraitFromName(const std::string& name, Trait& out);

// Something one household remembers about another.
enum class MemoryKind {
    HelpedUs,     // gave us food or firewood when we needed it
    WeHelped,     // we gave them something
    RefusedUs,    // had plenty and wouldn't share
    Quarrel,
};

const char* MemoryText(MemoryKind kind);  // "Gave us firewood when we needed it" style, without the item

struct Memory {
    MemoryKind kind = MemoryKind::HelpedUs;
    int about = -1;        // the other household
    float impact = 0.0f;   // effect on our opinion of them when fresh
    int64_t minute = 0;
    int item = 0;          // 0 food, 1 firewood, where it matters
};

// How one household feels about another, from -100 to 100.
enum class Feeling { Enemies, Cold, Neutral, Friendly, Close };
Feeling FeelingFor(float opinion);
const char* FeelingName(Feeling feeling);

// Balance numbers for neighbours, from data/social.json.
struct SocialConfig {
    float memoryFadeDays = 30.0f;       // how long a memory takes to fade to its lasting part
    float lastingShare = 0.3f;          // share of a big memory that never fades
    float bigMemory = 20.0f;            // impacts at least this size leave a lasting part
    float grudgeFadeFactor = 2.0f;      // grudge-holders remember wrongs this much longer
    float forgivingFadeFactor = 0.5f;

    float helpedImpact = 25.0f;
    float weHelpedImpact = 5.0f;
    float refusedImpact = -15.0f;
    float quarrelImpact = -12.0f;
    float visitGain = 1.0f;             // per visit, to both sides
    float visitCap = 20.0f;             // most a household's opinion can rise from visits alone

    float askWhenDaysLeft = 3.0f;       // ask for help below this many days of food or wood
    float proudAskWhenDaysLeft = 1.0f;
    float spareFoodDays = 15.0f;        // only share what's beyond this many days
    float spareFirewoodDays = 20.0f;
    float giveDays = 3.0f;              // how many days of need a gift covers
    float helpThreshold = 10.0f;        // opinion needed to help
    float generousBonus = 20.0f;
    float stingyPenalty = 20.0f;
    float kindBonus = 15.0f;            // towards families with children

    float visitChance = 0.25f;          // per household per evening
    float sociableBonus = 0.25f;
    float privatePenalty = 0.15f;
    float quarrelChance = 0.01f;        // per pair per day
    float quarrelsomeBonus = 0.02f;
    float dislikeBonus = 0.02f;         // when they already dislike each other

    float hardworkingRate = 1.2f;
    float lazyRate = 0.8f;
};

}  // namespace tls
