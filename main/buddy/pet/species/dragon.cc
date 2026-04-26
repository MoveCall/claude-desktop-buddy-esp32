#include "species_data.h"

namespace {
static const char* const DRAGON_SLEEP_FRAMES[] = {
    "\n\n   _____\n  (--   )~\n  `vvvvv'",
    "\n\n   _____\n  (--   )~~\n  `vvvvv'",
    "\n       o\n   _____\n  (--   )~~\n  `vvvvv'",
    "\n  /v\\  /v\\\n <  --  -- >\n (        )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  oo  oo >\n (   __   )\n  `-vvvv-'",
    "\n\n   _____\n  (--   )$\n  `vvvvv'$$",
};
static const uint8_t DRAGON_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 4, 3, 4, 1, 2, 1, 5, 5, 0, 0, 1, 2, 1, 2 };

static const char* const DRAGON_IDLE_FRAMES[] = {
    "\n  /^\\  /^\\\n <  o    o >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <o     o  >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  o     o>\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  -    - >\n (   ww   )\n  `-vvvv-'",
    "  /^\\  /^\\\n  \\_/  \\_/\n <  o    o >\n (   ww   )\n  `-vvvv-'",
    "\n  \\v/  \\v/\n <  o    o >\n (   ww   )\n  `-vvvv-'",
    "      ~\n  /^\\  /^\\\n <  o    o >\n (   nn   )\n  `-vvvv-'",
    "         ~\n  /^\\  /^\\\n <  o    o >\n (   ww   )~\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  ^    ^ >\n (   --   )\n  `-vvvv-'",
    "  /^\\  /^\\\n //^\\  /^\\\\\n< <  o   o> >\n  (   ww  )\n   `-vvvv-'",
};
static const uint8_t DRAGON_IDLE_SEQ[] = { 0, 0, 1, 0, 2, 0, 3, 4, 5, 4, 5, 4, 5, 0, 0, 6, 7, 0, 3, 0, 8, 8, 0, 1, 2, 0, 9, 9, 0, 0, 7, 0, 8, 0 };

static const char* const DRAGON_BUSY_FRAMES[] = {
    "    $$$$\n  /^\\  /^\\\n <  v    v >\n (   --   )\n /`-vvvv-'\\",
    "    $$$$\n  /^\\  /^\\\n <  v    v >\n (   __   )\n \\`-vvvv-'/",
    "      ?\n  /^\\  /^\\\n <  ^    ^ >\n (   ..   )\n  `-vvvv-'",
    "    [$]\n  /^|  /^\\\n <  v|   v >\n (   --   )\n  `-vvvv-'",
    "      *\n  /^\\  /^\\\n <  O    O >\n (   ^^   )~\n  `-vvvv-'",
    "    ~~~~\n  /^\\  /^\\\n <  -    - >\n (   __   )\n  `-vvvv-'",
};
static const uint8_t DRAGON_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const DRAGON_ATTENTION_FRAMES[] = {
    "    ^  ^\n /^^\\  /^^\\\n<  O    O  >\n (   <>   )\n  `-vvvv-'",
    "    ^  ^\n /^^\\  /^^\\\n< O      O >\n (   O    )\n  `-vvvv-'",
    "    ^  ^\n /^^\\  /^^\\\n<  O      O>\n (    O   )\n  `-vvvv-'",
    "  ~~~  ~~~\n /^^\\  /^^\\\n<  O    O  >\n (   <>   )~\n  `-vvvv-'",
    "    ^  ^\n/^^^\\  /^^^\\\n<  O    O  >\n((  <>   ))~\n /`-vvvv-'\\",
    "    ^  ^\n /^^\\  /^^\\\n<  o    o  >\n (   ss   )\n  `-vvvv-'",
};
static const uint8_t DRAGON_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 3, 5, 0 };

static const char* const DRAGON_CELEBRATE_FRAMES[] = {
    "\n  /^\\  /^\\\n <  ^    ^ >\n (   WW   )\n /`-vvvv-'\\",
    "  \\(    )/\n   /^\\/^\\\n <  ^    ^ >\n (   WW   )\n  `-vvvv-'",
    "  \\^    ^/\n  //^\\/^\\\\\n <  *    * >\n (   OO   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n< <    <  >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  >    > >\n (   ww   )\n  `-vvvv-'",
    "    \\$$/\n  /^\\  /^\\\n <  ^    ^ >\n/(   WW   )\\\n  `-vvvv-'",
};
static const uint8_t DRAGON_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const DRAGON_DIZZY_FRAMES[] = {
    "\n /^\\  /^\\\n< @    @ )\n (   ~~   )\n  `-vvvv-'",
    "\n   /^\\  /^\\\n  ( @    @ >\n (   ~~   )\n  `-vvvv-'",
    "\n  /v\\  /^\\\n <  x    @ >\n (   ~v   )\n  `-vvvv-'",
    "\n  /^\\  /v\\\n <  @    x >\n (   v~   )\n  `-vvvv-'",
    "\n  /v\\  /v\\\n <  @    @ >\n (   --   )\n /`_-vv-_'\\",
};
static const uint8_t DRAGON_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const DRAGON_HEART_FRAMES[] = {
    "\n  /^\\  /^\\\n <  ^    ^ >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <#^    ^# >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n < <3  <3  >\n (   ww   )\n  `-vvvv-'",
    "\n  /^\\  /^\\\n <  @    @ >\n (   ww   )\n /`-vvvv-'\\",
    "      v\n  /^\\  /^\\\n <  -    - >\n (   ^^   )~\n  `-vvvv-'",
};
static const uint8_t DRAGON_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kDragonSpecies = {
    "Dragon",
    {
        BUDDY_ANIM_SEQ(DRAGON_SLEEP_FRAMES, DRAGON_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(DRAGON_IDLE_FRAMES, DRAGON_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(DRAGON_BUSY_FRAMES, DRAGON_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(DRAGON_ATTENTION_FRAMES, DRAGON_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(DRAGON_CELEBRATE_FRAMES, DRAGON_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(DRAGON_DIZZY_FRAMES, DRAGON_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(DRAGON_HEART_FRAMES, DRAGON_HEART_SEQ, 5),
    },
};
