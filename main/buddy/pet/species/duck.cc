#include "species_data.h"

namespace {
static const char* const DUCK_SLEEP_FRAMES[] = {
    "\n\n    __\n  <(-_)_)\n ~~~~~~~~~~",
    "\n    __\n  <(-_)_)\n  ~~~~~~~~\n   ~~~~~~",
    "\n    __\n  <(o.)_)\n  ~~~~~~~~\n   ~~~~~~",
    "\n   __\n <(-_)_)\n ~~~~~~~~~~\n  ~~~~~~~~",
    "\n     __\n   <(-_)_)\n ~~~~~~~~~~\n  ~~~~~~~~",
    "\n    __\n  <(uu)_)\n ~~~~~~~~~~\n  ~~~~~~~~",
};
static const uint8_t DUCK_SLEEP_SEQ[] = { 0, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 3, 4, 4, 3, 4, 0, 0, 1, 5, 1, 1 };

static const char* const DUCK_IDLE_FRAMES[] = {
    "\n    __\n  <(o )___\n   (  ._>\n    `--'",
    "\n    __\n <<(o )___\n   (  ._>\n    `--'",
    "\n    __\n  <( o)___\n   (  ._>\n    `--'",
    "    __\n  <(^ )\n  (    )___\n   (  ._>\n    `--'",
    "\n    __\n  <(- )___\n   (  ._>\n    `--'",
    "\n    __\n  <O(o)___\n   (  ._>\n    `--'",
    "\n    __\n  <(o )___\n   ( v.->\n    `--'",
    "\n    __\n  <(o )___\n   ( ^.->\n    `--'",
    "\n    __\n  <(o )___\n   (  ._<\n    `--'",
    "\n    __\n  <(o )___\n  ~(  ._>~\n   ~`--'~",
};
static const uint8_t DUCK_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 0, 0, 6, 7, 6, 7, 0, 0, 3, 3, 0, 4, 8, 0, 8, 0, 9, 9, 0, 0 };

static const char* const DUCK_BUSY_FRAMES[] = {
    "\n    __\n  <(o )___\n   (  ._>\n  ~ `--'",
    "\n    __\n  <(o )___\n   (  ._>\n    `--' ~",
    "\n\n    __\n  <(v )_O_\n   ( ._>~~~",
    "\n\n      _o_\n    ^>>\n  ~~~~~~~~",
    "\n    __\n  <(O )___\n  *(  ._>*\n  ~~~~~~~~",
    "      ?\n    __\n  <(o )___\n   (  ._>\n    `--'",
};
static const uint8_t DUCK_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 5, 5, 0, 1, 0, 1, 2, 3, 3, 2, 4, 4, 0, 1, 0, 1, 5 };

static const char* const DUCK_ATTENTION_FRAMES[] = {
    "    __\n  <(O )\n  (    )___\n   (  ._>\n    `--'",
    "    __\n <<(O )\n  (    )___\n   (  ._>\n    `--'",
    "    __\n  <( O)\n  (    )___\n   (  ._>\n    `--'",
    "  <(O )\n    ||\n    ||\n   (  ._>\n    `--'",
    "    __\n /<(O )\\\n /(    )___\n  /(  ._>\\\n   /`--'\\",
    "    __\n  <O(O )\n  (    )___\n   (  ._>\n    `--'",
};
static const uint8_t DUCK_ATTENTION_SEQ[] = { 0, 5, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const DUCK_CELEBRATE_FRAMES[] = {
    "\n    __\n  <(^ )___\n   (  ._>\n /`--'\\",
    "  \\(    )/\n    __\n  <(^ )___\n   (  ._>\n    `--'",
    "  \\^ __ ^/\n   <(^ )___\n   (  ._>\n    `--'\n  ~~~~~~~~",
    "\n    __\n  <(^ )___\n ~~( ._> )~\n  ~~`--'~~",
    "\n    __\n  <(^ )___\n  ~~( ._>~~\n   ~`--'~",
    "    \\__/\n    __\n  <(^ )___\n /(  ._>\\\n    `--'",
};
static const uint8_t DUCK_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const DUCK_DIZZY_FRAMES[] = {
    "\n   __\n <(@ )___\n  (  .~>\n   `--'",
    "\n     __\n   <(@ )___\n    (  .~>\n     `--'",
    "\n    __\n  <(x@)___\n   ( ~~>\n    `--'",
    "\n    __\n  <(@x)___\n   ( ~~>\n    `--'",
    "\n    __\n  <(@ )___\n   (  ~~>\n /`-_---_'\\",
};
static const uint8_t DUCK_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const DUCK_HEART_FRAMES[] = {
    "\n    __\n  <(^ )___\n   (  ._>\n    `--'",
    "\n    __\n  <(^#)___\n   (  ._>\n    `--'",
    "\n    __\n  <(<3)___\n   (  ._>\n    `--'",
    "\n    __\n  <(@ )___\n   (  ._>\n /`--'\\",
    "\n    __\n  <(- )___\n   (  ^_>\n    `--'",
};
static const uint8_t DUCK_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kDuckSpecies = {
    "Duck",
    {
        BUDDY_ANIM_SEQ(DUCK_SLEEP_FRAMES, DUCK_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(DUCK_IDLE_FRAMES, DUCK_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(DUCK_BUSY_FRAMES, DUCK_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(DUCK_ATTENTION_FRAMES, DUCK_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(DUCK_CELEBRATE_FRAMES, DUCK_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(DUCK_DIZZY_FRAMES, DUCK_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(DUCK_HEART_FRAMES, DUCK_HEART_SEQ, 5),
    },
};
