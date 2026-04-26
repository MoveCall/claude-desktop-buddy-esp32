#include "species_data.h"

namespace {
static const char* const OWL_SLEEP_FRAMES[] = {
    "\n   .-..-.\n  ( -  - )\n  (  __  )\n   `----'",
    "    .--.\n   /-..-\\\n  ( -  - )\n  (  __  )\n   `----'",
    "    .--.\n   /-..-\\\n  ( _  _ )\n  (  ZZ  )\n   `----'",
    "\n   .-..-.\n ( -  -  )\n  (  __  )\n   `----'",
    "\n   .-..-.\n  (  - - -)\n  (  __  )\n   `----'",
    "      hoo\n   .-..-.\n  ( -  - )\n  (  oo  )\n   `----'",
};
static const uint8_t OWL_SLEEP_SEQ[] = { 0, 1, 0, 1, 2, 2, 1, 0, 1, 0, 3, 3, 0, 4, 4, 0, 1, 2, 2, 1, 0, 5, 0, 0 };

static const char* const OWL_IDLE_FRAMES[] = {
    "\n   /\\  /\\\n  ((O)(O))\n  (  ><  )\n   `----'",
    "\n   /\\  /\\\n  ((-)(-))\n  (  ><  )\n   `----'",
    "\n   /\\  /\\\n  ((O)(O))\n (  ><   )\n   `----'",
    "\n   /\\  /\\\n  ((O)(O))\n  (   >< )\n   `----'",
    "\n  /\\  /\\\n ((O)(O))\n  (  ><  )\n   `----'",
    "\n    /\\  /\\\n   ((O)(O))\n  (  ><  )\n   `----'",
    "\n   /\\  /\\\n  ((O)(-))\n  (  ><  )\n   `----'",
    "\n   /\\  /\\\n  ((o)(o))\n  (  v<  )\n   `----'",
    "\n  /^\\  /^\\\n (((O)(O)))\n (  ><  )\n  `------'",
    "      hoo\n   /\\  /\\\n  ((O)(O))\n  (  oo  )\n   `----'",
};
static const uint8_t OWL_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 0, 2, 2, 0, 3, 3, 0, 1, 0, 4, 4, 0, 5, 5, 0, 6, 0, 7, 7, 0, 8, 8, 0, 9, 9, 0, 0 };

static const char* const OWL_BUSY_FRAMES[] = {
    "    [___]\n   /\\  /\\\n  ((v)(v))\n  (  --  )\n   `----'",
    "    [___]\n   /\\  /\\\n  ((v)(v))\n  (  >>  )\n   `----'",
    "    [___]\n   /\\  /\\\n  ((v)(v))\n  (  <<  )\n   `----'",
    "      ?\n   /\\  /\\\n  ((^)(^))\n  (  ..  )\n   `----'",
    "      *\n   /\\  /\\\n  ((O)(O))\n  (  ^^  )\n  /`----'\\",
    "   [____]\n  /\\   /\\\n ((v) (v))\n  (  --  )\n   `----'",
};
static const uint8_t OWL_BUSY_SEQ[] = { 0, 1, 0, 2, 0, 1, 0, 2, 3, 3, 0, 1, 0, 2, 5, 5, 0, 1, 4, 4, 0, 2, 0, 1, 3 };

static const char* const OWL_ATTENTION_FRAMES[] = {
    "\n  /^\\  /^\\\n ((O))((O))\n (   ><   )\n  `------'",
    "\n  /^\\  /^\\\n((O))((O))\n (   ><   )\n  `------'",
    "\n  /^\\  /^\\\n  ((O))((O))\n (   ><   )\n  `------'",
    "\n //^\\\\//^\\\\\n ((O))((O))\n (   ><   )\n  `------'",
    "\n /^^\\/^^\\\n((O))((O))\n((   ><   ))\n /`------'\\",
    "\n  /^\\  /^\\\n ((-))((-))\n (   ><   )\n  `------'",
};
static const uint8_t OWL_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const OWL_CELEBRATE_FRAMES[] = {
    "\n   /\\  /\\\n  ((^)(^))\n  (  ww  )\n  /`----'\\",
    "  </    \\>\n   /\\  /\\\n  ((^)(^))\n  (  ww  )\n   `----'",
    "\n \\_/\\  /\\_/\n  ((^)(^))\n  (  WW  )\n   `----'",
    "\n </\\  /\\\n((<)(<))\n  (  ww  )\n   `----'",
    "\n    /\\  /\\>\n    ((>)(>))\n  (  ww  )\n   `----'",
    "    \\__/\n  </\\  /\\>\n ((^)(^))\n /(  WW  )\\\n   `----'",
};
static const uint8_t OWL_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const OWL_DIZZY_FRAMES[] = {
    "\n  /\\  /\\\n ((@)(@))\n  (  ~~  )\n   `----'",
    "\n    /\\  /\\\n   ((@)(@))\n  (  ~~  )\n   `----'",
    "\n   /\\  /\\\n  ((x)(@))\n  (  ~v  )\n   `----'",
    "\n   /\\  /\\\n  ((@)(x))\n  (  v~  )\n   `----'",
    "\n   \\/  \\/\n  ((@)(@))\n  (  --  )\n  /`-_--'\\",
};
static const uint8_t OWL_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const OWL_HEART_FRAMES[] = {
    "\n   /\\  /\\\n  ((^)(^))\n  (  ww  )\n   `----'",
    "\n   /\\  /\\\n #((^)(^))#\n  (  ww  )\n   `----'",
    "\n   /\\  /\\\n  ((<3)(<3))\n  (  ww  )\n   `----'",
    "\n   /\\  /\\\n  ((@)(@))\n  (  ww  )\n  /`----'\\",
    "      hoo\n   /\\  /\\\n  ((-)(-))\n  (  vv  )\n   `----'",
};
static const uint8_t OWL_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kOwlSpecies = {
    "Owl",
    {
        BUDDY_ANIM_SEQ(OWL_SLEEP_FRAMES, OWL_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(OWL_IDLE_FRAMES, OWL_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(OWL_BUSY_FRAMES, OWL_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(OWL_ATTENTION_FRAMES, OWL_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(OWL_CELEBRATE_FRAMES, OWL_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(OWL_DIZZY_FRAMES, OWL_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(OWL_HEART_FRAMES, OWL_HEART_SEQ, 5),
    },
};
