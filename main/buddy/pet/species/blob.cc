#include "species_data.h"

namespace {
static const char* const BLOB_SLEEP_FRAMES[] = {
    "\n\n   .----.\n  ( -- -- )\n  `~------~`",
    "\n   .----.\n  ( -- -- )\n  (        )\n   `------`",
    "\n  .------.\n ( -- -- )\n (         )\n  `~------~`",
    "\n\n   .----.\n  ( -- -- )\n  `--.----`",
    "\n\n   .----.\n  ( __ __ )\n `~~~~~~~~~`",
    "\n  .------.\n ( __ __ )\n (    o    )\n  `~------~`",
};
static const uint8_t BLOB_SLEEP_SEQ[] = { 0, 1, 2, 1, 0, 1, 2, 1, 0, 4, 4, 0, 1, 2, 5, 2, 1, 3, 3, 0, 0, 1, 2, 1, 0 };

static const char* const BLOB_IDLE_FRAMES[] = {
    "\n    .--.\n   (o  o)\n   (    )\n    `--`",
    "\n   .----.\n  ( o  o )\n  (      )\n   `----`",
    "\n  .------.\n ( o    o )\n (        )\n  `------`",
    "\n   .----.\n  (o   o )\n  (      )\n   `----`",
    "\n   .----.\n  ( o   o)\n  (      )\n   `----`",
    "\n   .----.\n  ( -  - )\n  (      )\n   `----`",
    "\n  .----.\n ( o  o )\n (      )\n  `----`",
    "\n    .----.\n   ( o  o )\n   (      )\n    `----`",
    "\n  .~~~~~~.\n ( o    o )\n (        )\n  `~~~~~~`",
    "\n   .----.\n  ( o  o )\n  (      )\n  `--.--.`",
};
static const uint8_t BLOB_IDLE_SEQ[] = { 1, 2, 1, 0, 1, 2, 1, 3, 1, 4, 1, 5, 2, 2, 8, 8, 2, 6, 7, 6, 7, 1, 1, 2, 9, 9, 1, 1, 3, 4, 3, 4, 1, 5, 1, 0, 0, 2, 2 };

static const char* const BLOB_BUSY_FRAMES[] = {
    "\n   .----.\n  ( v  v )\n  (   --  )\n   `----`",
    "\n   .----.\n  ( v  v )\n  (   __  )\n   `----`",
    "\n  .~----~.\n ( v    v )\n (   oo   )\n  `~----~`",
    "      ?\n   .----.\n  ( ^  ^ )\n  (   ..  )\n   `----`",
    "      *\n  .------.\n ( O    O )\n (   ==   )\n  `------`",
    "\n   .----.\n  ( v  v )\n  (   --  )\n  `--.----`",
};
static const uint8_t BLOB_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 4, 4, 0, 1, 5, 5, 2 };

static const char* const BLOB_ATTENTION_FRAMES[] = {
    "    .--.\n   (    )\n  ( O  O )\n  (   !   )\n  `------`",
    "    .--.\n   (    )\n ( O  O  )\n (   !    )\n `------`",
    "    .--.\n   (    )\n  ( O  O )\n   (   !  )\n   `------`",
    "     ||\n    /  \\\n  ( O  O )\n  (   !   )\n  `------`",
    "    .--.\n  /(    )\\\n /( O  O )\\\n (   !!   )\n /`------`\\",
    "\n    .--.\n   (O  O)\n   (  !  )\n    `--`",
};
static const uint8_t BLOB_ATTENTION_SEQ[] = { 0, 3, 0, 1, 0, 0, 0, 2, 3, 3, 0, 1, 0, 0, 4, 2, 0, 0, 5, 0 };

static const char* const BLOB_CELEBRATE_FRAMES[] = {
    "\n\n  .--------.\n ( ^      ^)\n `~~------~`",
    "\n   .----.\n  ( ^  ^ )\n /(  ww  )\\\n  `------`",
    "    .--.\n   ( ^^ )\n   (  WW)\n    `--`\n    : :",
    "\n\n .---------.\n( ^      ^ )\n `~~~------`",
    "\n\n.---------.\n( ^      ^ )\n`------~~~`",
    "    \\__/\n   .----.\n  ( *  * )\n /(  WW  )\\\n  `------`",
};
static const uint8_t BLOB_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const BLOB_DIZZY_FRAMES[] = {
    "\n  .----.\n ( @  @ )\n (  ~~  )\n  `----`",
    "\n    .----.\n   ( @  @ )\n   (  ~~  )\n    `----`",
    "\n  .~----~.\n ( x    @ )\n (   vv   )\n  `~----~`",
    "\n  .~----~.\n ( @    x )\n (   vv   )\n  `~----~`",
    "\n\n .---------.\n( @      @ )\n `--._.--._`",
};
static const uint8_t BLOB_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const BLOB_HEART_FRAMES[] = {
    "\n   .----.\n  ( ^  ^ )\n  (   ww  )\n   `----`",
    "\n   .----.\n  (#^  ^#)\n  (   ww  )\n   `----`",
    "\n  .------.\n ( <3  <3 )\n (    v   )\n  `------`",
    "\n  .~~~~~~.\n ( @    @ )\n (   ww   )\n  `~------`",
    "\n   .----.\n  ( -  - )\n  (   ^^  )\n   `----`",
};
static const uint8_t BLOB_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kBlobSpecies = {
    "Blob",
    {
        BUDDY_ANIM_SEQ(BLOB_SLEEP_FRAMES, BLOB_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(BLOB_IDLE_FRAMES, BLOB_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(BLOB_BUSY_FRAMES, BLOB_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(BLOB_ATTENTION_FRAMES, BLOB_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(BLOB_CELEBRATE_FRAMES, BLOB_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(BLOB_DIZZY_FRAMES, BLOB_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(BLOB_HEART_FRAMES, BLOB_HEART_SEQ, 5),
    },
};
