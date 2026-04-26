#include "species_data.h"

namespace {
static const char* const CAPYBARA_SLEEP_FRAMES[] = {
    "\n\n    .--.\n  _( -- )_\n (___zz___)",
    "\n    .--.\n  _( -- )_\n (___..___)\n  ~~~~~~~~",
    "\n    .--.\n  _( __ )_\n (___oO___)\n  ~~~~~~~~",
    "\n\n  .---___\n (--   --)=\n  `~~~~~~`",
    "\n\n  .---___\n (-- ZZZ-)=\n  `~~~~~~`",
    "\n    .--.\n  _( ^^ )_\n (___O____)\n  ~~~~~~~~",
};
static const uint8_t CAPYBARA_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 4, 3, 4, 3, 4, 3, 3, 1, 5, 1, 1 };

static const char* const CAPYBARA_IDLE_FRAMES[] = {
    "\n  n______n\n ( o    o )\n (   oo   )\n  `------'",
    "\n  n______n\n (o     o )\n (   oo   )\n  `------'",
    "\n  n______n\n ( o     o)\n (   oo   )\n  `------'",
    "\n  n______n\n ( ^    ^ )\n (   oo   )\n  `------'",
    "\n  n______n\n ( -    - )\n (   oo   )\n  `------'",
    "\n  ^______n\n ( o    o )\n (   oo   )\n  `------'",
    "\n  n______n\n ( o    o )\n (   ww   )\n  `------'",
    "\n  n______n\n ( o    o )\n (   WW   )\n  `------'",
    "\n  n______n\n ( -    - )\n (   OO   )\n  `------'",
    "\n /n______n\\\n/( o    o )\\\n (   oo   )\n  `------'",
};
static const uint8_t CAPYBARA_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 0, 0, 6, 7, 6, 7, 0, 0, 3, 3, 0, 4, 8, 8, 0, 0, 9, 9, 0, 0 };

static const char* const CAPYBARA_BUSY_FRAMES[] = {
    "\n  n______n\n ( v    v )\n (   --   )\n /`------'\\",
    "\n  n______n\n ( v    v )\n (   __   )\n \\`------'/",
    "      ?\n  n______n\n ( ^    ^ )\n (   ..   )\n  `------'",
    "    [_]\n  n_____|n\n ( o    o|)\n (   --   )\n  `------'",
    "      *\n  n______n\n ( O    O )\n (   ^^   )\n /`------'\\",
    "    ~~~\n  n______n\n ( -    - )\n (   __   )\n  `------'",
};
static const uint8_t CAPYBARA_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const CAPYBARA_ATTENTION_FRAMES[] = {
    "    ^  ^\n /^_____^\\\n( O      O )\n (   O    )\n  `------'",
    "    ^  ^\n /^_____^\\\n(O       O )\n (   O    )\n  `------'",
    "    ^  ^\n /^_____^\\\n( O       O)\n (   O    )\n  `------'",
    "    ^  ^\n /^_____^\\\n( ^      ^ )\n (   O    )\n  `------'",
    "    ^  ^\n/^^_____^^\\\n( O      O )\n (   O    )\n /`------'\\",
    "    ^  ^\n /^_____^\\\n( o      o )\n (   .    )\n  `------'",
};
static const uint8_t CAPYBARA_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const CAPYBARA_CELEBRATE_FRAMES[] = {
    "\n  n______n\n ( ^    ^ )\n (   ww   )\n /`------'\\",
    "  \\(    )/\n   n____n\n ( ^    ^ )\n (   ww   )\n  `------'",
    "  \\^    ^/\n   n____n\n ( ^    ^ )\n (   WW   )\n  `------'",
    "\n  n______n\n( <    < ) /\n (   ww   )\n  `------'",
    "\n  n______n\n\\( >    > )\n (   ww   )\n  `------'",
    "    \\__/\n  n______n\n ( ^    ^ )\n/(   WW   )\\\n  `------'",
};
static const uint8_t CAPYBARA_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const CAPYBARA_DIZZY_FRAMES[] = {
    "\n n______n\n( @    @ )\n (   ~~   )\n  `------'",
    "\n   n______n\n  ( @    @ )\n (   ~~   )\n  `------'",
    "\n  n______n\n ( x    @ )\n (   ~v   )\n  `------'",
    "\n  n______n\n ( @    x )\n (   v~   )\n  `------'",
    "\n  n______n\n ( @    @ )\n (   --   )\n /`-_---_'\\",
};
static const uint8_t CAPYBARA_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const CAPYBARA_HEART_FRAMES[] = {
    "\n  n______n\n ( ^    ^ )\n (   ww   )\n  `------'",
    "\n  n______n\n (#^    ^#)\n (   ww   )\n  `------'",
    "\n  n______n\n ( <3  <3 )\n (   ww   )\n  `------'",
    "\n  n______n\n ( @    @ )\n (   ww   )\n /`------'\\",
    "\n  n______n\n ( -    - )\n (   ^^   )\n  `------'",
};
static const uint8_t CAPYBARA_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kCapybaraSpecies = {
    "Capybara",
    {
        BUDDY_ANIM_SEQ(CAPYBARA_SLEEP_FRAMES, CAPYBARA_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(CAPYBARA_IDLE_FRAMES, CAPYBARA_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(CAPYBARA_BUSY_FRAMES, CAPYBARA_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(CAPYBARA_ATTENTION_FRAMES, CAPYBARA_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(CAPYBARA_CELEBRATE_FRAMES, CAPYBARA_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(CAPYBARA_DIZZY_FRAMES, CAPYBARA_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(CAPYBARA_HEART_FRAMES, CAPYBARA_HEART_SEQ, 5),
    },
};
