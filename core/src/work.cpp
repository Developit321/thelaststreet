// Work, the garden and food storage. Part of Simulation.

#include <algorithm>

#include "tls/simulation.h"

namespace tls {

Activity Simulation::GardenActivity(const Household& household) const {
    const Garden& g = household.garden;
    switch (CurrentDate().season) {
        case Season::Spring:
            return g.planted < 1.0f ? Activity::Planting : Activity::Tending;
        case Season::Summer:
            if (g.growth >= 1.0f && household.CropInGround(work_) > 0.0f) return Activity::Harvesting;
            if (g.planted < 1.0f) return Activity::Planting;  // late sowing: may not ripen in time
            return Activity::Tending;
        case Season::Autumn:
            if (g.growth >= 1.0f && household.CropInGround(work_) > 0.0f) return Activity::Harvesting;
            if (g.planted > 0.0f && g.growth < 1.0f) return Activity::Tending;
            return Activity::Indoors;  // nothing left to do in the garden
        case Season::Winter:
            return Activity::Indoors;
    }
    return Activity::Indoors;
}

float Simulation::WorkRate(const Person& p, bool outside) const {
    float rate = work_.workerRate;
    switch (p.role) {
        case Role::Worker: rate = work_.workerRate; break;
        case Role::Homemaker: rate = work_.homemakerRate; break;
        case Role::Child: rate = work_.childRate; break;
        case Role::Elder: rate = work_.elderRate; break;
    }
    if (p.sick) rate *= work_.sickRate;
    if (p.health < 50.0f) rate *= p.health / 50.0f;
    if (outside) {
        const float t = TemperatureC();
        if (t < work_.tooColdC) rate *= 0.25f;  // only under orders: barely anything gets done
        else if (t < work_.bitterColdC) rate *= 0.5f;
    }
    return rate;
}

void Simulation::DoWork(int index) {
    Household& h = households_[index];
    Stores& s = h.stores;
    Garden& g = h.garden;

    for (const Person& p : h.members) {
        if (!p.alive) continue;
        const float perMinute = WorkRate(p, IsOutside(p.place)) / 60.0f;

        switch (p.activity) {
            case Activity::ChoppingWood:
                s.firewood += work_.firewoodPerHour * perMinute;
                break;

            case Activity::Planting: {
                const bool wasDone = g.planted >= 1.0f;
                g.planted = std::min(1.0f, g.planted + perMinute / work_.plantingHours);
                if (!wasDone && g.planted >= 1.0f && !g.plantingAnnounced) {
                    g.plantingAnnounced = true;
                    Emit(EventType::PlantingDone, index);
                }
                break;
            }

            case Activity::Tending:
                g.care = std::min(1.0f, g.care + work_.carePerHour * perMinute);
                break;

            case Activity::Harvesting: {
                const float amount = std::min(h.CropInGround(work_), work_.harvestPerHour * perMinute);
                s.freshFood += amount;
                g.harvested += amount;
                if (h.CropInGround(work_) <= 0.0f && !g.harvestAnnounced) {
                    g.harvestAnnounced = true;
                    Emit(EventType::HarvestDone, index, -1, g.harvested);
                }
                break;
            }

            case Activity::Preserving: {
                // Drying and smoking takes a little firewood and loses some weight.
                const float room = (work_.preservedCapacity - s.preservedFood) / (1.0f - work_.preserveLoss);
                float amount = std::min({s.freshFood, work_.preservePerHour * perMinute, std::max(0.0f, room)});
                if (work_.preserveFirewoodPerRation > 0.0f) {
                    amount = std::min(amount, s.firewood / work_.preserveFirewoodPerRation);
                }
                if (amount <= 0.0f) break;
                s.freshFood -= amount;
                s.preservedFood += amount * (1.0f - work_.preserveLoss);
                const float wood = amount * work_.preserveFirewoodPerRation;
                s.firewood -= wood;
                h.firewoodUsedToday += wood;
                break;
            }

            default:
                break;
        }
    }
}

void Simulation::GardenAndStorageDaily(int index, int64_t dayEnded) {
    Household& h = households_[index];
    Stores& s = h.stores;
    Garden& g = h.garden;
    const float mean = weather_.DailyMeanC(dayEnded);

    // Fresh food spoils, faster when it's warm and when there's no room for it.
    float spoilRate = work_.freshSpoilCold;
    if (mean > 12.0f) spoilRate = work_.freshSpoilWarm;
    else if (mean > 2.0f) spoilRate = work_.freshSpoilMild;
    float spoiled = s.freshFood * spoilRate;
    const float overflow = std::max(0.0f, s.freshFood - work_.freshCapacity);
    spoiled += overflow * work_.overflowSpoilPerDay;
    spoiled = std::min(spoiled, s.freshFood);
    s.freshFood -= spoiled;
    s.preservedFood -= s.preservedFood * work_.preservedSpoil;
    if (spoiled >= 1.0f) Emit(EventType::FoodSpoiled, index, -1, spoiled);

    // The garden grows on warm days once something is sown, and weeds creep back.
    if (g.planted > 0.0f && g.growth < 1.0f) {
        if (mean > work_.growMinTempC) g.growth = std::min(1.0f, g.growth + 1.0f / work_.growDays);
        g.care = std::max(0.0f, g.care - work_.careDecayPerDay);
        if (g.growth >= 1.0f && !g.readyAnnounced) {
            g.readyAnnounced = true;
            Emit(EventType::CropsReady, index, -1, h.CropInGround(work_));
        }
    }

    // Frost ruins whatever is still in the ground.
    const float inGround = h.CropInGround(work_);
    if (g.growth > 0.0f && inGround > 0.0f && mean < -1.0f) {
        const float lost = inGround * work_.frostLossPerDay;
        g.lostToFrost += lost;
        if (lost >= 1.0f) Emit(EventType::CropsLostToFrost, index, -1, lost);
    }
}

}  // namespace tls
