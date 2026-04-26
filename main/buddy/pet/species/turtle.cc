#include "species_data.h"

namespace {
static const char* const TURTLE_SLEEP_FRAMES[] = {
    "\n   _,--._\n  /      \\\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  (-_    -)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  ( -    -)\n /[__zz__]\\\n  ``    ``",
    "\n   _,--._\n  ( -    -)\n /[__ZZ__]\\\n  ``    ``",
    "\n\n   _,--._\n  ( ____  )\n /[______]\\",
    "\n   _,--._\n  ( o    o)\n /[__~~__]\\\n  ``    ``",
};
static const uint8_t TURTLE_SLEEP_SEQ[] = { 0, 0, 0, 0, 1, 0, 2, 3, 2, 3, 2, 3, 2, 3, 0, 0, 4, 4, 4, 2, 3, 2, 3, 1, 5, 1, 0 };

static const char* const TURTLE_IDLE_FRAMES[] = {
    "\n   _,--._\n  ( o    o)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  (o     o)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  ( o     o)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  ( -    -)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n <( o    o)\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  ( o    o)>\n /[______]\\\n  ``    ``",
    "\n   _,--._\n  ( o    o)\n /[______]\\\n  `'    `'",
    "\n   _,--._\n  ( o    o)\n /[______]\\\n  ,'    ``",
    "\n   _,--._\n  ( o    o)\n /[______]\\\n  ``    ',",
    "\n   _,--._\n _( o    o)_\n/-[______]-\\\n  ``    ``",
};
static const uint8_t TURTLE_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 4, 4, 0, 5, 5, 0, 0, 3, 0, 0, 7, 8, 7, 8, 7, 8, 0, 0, 6, 6, 0, 0, 3, 0, 0, 9, 9, 9, 0, 0 };

static const char* const TURTLE_BUSY_FRAMES[] = {
    "\n   _,--._\n  ( v    v)\n /[______]\\\n  >`    >`",
    "\n   _,--._\n  ( v    v)\n /[______]\\\n  `>    `>",
    "      ?\n   _,--._\n  ( ^    ^)\n /[__..__]\\\n  ``    ``",
    "    [#]\n   _,-#-._\n  ( o    o)\n /[______]\\\n  ``    ``",
    "      *\n   _,--._\n  ( O    O)\n /[__^^__]\\\n  >`    >`",
    "    ~~~\n   _,--._\n  ( -    -)\n /[______]\\\n  ``    ``",
};
static const uint8_t TURTLE_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const TURTLE_ATTENTION_FRAMES[] = {
    "\n   _,--._\n  ( O    O)\n /[__||__]\\\n  ``    ``",
    "\n   _,--._\n <(O     O)\n /[__||__]\\\n  ``    ``",
    "\n   _,--._\n  ( O     O)>\n /[__||__]\\\n  ``    ``",
    "    ^  ^\n   _,--._\n  ( ^    ^)\n /[__||__]\\\n  ``    ``",
    "\n  /^_,--._^\\\n  ( O    O)\n /[__||__]\\\n /``    ``\\",
    "\n   _,--._\n  ( o    o)\n /[__><__]\\\n  ``    ``",
};
static const uint8_t TURTLE_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const TURTLE_CELEBRATE_FRAMES[] = {
    "\n   _,--._\n  ( ^    ^)\n /[__ww__]\\\n /``    ``\\",
    "  \\(    )/\n   _,--._\n  ( ^    ^)\n /[__ww__]\\\n  ``    ``",
    "  \\^    ^/\n   _,--._\n  ( ^    ^)\n /[__WW__]\\\n  ``    ``",
    "\n   .--._\n  ( ^    ^)\n/[==_v_==]\\\n  ``    ``",
    "\n    _,--.\n  ( ^    ^)\n /[==_v_==]\\\n  ``    ``",
    "    \\__/\n   _,--._\n  ( ^    ^)\n/[__WW__]\\\n /``    ``\\",
};
static const uint8_t TURTLE_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const TURTLE_DIZZY_FRAMES[] = {
    "\n  _,--._\n ( @    @)\n/[__~~__]\\\n ``    ``",
    "\n    _,--._\n  ( @    @)\n  /[__~~__]\\\n   ``    ``",
    "\n   _,--._\n  ( x    @)\n /[__~v__]\\\n  ``    ``",
    "\n   _,--._\n  ( @    x)\n /[__v~__]\\\n  ``    ``",
    "\n  ``    ``\n \\[__~~__]/\n  ( @    @)\n   `--''",
};
static const uint8_t TURTLE_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const TURTLE_HEART_FRAMES[] = {
    "\n   _,--._\n  ( ^    ^)\n /[__ww__]\\\n  ``    ``",
    "\n   _,--._\n  (#^    ^#)\n /[__ww__]\\\n  ``    ``",
    "\n   _,--._\n  (<3    <3)\n /[__ww__]\\\n  ``    ``",
    "\n   _,--._\n  ( @    @)\n /[__ww__]\\\n /``    ``\\",
    "\n   _,--._\n  ( -    -)\n /[__^^__]\\\n  ``    ``",
};
static const uint8_t TURTLE_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kTurtleSpecies = {
    "Turtle",
    {
        BUDDY_ANIM_SEQ(TURTLE_SLEEP_FRAMES, TURTLE_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(TURTLE_IDLE_FRAMES, TURTLE_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(TURTLE_BUSY_FRAMES, TURTLE_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(TURTLE_ATTENTION_FRAMES, TURTLE_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(TURTLE_CELEBRATE_FRAMES, TURTLE_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(TURTLE_DIZZY_FRAMES, TURTLE_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(TURTLE_HEART_FRAMES, TURTLE_HEART_SEQ, 5),
    },
};
