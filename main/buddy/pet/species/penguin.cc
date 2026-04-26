#include "species_data.h"

namespace {
static const char* const PENGUIN_SLEEP_FRAMES[] = {
    "\n   .---.\n  ( -- )\n  (_____)\n   ~~~~~",
    "\n   .---.\n  ( -- )\n  (_____)\n   =====",
    "    o O .\n   .---.\n  ( __ )\n  (_____)\n   =====",
    "\n\n  .-----.\n ( --   )=>\n  `~~~~~`",
    "\n\n  .-----.\n ( zz   )=>\n  `~~~~~`",
    "\n   .---.\n  ( ^^ )\n /(_____)\n   ~~~~~",
};
static const uint8_t PENGUIN_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 4, 3, 4, 3, 4, 3, 3, 1, 5, 1, 1 };

static const char* const PENGUIN_IDLE_FRAMES[] = {
    "   .---.\n  ( o>o )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( o>o )\n/(     )\n  `-----`\n  J    L",
    "   .---.\n  ( o>o )\n (     )\\\n  `-----`\n   J    L",
    "   .---.\n  ( ->- )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  (o> o )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( o >o)\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( o>o )\n /(  v  )\\\n  `-----`\n   J   L",
    "  \\.---./\n  ( o>o )\n/(     )\\\n  `-----`\n   J   L",
    "\n   .---.\n  ( v>v )\n /(_____)\\\n   J   L",
    "  /.---.\\\n ( ^>^ )\n//(     )\\\\\n  `-----`\n   J   L",
};
static const uint8_t PENGUIN_IDLE_SEQ[] = { 0, 0, 1, 2, 1, 2, 0, 3, 0, 4, 0, 5, 0, 6, 6, 0, 3, 0, 1, 2, 1, 2, 0, 7, 7, 0, 0, 8, 8, 0, 9, 9, 0, 0 };

static const char* const PENGUIN_BUSY_FRAMES[] = {
    "   .---.\n  ( v>v )\n /(     )\\\n /`-----`\\\n   J   L",
    "   .---.\n  ( v>v )\n \\(     )/\n \\`-----`/\n   J   L",
    "      ?\n   .---.\n  ( ^>^ )\n /(  .  )\\\n   J   L",
    "    [_]\n   .---.|\n  ( o>o |\n /(     )\\\n   J   L",
    "      *\n   .---.\n  ( O>O )\n /(  ^  )\\\n   J   L",
    "    ~~~\n   .---.\n  ( ->- )\n /(  _  )\\\n   J   L",
};
static const uint8_t PENGUIN_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const PENGUIN_ATTENTION_FRAMES[] = {
    "   .---.\n  ( O>O )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  (O> O )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( O >O)\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( ^>^ )\n /(     )\\\n  `-----`\n   J   L",
    "  /.---.\\\n /( O>O )\\\n//(     )\\\\\n  `-----`\n  J     L",
    "   .---.\n  ( o>o )\n /(  .  )\\\n  `-----`\n   J   L",
};
static const uint8_t PENGUIN_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const PENGUIN_CELEBRATE_FRAMES[] = {
    "\n   .---.\n  ( ^>^ )\n /(_____)\\\n   J   L",
    "  \\.---./\n  ( ^>^ )\n /(     )\\\n  `-----`\n   ^   ^",
    "  \\^---^/\n  ( O>O )\n /(  W  )\\\n  `-----`\n   v   v",
    "   .---.\n  ( <>< )\n/(     )\n  `-----`\n   /   \\",
    "   .---.\n  (>< ><)\n (     )\\\n  `-----`\n   \\   /",
    "    \\_/\n   .---.\n  ( ^>^ )\n/(  W  )\\\n   J   L",
};
static const uint8_t PENGUIN_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const PENGUIN_DIZZY_FRAMES[] = {
    "  .---.\n ( @>@ )\n/(     )\n `-----`\n  J   L",
    "    .---.\n   ( @>@ )\n   (     )\\\n    `-----`\n     J   L",
    "   .---.\n  ( x>@ )\n /(  ~  )\\\n  `-----`\n   J   L",
    "   .---.\n  ( @>x )\n /(  ~  )\\\n  `-----`\n   J   L",
    "\n   .---.\n  ( @>@ )\n (_______)=\n   ~~~~~",
};
static const uint8_t PENGUIN_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const PENGUIN_HEART_FRAMES[] = {
    "   .---.\n  ( ^>^ )\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  (#^>^#)\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  (<3><3)\n /(     )\\\n  `-----`\n   J   L",
    "   .---.\n  ( @>@ )\n/(     )\\\n  `-----`\n    \\ /",
    "   .---.\n  ( ->- )\n /(  ^  )\\\n  `-----`\n   J   L",
};
static const uint8_t PENGUIN_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kPenguinSpecies = {
    "Penguin",
    {
        BUDDY_ANIM_SEQ(PENGUIN_SLEEP_FRAMES, PENGUIN_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(PENGUIN_IDLE_FRAMES, PENGUIN_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(PENGUIN_BUSY_FRAMES, PENGUIN_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(PENGUIN_ATTENTION_FRAMES, PENGUIN_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(PENGUIN_CELEBRATE_FRAMES, PENGUIN_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(PENGUIN_DIZZY_FRAMES, PENGUIN_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(PENGUIN_HEART_FRAMES, PENGUIN_HEART_SEQ, 5),
    },
};
