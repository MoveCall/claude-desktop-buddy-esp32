#include "species_data.h"

extern const SpeciesData kCatSpecies;
extern const SpeciesData kDuckSpecies;
extern const SpeciesData kPenguinSpecies;
extern const SpeciesData kGhostSpecies;
extern const SpeciesData kRobotSpecies;
extern const SpeciesData kBlobSpecies;
extern const SpeciesData kOctopusSpecies;
extern const SpeciesData kCapybaraSpecies;
extern const SpeciesData kDragonSpecies;
extern const SpeciesData kGooseSpecies;
extern const SpeciesData kOwlSpecies;
extern const SpeciesData kRabbitSpecies;
extern const SpeciesData kTurtleSpecies;
extern const SpeciesData kSnailSpecies;
extern const SpeciesData kMushroomSpecies;
extern const SpeciesData kCactusSpecies;
extern const SpeciesData kChonkSpecies;
extern const SpeciesData kAxolotlSpecies;

namespace {
const SpeciesData* const kSpeciesRegistry[] = {
    &kCatSpecies,
    &kDuckSpecies,
    &kPenguinSpecies,
    &kGhostSpecies,
    &kRobotSpecies,
    &kBlobSpecies,
    &kOctopusSpecies,
    &kCapybaraSpecies,
    &kDragonSpecies,
    &kGooseSpecies,
    &kOwlSpecies,
    &kRabbitSpecies,
    &kTurtleSpecies,
    &kSnailSpecies,
    &kMushroomSpecies,
    &kCactusSpecies,
    &kChonkSpecies,
    &kAxolotlSpecies,
};
} // namespace

const SpeciesData* const* buddy_species_registry() {
    return kSpeciesRegistry;
}

uint8_t buddy_species_registry_count() {
    return static_cast<uint8_t>(BUDDY_ARRAY_LEN(kSpeciesRegistry));
}
