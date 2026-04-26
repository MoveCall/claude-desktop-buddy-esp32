#include "species_data.h"

namespace {
static const char* const CHONK_SLEEP_FRAMES[] = {
    "\n\n  /\\____/\\\n ( -    - )\n (___zz___)",
    "\n  /\\____/\\\n ( -    - )\n (   --   )\n (________)",
    "\n  /\\____/\\\n ( -    - )\n(    --    )\n(__________)",
    "\n  /\\____/\\\n ( -    - )\n(    OO    )\n(__________)",
    "\n  /\\____/\\\n ( -    - )\n(   ~~~~   )\n ~~~~~~~~~~",
};
static const uint8_t CHONK_SLEEP_SEQ[] = { 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 3, 3, 1, 2, 1, 2, 1, 2, 4, 1, 4, 1, 0, 0, 1, 2, 3, 2 };

static const char* const CHONK_IDLE_FRAMES[] = {
    "\n  /\\____/\\\n ( o    o )\n (   ..   )\n  `------'",
    "\n  /\\____/\\\n (o     o )\n (   ..   )\n  `------'",
    "\n  /\\____/\\\n ( o     o)\n (   ..   )\n  `------'",
    "\n  /\\____/\\\n ( '    ' )\n (   ..   )\n  `------'",
    "\n  /\\____/\\\n ( -    - )\n (   ..   )\n  `------'",
    "\n  /|____/\\\n ( o    o )\n (   ..   )\n  `------'",
    "\n  /\\____|\\\n ( o    o )\n (   ..   )\n  `------'",
    "\n  /\\____/\\\n ( o    o )\n(    ..    )\n (________)",
    "\n  /\\____/\\\n ( o    o )\n (   ..   )\n(__________)",
    "\n  /\\____/\\\n ( o    o )\n (   oo   )\n  `------'",
};
static const uint8_t CHONK_IDLE_SEQ[] = { 0, 0, 0, 4, 0, 1, 1, 0, 2, 2, 0, 4, 5, 0, 6, 0, 0, 3, 3, 0, 7, 8, 7, 8, 0, 0, 4, 0, 9, 9, 0, 0, 0, 5, 6, 0 };

static const char* const CHONK_BUSY_FRAMES[] = {
    "      ?\n  /\\____/\\\n ( o    o )\n (   --   )\n  `------'",
    "\n  /\\____/\\\n ( v    v )\n (   ::   )\n /`------'\\",
    "\n  /\\____/\\\n ( v    v )\n (   ;;   )\n \\`------'/",
    "    ___\n  /\\___/\\/\n ( o    o--\n (   --   )\n  `------'",
    "      *\n  /\\____/\\\n ( O    O )\n (   ^^   )\n /`------'\\",
    "    ~~~\n  /\\____/\\\n ( -    - )\n (   __   )\n  `------'",
};
static const uint8_t CHONK_BUSY_SEQ[] = { 1, 2, 1, 2, 1, 2, 0, 0, 1, 2, 1, 2, 3, 3, 3, 4, 4, 1, 2, 1, 2, 5, 5 };

static const char* const CHONK_ATTENTION_FRAMES[] = {
    "    ^  ^\n  /^____^\\\n ( O    O )\n (   o    )\n  `------'",
    "    ^  ^\n  /^____^\\\n (O     O )\n (   o    )\n  `------'",
    "    ^  ^\n  /^____^\\\n ( O     O)\n (   o    )\n  `------'",
    "    /\\/\\\n  /^____^\\\n ( ^    ^ )\n (   o    )\n  `------'",
    "    ^  ^\n /^^____^^\\\n ( O    O )\n (   o    )\n /`------'\\",
    "    ^  ^\n  /^____^\\\n ( o    o )\n (   O    )\n  `------'",
};
static const uint8_t CHONK_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const CHONK_CELEBRATE_FRAMES[] = {
    "\n  /\\____/\\\n ( ^    ^ )\n(    WW    )\n(__________)",
    "  \\(    )/\n   /____\\\n ( ^    ^ )\n (   WW   )\n  `------'",
    "  \\^    ^/\n   /____\\\n ( ^    ^ )\n (   OO   )\n  `------'",
    "\n  /\\____/\\\n( <    < ) /\n (   ww   )\n  `------'",
    "\n  /\\____/\\\n\\( >    > )\n (   ww   )\n  `------'",
    "    \\__/\n  /\\____/\\\n ( ^    ^ )\n/(   WW   )\\\n  `------'",
};
static const uint8_t CHONK_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const CHONK_DIZZY_FRAMES[] = {
    "\n /\\____/\\\n( @    @ )\n (   ~~   )\n  `------'",
    "\n   /\\____/\\\n  ( @    @ )\n (   ~~   )\n  `------'",
    "\n  /\\____/\\\n ( x    @ )\n (   ~v   )\n  `------'",
    "\n  /\\____/\\\n ( @    x )\n (   v~   )\n  `------'",
    "\n  /\\____/\\\n ( @    @ )\n(    --    )\n /`-_---_'\\",
};
static const uint8_t CHONK_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const CHONK_HEART_FRAMES[] = {
    "\n  /\\____/\\\n ( ^    ^ )\n (   ww   )\n  `------'",
    "\n  /\\____/\\\n (#^    ^#)\n (   ww   )\n  `------'",
    "\n  /\\____/\\\n ( <3  <3 )\n (   ww   )\n  `------'",
    "\n  /\\____/\\\n ( @    @ )\n(    ww    )\n /`------'\\",
    "\n  /\\____/\\\n ( -    - )\n (   ^^   )\n  `------'",
    "  v      v\n  /\\____/\\\n ( ^    ^ )\n/(   ww   )\\\n  `------'",
};
static const uint8_t CHONK_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 5, 5, 0, 1, 0 };

} // namespace

extern const SpeciesData kChonkSpecies = {
    "Chonk",
    {
        BUDDY_ANIM_SEQ(CHONK_SLEEP_FRAMES, CHONK_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(CHONK_IDLE_FRAMES, CHONK_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(CHONK_BUSY_FRAMES, CHONK_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(CHONK_ATTENTION_FRAMES, CHONK_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(CHONK_CELEBRATE_FRAMES, CHONK_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(CHONK_DIZZY_FRAMES, CHONK_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(CHONK_HEART_FRAMES, CHONK_HEART_SEQ, 5),
    },
};
