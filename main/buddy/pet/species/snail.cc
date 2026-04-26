#include "species_data.h"

namespace {
static const char* const SNAIL_SLEEP_FRAMES[] = {
    "\n     .--.\n    ( zz )\n   `-____-`\n  ~~~~~~~~",
    "\n     .--.\n    ( -- )\n   `-____-`\n  ~~~~~~~~",
    "      o\n     .--.\n    ( __ )\n   `-____-`\n  ~~~~~~~~",
    "      O\n     .--.\n    ( __ )\n   `-zzzz-`\n  ~~~~~~~~",
    "    . o O\n     .--.\n    ( UU )\n   `-____-`\n  ~~~~~~~~",
    "\n     .--.\n    ( ^^ )\n   `-____-`\n ~~~~~~~~~~",
};
static const uint8_t SNAIL_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 3, 3, 3, 4, 4, 4, 4, 1, 5, 1, 1 };

static const char* const SNAIL_IDLE_FRAMES[] = {
    "  \\\\  /\n    .--.\n  _( oo )_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _(<<  )_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _(  >>)_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _( ^^ )_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _( -- )_\n (___@@___)\n  ~~~~~~~~",
    "  /  //\n    .--.\n  _( oo )_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  \\\\\n    .--.\n  _( oo )_\n (___@@___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _( oo )_\n (___ww___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _( oo )_\n (___WW___)\n  ~~~~~~~~",
    "   \\  /\n    .--.\n  _( oo )__\n (___@@____)\n ~~~~~~~~~~",
};
static const uint8_t SNAIL_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 6, 5, 6, 0, 0, 0, 3, 3, 0, 4, 7, 8, 7, 8, 0, 9, 9, 9, 0, 0 };

static const char* const SNAIL_BUSY_FRAMES[] = {
    "  \\\\  /\n    .o-.\n  _( vv )_\n (___--___)\n  ~~~~~~~~",
    "  \\\\  /\n    .-o.\n  _( vv )_\n (___--___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _( vv )o\n (___--___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  o( vv )_\n (___--___)\n  ~~~~~~~~",
    " ? \\\\  /\n    .--.\n  _( ^^ )_\n (___..___)\n  ~~~~~~~~",
    " * \\\\  / *\n    .--.\n  _( OO )_\n (___^^___)\n  ~~~~~~~~",
};
static const uint8_t SNAIL_BUSY_SEQ[] = { 0, 1, 2, 3, 0, 1, 2, 3, 4, 4, 0, 1, 2, 3, 4, 4, 5, 0, 1, 2, 3 };

static const char* const SNAIL_ATTENTION_FRAMES[] = {
    "  ||  ||\n    .--.\n  _( OO )_\n (___OO___)\n  ~~~~~~~~",
    "  \\\\  \\\\\n    .--.\n  _(OO  )_\n (___OO___)\n  ~~~~~~~~",
    "  //  //\n    .--.\n  _(  OO)_\n (___OO___)\n  ~~~~~~~~",
    "  ||  ||\n    .--.\n  _( ^^ )_\n (___OO___)\n  ~~~~~~~~",
    "  ||  ||\n   /.--.\\\n  _( OO )_\n/(___OO___)\\\n /~~~~~~~~\\",
    "  \\|  |/\n    .--.\n  _( oo )_\n (___..___)\n  ~~~~~~~~",
};
static const uint8_t SNAIL_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const SNAIL_CELEBRATE_FRAMES[] = {
    "  \\\\  /\n    .--.\n  _( ^^ )_\n (___ww___)\n /~~~~~~~~\\",
    "  \\^  ^/\n   _.--._\n  ( ^  ^ )\n (___ww___)\n  ~~~~~~~~",
    "  \\(  )/\n   _.--._\n  ( **  ) )\n (___WW___)\n  ~~~~~~~~",
    "  /\\\n    .--.\n  _( <<)_\n (___ww___)\n  ~~~~~~~~",
    "        /\\\n    .--.\n   _(>> )_\n (___ww___)\n  ~~~~~~~~",
    "  \\\\  // *\n  * .--.\n  _( ^^ )_\n/(___WW___)\\\n  ~~~~~~~~",
};
static const uint8_t SNAIL_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const SNAIL_DIZZY_FRAMES[] = {
    "  \\\\\\\n   .--.\n  ( @x )\n (___~v___)\n  ~~~~~~~~",
    "       ///\n     .--.\n    ( x@ )\n (___v~___)\n  ~~~~~~~~",
    "  \\~  ~/\n    .--.\n  _( @x )_\n (___~v___)\n  ~~~~~~~~",
    "  ~\\  /~\n    .--.\n  _( x@ )_\n (___v~___)\n  ~~~~~~~~",
    "  \\v  v/\n    .--.\n  _( @@ )_\n (___--___)\n /~_~~_~~\\",
};
static const uint8_t SNAIL_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const SNAIL_HEART_FRAMES[] = {
    "  \\\\  /\n    .--.\n  _( ^^ )_\n (___ww___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _(#^^#)_\n (___ww___)\n  ~~~~~~~~",
    "  \\\\  /\n    .--.\n  _(<3<3)_\n (___ww___)\n  ~~~~~~~~",
    "  \\\\//\n    .--.\n  _( @@ )_\n (___ww___)\n /~~~~~~~~\\",
    "  \\\\  /\n    .--.\n  _( -- )_\n (___^^___)\n  ~~~~~~~~",
};
static const uint8_t SNAIL_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kSnailSpecies = {
    "Snail",
    {
        BUDDY_ANIM_SEQ(SNAIL_SLEEP_FRAMES, SNAIL_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(SNAIL_IDLE_FRAMES, SNAIL_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(SNAIL_BUSY_FRAMES, SNAIL_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(SNAIL_ATTENTION_FRAMES, SNAIL_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(SNAIL_CELEBRATE_FRAMES, SNAIL_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(SNAIL_DIZZY_FRAMES, SNAIL_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(SNAIL_HEART_FRAMES, SNAIL_HEART_SEQ, 5),
    },
};
