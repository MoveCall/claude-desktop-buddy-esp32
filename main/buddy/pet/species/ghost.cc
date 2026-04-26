#include "species_data.h"

namespace {
static const char* const GHOST_SLEEP_FRAMES[] = {
    "\n   .----.\n  ( -    - )\n  |        |\n  ~`~``~`~",
    "\n   .----.\n  ( -    - )\n  |        |\n  `~`~~`~`",
    "\n   . -- .\n  ( -    - )\n  .        .\n  . . . . .",
    "\n\n   .    .\n\n  . .  . .",
    "\n   .    .\n  ( -    - )\n  .        .\n  . `~`  .",
    "\n   .----.\n  ( z    z )\n  |   o    |\n  ~~`~~`~~",
};
static const uint8_t GHOST_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 5, 5, 0, 1, 0, 1, 2, 2, 3, 3, 4, 4, 0, 1, 0, 1, 5, 5, 2, 3, 4, 0, 1 };

static const char* const GHOST_IDLE_FRAMES[] = {
    "\n   .----.\n  ( o    o )\n  |   __   |\n  ~`~``~`~",
    "\n   .----.\n  (o    o  )\n  |   __   |\n  ~`~``~`~",
    "\n   .----.\n  (  o    o)\n  |   __   |\n  ~`~``~`~",
    "\n   .----.\n  ( ^    ^ )\n  |   __   |\n  ~`~``~`~",
    "\n   .----.\n  ( -    - )\n  |   __   |\n  ~`~``~`~",
    "\n   .----.\n  ( o    o )\n  |   uu   |\n  ~`~``~`~",
    "\n   .----.\n  ( o    o )\n  |   __   |\n  `~`~~`~`",
    "\n   .----.\n  ( o    o )\n  |   __   |\n  ~~`~~`~~",
    "\n   . -- .\n  . o    o .\n  .   __   .\n  . `~``~ .",
    "\n   .----.\n  ( O    O )\n  |   oo   |\n  ~`~``~`~",
};
static const uint8_t GHOST_IDLE_SEQ[] = { 0, 0, 6, 7, 0, 1, 0, 2, 0, 4, 0, 6, 5, 5, 0, 0, 7, 6, 0, 3, 0, 4, 8, 8, 0, 0, 9, 9, 0, 6, 7, 0 };

static const char* const GHOST_BUSY_FRAMES[] = {
    "\n   .----.\n  ( v    v )\n  |   --   |\n /~`~``~`~\\",
    "\n   .----.\n  ( v    v )\n  |   __   |\n \\~`~``~`~/",
    "      ?\n   .----.\n  ( ^    ^ )\n  |   ..   |\n  ~`~``~`~",
    "    * . *\n   .----.\n  ( O    O )\n  |   oo   |\n  ~`~``~`~",
    "      !\n   .----.\n  ( O    O )\n  |   ^^   |\n /~`~``~`~\\",
    "    ~ ~ ~\n   .----.\n  ( -    - )\n  |   __   |\n  `~`~~`~`",
};
static const uint8_t GHOST_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const GHOST_ATTENTION_FRAMES[] = {
    "    ! !!\n   .----.\n  ( O    O )\n  |   OO   |\n /~`~``~`~\\",
    "    ! !!\n   .----.\n  (O      O)\n  |   O    |\n  ~`~``~`~",
    "    ! !!\n   .----.\n  ( O    O )\n  |    O   |\n  ~`~``~`~",
    "    ! !!\n   .----.\n  ( ^    ^ )\n  |   O    |\n  ~`~``~`~",
    "    ! !!\n  /.----.\\\n  ( O    O )\n  |   O    |\n /~`~``~`~\\",
    "    .  .\n   .----.\n  ( o    o )\n  |   .    |\n  ~`~``~`~",
};
static const uint8_t GHOST_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const GHOST_CELEBRATE_FRAMES[] = {
    "\n   .----.\n  ( ^    ^ )\n  |   ww   |\n /~`~``~`~\\",
    "  \\(    )/\n   .----.\n  ( ^    ^ )\n  |   ww   |\n  ~`~``~`~",
    "  \\^    ^/\n   .----.\n  ( ^    ^ )\n  |   WW   |\n  ~`~``~`~",
    "\n   .----.\n  ( <    < )\n  |   ww   |\n  ~`~``~`~/",
    "\n   .----.\n  ( >    > )\n  |   ww   |\n \\~`~``~`~",
    "    \\__/\n   .----.\n  ( ^    ^ )\n  |   WW   |\n/~`~``~`~\\",
};
static const uint8_t GHOST_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const GHOST_DIZZY_FRAMES[] = {
    "\n  .----.\n ( @    @ )\n |   ~~   |\n  ~`~``~`~",
    "\n    .----.\n  ( @    @ )\n   |   ~~   |\n  ~`~``~`~",
    "\n   .----.\n  ( x    @ )\n  |   ~v   |\n  ~`~``~`~",
    "\n   .----.\n  ( @    x )\n  |   v~   |\n  ~`~``~`~",
    "\n   .----.\n  ( @    @ )\n  |   --   |\n /~_~``_~\\",
};
static const uint8_t GHOST_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const GHOST_HEART_FRAMES[] = {
    "\n   .----.\n  ( ^    ^ )\n  |   ww   |\n  ~`~``~`~",
    "\n   .----.\n  (#^    ^#)\n  |   ww   |\n  ~`~``~`~",
    "\n   .----.\n  ( <3  <3 )\n  |   ww   |\n  ~`~``~`~",
    "\n   .----.\n  ( @    @ )\n  |   ww   |\n /~`~``~`~\\",
    "\n   .----.\n  ( -    - )\n  |   ^^   |\n  `~`~~`~`",
};
static const uint8_t GHOST_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kGhostSpecies = {
    "Ghost",
    {
        BUDDY_ANIM_SEQ(GHOST_SLEEP_FRAMES, GHOST_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(GHOST_IDLE_FRAMES, GHOST_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(GHOST_BUSY_FRAMES, GHOST_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(GHOST_ATTENTION_FRAMES, GHOST_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(GHOST_CELEBRATE_FRAMES, GHOST_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(GHOST_DIZZY_FRAMES, GHOST_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(GHOST_HEART_FRAMES, GHOST_HEART_SEQ, 5),
    },
};
