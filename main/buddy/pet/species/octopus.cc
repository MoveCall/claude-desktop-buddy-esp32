#include "species_data.h"

namespace {
static const char* const OCTOPUS_SLEEP_FRAMES[] = {
    "\n\n   .----.\n  ( -- -- )\n  (~~zz~~)",
    "\n   .----.\n  ( -- -- )\n  (______)\n  ~~~~~~~~",
    "\n   .----.\n  ( __ __ )\n  (______)\n  ~~~~~~~~",
    "\n  .----.\n ( -- -- )\n (______)\n ~/~/~/~/",
    "\n    .----.\n   ( -- -- )\n   (______)\n   /~/~/~/~",
    "\n   .----.\n  ( oO oO )\n  (__o___)\n  ~~~~~~~~",
};
static const uint8_t OCTOPUS_SLEEP_SEQ[] = { 0, 0, 1, 1, 0, 0, 2, 2, 1, 1, 0, 0, 3, 3, 4, 4, 3, 4, 1, 1, 2, 2, 0, 5, 1, 1 };

static const char* const OCTOPUS_IDLE_FRAMES[] = {
    "\n   .----.\n  ( o  o )\n  (______)\n  /\\/\\/\\/\\",
    "\n   .----.\n  ( o  o )\n  (______)\n  \\/\\/\\/\\/",
    "\n   .----.\n  (o   o )\n  (______)\n  /\\/\\/\\/\\",
    "\n   .----.\n  ( o   o)\n  (______)\n  \\/\\/\\/\\/",
    "\n   .----.\n  ( -  - )\n  (______)\n  /\\/\\/\\/\\",
    "\n   .----.\n  ( o  o )\n  (______)\n  /)/\\/\\(\\",
    "\n   .----.\n  ( ^  ^ )\n  (______)\n  )(/\\/\\)(",
    "   .----.\n  ( o  o )\n  (______)\n  /\\/\\/\\/\\\n",
    "\n   .----.\n  ( ^  ^ )\n  (\\__/\\)\n  /\\/\\/\\/\\",
    "\n  /.----.\\\n /( o  o )\\\n \\(______)/\n //\\/\\/\\/\\\\",
};
static const uint8_t OCTOPUS_IDLE_SEQ[] = { 0, 1, 0, 1, 0, 2, 1, 3, 0, 1, 4, 0, 5, 6, 5, 6, 0, 1, 0, 4, 1, 0, 7, 7, 0, 1, 8, 8, 0, 1, 9, 9, 0, 1 };

static const char* const OCTOPUS_BUSY_FRAMES[] = {
    "\n   .----.\n  ( v  v )\n  (__--__)\n  /)\\/\\/(\\",
    "\n   .----.\n  ( v  v )\n  (__==__)\n  (\\/\\/\\/)",
    "      ?\n   .----.\n  ( ^  ^ )\n  (__..__)\n  /\\/\\/\\/\\",
    "    [_]\n   .---|.\n  ( o  o|)\n  (__--__)\n  /\\(\\/\\/\\",
    "      *\n   .----.\n  ( O  O )\n  (__^^__)\n //\\/\\/\\\\\\",
    "    ~~~\n   .----.\n  ( -  - )\n  (__--__)\n  /\\/\\/\\/\\",
};
static const uint8_t OCTOPUS_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const OCTOPUS_ATTENTION_FRAMES[] = {
    "    ^  ^\n   .----.\n  ( O  O )\n  (__O___)\n  /\\/\\/\\/\\",
    "    ^  ^\n   .----.\n  (O   O )\n  (__O___)\n  /\\/\\/\\/\\",
    "    ^  ^\n   .----.\n  ( O   O)\n  (__O___)\n  \\/\\/\\/\\/",
    "    ^  ^\n   .----.\n  ( ^  ^ )\n  (__O___)\n  /\\/\\/\\/\\",
    "    ^  ^\n  /.----.\\\n ( O    O )\n  (__O___)\n //\\/\\/\\\\\\",
    "    ^  ^\n   .----.\n  ( o  o )\n  (__.___)\n  /\\/\\/\\/\\",
};
static const uint8_t OCTOPUS_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const OCTOPUS_CELEBRATE_FRAMES[] = {
    "\n   .----.\n  ( ^  ^ )\n  (__ww__)\n //\\/\\/\\\\\\",
    "  \\/    \\/\n   .----.\n  ( ^  ^ )\n  (__ww__)\n  )(\\/\\/)(",
    "  \\^    ^/\n   .----.\n  ( ^  ^ )\n  (__WW__)\n  ((    ))",
    "\n   .----.\n (( <  < ))\n  (__ww__)\n /)/\\/\\(\\",
    "\n   .----.\n (( >  > ))\n  (__ww__)\n (\\/\\/\\/)/",
    "    \\__/\n   .----.\n /( ^  ^ )\\\n \\(__WW__)/\n //\\/\\/\\\\\\",
};
static const uint8_t OCTOPUS_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const OCTOPUS_DIZZY_FRAMES[] = {
    "\n  .----.\n ( @  @ )\n (__~~__)\n /\\/\\/\\/\\",
    "\n    .----.\n   ( @  @ )\n   (__~~__)\n   \\/\\/\\/\\/",
    "\n   .----.\n  ( x  @ )\n  (__~v__)\n  /\\)/\\(\\",
    "\n   .----.\n  ( @  x )\n  (__v~__)\n  (\\/\\(/\\/",
    "\n   .----.\n  ( @  @ )\n  (__--__)\n /)\\_/\\_(\\",
};
static const uint8_t OCTOPUS_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const OCTOPUS_HEART_FRAMES[] = {
    "\n   .----.\n  ( ^  ^ )\n  (__ww__)\n  /\\/\\/\\/\\",
    "\n   .----.\n  (#^  ^#)\n  (__ww__)\n  /\\/\\/\\/\\",
    "\n   .----.\n  (<3  <3)\n  (__ww__)\n  \\/\\/\\/\\/",
    "\n   .----.\n  ( @  @ )\n  (__ww__)\n //\\/\\/\\\\\\",
    "    ~~~\n   .----.\n  ( -  - )\n  (__^^__)\n  /\\/\\/\\/\\",
    "\n  /.----.\\\n /(#^  ^#)\\\n \\(__ww__)/\n  )(\\/\\/)(",
};
static const uint8_t OCTOPUS_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 5, 5, 1, 0 };

} // namespace

extern const SpeciesData kOctopusSpecies = {
    "Octopus",
    {
        BUDDY_ANIM_SEQ(OCTOPUS_SLEEP_FRAMES, OCTOPUS_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(OCTOPUS_IDLE_FRAMES, OCTOPUS_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(OCTOPUS_BUSY_FRAMES, OCTOPUS_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(OCTOPUS_ATTENTION_FRAMES, OCTOPUS_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(OCTOPUS_CELEBRATE_FRAMES, OCTOPUS_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(OCTOPUS_DIZZY_FRAMES, OCTOPUS_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(OCTOPUS_HEART_FRAMES, OCTOPUS_HEART_SEQ, 5),
    },
};
