#include "species_data.h"

namespace {
static const char* const ROBOT_SLEEP_FRAMES[] = {
    "\n   .[__].\n  [ -    - ]\n  [ ____ ]\n  `------'",
    "\n   .[..].\n  [ .    . ]\n  [ ____ ]\n  `------'",
    "\n   .[  ].\n  [        ]\n  [ ____ ]\n  `------'",
    "\n   .[||].\n  [ -    - ]\n  [ z__z ]\n  `------'",
    "    .[*].\n   .[||].\n  [ -    - ]\n  [ zzzz ]\n  `------'",
    "\n   .[..].\n  [ o    - ]\n  [ ____ ]\n  `------'",
};
static const uint8_t ROBOT_SLEEP_SEQ[] = { 0, 1, 2, 1, 0, 1, 2, 1, 0, 0, 3, 3, 4, 4, 4, 3, 0, 1, 2, 1, 0, 5, 0, 1, 0 };

static const char* const ROBOT_IDLE_FRAMES[] = {
    "\n   .[||].\n  [ o    o ]\n  [ ==== ]\n  `------'",
    "\n   .[||].\n  [o     o ]\n  [ ==== ]\n  `------'",
    "\n   .[||].\n  [ o     o]\n  [ ==== ]\n  `------'",
    "\n   .[||].\n  [ -    - ]\n  [ ==== ]\n  `------'",
    "\n   .[\\\\].\n  [ o    o ]\n  [ ==== ]\n  `------'",
    "\n   .[//].\n  [ o    o ]\n  [ ==== ]\n  `------'",
    "\n   .[||].\n  [ o    o ]\n  [ -==- ]\n  `------'",
    "\n   .[||].\n  [ o    o ]\n  [ =--= ]\n  `------'",
    "    .[*].\n   .[||].\n  [ ^    ^ ]\n  [ ==== ]\n  `------'",
    "\n   .[||].\n  [ o    o ]\n  [ ==== ]\n /`------'\\",
};
static const uint8_t ROBOT_IDLE_SEQ[] = { 0, 0, 1, 1, 0, 2, 2, 0, 3, 0, 0, 4, 5, 4, 5, 0, 6, 7, 6, 7, 0, 0, 8, 8, 0, 9, 9, 0, 3, 0 };

static const char* const ROBOT_BUSY_FRAMES[] = {
    "    01010\n   .[||].\n  [ #    # ]\n  [ ==== ]\n /`------'\\",
    "    10101\n   .[||].\n  [ #    # ]\n  [ -==- ]\n \\`------'/",
    "     ?\n   .[||].\n  [ ^    ^ ]\n  [ .... ]\n  `------'",
    "    [@@]\n   .[||].\n  [ o    o ]\n  [ ==== ]\n  `------'",
    "     !\n   .[||].\n  [ O    O ]\n  [ ^^^^ ]\n /`------'\\",
    "    ~~~\n   .[||].\n  [ -    - ]\n  [ ____ ]\n  `------'",
};
static const uint8_t ROBOT_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const ROBOT_ATTENTION_FRAMES[] = {
    "    [!]\n   .[||].\n  [ O    O ]\n  [ #### ]\n /`------'\\",
    "    [!]\n   .[\\\\].\n  [O     O ]\n  [ #### ]\n /`------'\\",
    "    [!]\n   .[//].\n  [ O     O]\n  [ #### ]\n /`------'\\",
    "    [!]\n   .[||].\n  [ ^    ^ ]\n  [ #### ]\n /`------'\\",
    "    {!!}\n   .[||].\n  [ X    X ]\n  [ #### ]\n//`------'\\\\",
    "    [.]\n   .[||].\n  [ o    o ]\n  [ .... ]\n  `------'",
};
static const uint8_t ROBOT_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const ROBOT_CELEBRATE_FRAMES[] = {
    "\n   .[||].\n  [ ^    ^ ]\n  [ ==== ]\n /`------'\\",
    "  \\[||]/\n   .----.\n  [ ^    ^ ]\n  [ ==== ]\n  `------'",
    "  \\[**]/\n   .----.\n  [ O    O ]\n  [ ^^^^ ]\n  `------'",
    "\n   .[\\\\].\n  [ <    < ]\n  [ ==== ] /\n  `------'",
    "\n   .[//].\n  [ >    > ]\n \\[ ==== ]\n  `------'",
    "    [**]\n   .[||].\n  [ ^    ^ ]\n /[ #### ]\\\n  `------'",
};
static const uint8_t ROBOT_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const ROBOT_DIZZY_FRAMES[] = {
    "\n  .[||].\n [ x    x ]\n [ ~~~~ ]\n  `------'",
    "\n    .[||].\n  [ x    x ]\n   [ ~~~~ ]\n  `------'",
    "\n   .[/\\].\n  [ X    @ ]\n  [ #v#v ]\n  `--__--'",
    "\n   .[\\/].\n  [ @    X ]\n  [ v#v# ]\n  `--__--'",
    "\n   .[??].\n  [ x    x ]\n  [ ____ ]\n /`-_--_-'\\",
};
static const uint8_t ROBOT_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const ROBOT_HEART_FRAMES[] = {
    "    [<3]\n   .[||].\n  [ ^    ^ ]\n  [ ==== ]\n  `------'",
    "    [<3]\n   .[||].\n  [#^    ^#]\n  [ ==== ]\n  `------'",
    "    [<3]\n   .[||].\n  [ <3  <3 ]\n  [ ==== ]\n  `------'",
    "    [<3]\n   .[||].\n  [ @    @ ]\n  [ ==== ]\n /`------'\\",
    "    [<3]\n   .[||].\n  [ -    - ]\n  [ ^^^^ ]\n  `------'",
};
static const uint8_t ROBOT_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kRobotSpecies = {
    "Robot",
    {
        BUDDY_ANIM_SEQ(ROBOT_SLEEP_FRAMES, ROBOT_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(ROBOT_IDLE_FRAMES, ROBOT_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(ROBOT_BUSY_FRAMES, ROBOT_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(ROBOT_ATTENTION_FRAMES, ROBOT_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(ROBOT_CELEBRATE_FRAMES, ROBOT_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(ROBOT_DIZZY_FRAMES, ROBOT_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(ROBOT_HEART_FRAMES, ROBOT_HEART_SEQ, 5),
    },
};
