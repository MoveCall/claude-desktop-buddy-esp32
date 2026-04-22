#include "buddy_pet.h"
#include "../core/persona.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static uint8_t s_state = 0;
static uint32_t s_tick = 0;
static uint8_t s_species = 0;

// === CAT ===
static const char* CAT_SLEEP[] = {
    " .-..-.\n( -.- )\n`------`",
    " .-..-.\n( -.- )_\n`~------'",
};
static const char* CAT_IDLE[] = {
    " /\\_/\\\n( o o )\n(  w  )\n(\")_(\")",
    " /\\_/\\\n( - - )\n(  w  )\n(\")_(\")",
};
static const char* CAT_BUSY[] = {
    " /\\_/\\\n( o o )\n(  w  )/\n(\")_(\")",
    " /\\_/\\\n( o o )\n(  w  )_\n(\")_(\")",
};
static const char* CAT_ATTENTION[] = {
    " /^_^\\\n( O O )\n(  v  )\n(\")_(\")",
    " /^_^\\\n(O   O)\n(  !  )\n(\")_(\")",
};
static const char* CAT_CELEBRATE[] = {
    "\\^ ^/\n /\\_/\\\n( ^ ^ )\n(  W  )",
    " \\o/\n /\\_/\\\n( * * )\n(  W  )~",
};
static const char* CAT_DIZZY[] = {
    " /\\_/\\\n( @ @ )\n( ~~  )\n(\")_(\")",
    " /\\_/\\\n( x @ )\n(  v  )\n~(\")_(\")",
};
static const char* CAT_HEART[] = {
    " /\\_/\\\n( ^ ^ )\n(  u  )\n(\")_(\")",
    " /\\_/\\\n(#^ ^#)\n(  u  )\n(\")_(\")",
};

// === DUCK ===
static const char* DUCK_SLEEP_F[] = {
    " (__)\n (-.-)\n~(___)~",          // 0: tuck
    " (__)\n (-.-)\n~~(___)~",         // 1: breathe
    " (__)\n (o.)\n~(___)~",           // 2: snore
    " (__)\n (uu)\n~~(___)~",          // 3: dream
};
static const uint8_t DUCK_SLEEP_SEQ[] = { 0,0,1,0,1,2,1, 0,1,0,1, 3,3,0,0 };

static const char* DUCK_IDLE_F[] = {
    " (__)\n (o o)\n>(___)>",          // 0: rest
    " (__)\n (o  )\n>(___)>",          // 1: look L
    " (__)\n (  o)\n>(___)>",          // 2: look R
    " (__)\n (- -)\n>(___)>",          // 3: blink
    " (__)\nO(o o)\n>(___)>",          // 4: quack
};
static const uint8_t DUCK_IDLE_SEQ[] = { 0,0,0,3,0,1,0,2,0, 4,0,0, 0,3,0,1,2,0 };

static const char* DUCK_BUSY_F[] = {
    " (__)\n (o o)\n>(___)> !",
    " (__)\n (o o)\n<(___)< .",
    " (__)\n (o o)\n>(___)> .",
};
static const uint8_t DUCK_BUSY_SEQ[] = { 0,1,0,1, 2,0, 1,0,1,2 };

static const char* DUCK_ATT_F[] = {
    "  !\n (__)\n (O O)\n>(___)>",
    " !!\n (__)\n (O O)\n<(___)<",
    " (__)\n (O O)!\n>(___)>",
};
static const uint8_t DUCK_ATT_SEQ[] = { 0,2,0,1,0,2, 1,0,1,0 };

static const char* DUCK_CEL_F[] = {
    "~\n (__)\n (^ ^)\n>(___)>*",
    "   ~\n (__)\n (* *)\n*<(___)<",
    " (__)\n (^ ^)~\n>(___)>",
};
static const uint8_t DUCK_CEL_SEQ[] = { 0,1,0,1, 2,0, 1,2 };

static const char* DUCK_DIZ_F[] = {
    " (__)\n (@ @)\n~(___)~",
    " (__)\n (x @)\n~(___)~",
    " (__)\n (@ x)\n~(___)~",
};
static const uint8_t DUCK_DIZ_SEQ[] = { 0,1,0,2, 1,0,2,0 };

static const char* DUCK_HRT_F[] = {
    "  v\n (__)\n (^ ^)\n>(___)>",
    " v\n (__)\n (u u)\n>(___)>",
    " (__)\n (^ ^)v\n>(___)>",
};
static const uint8_t DUCK_HRT_SEQ[] = { 0,0,1,0, 2,2,0, 1,0 };

// === PENGUIN ===
static const char* PENG_SLEEP[] = {
    "(o_ _o)\n/|   |\\\n |   |",
    "(o_ _o)\n/|   |\\\n~|   |~",
};
static const char* PENG_IDLE[] = {
    "(o' 'o)\n/|   |\\\n |   |",
    "(o- -o)\n/|   |\\\n |   |",
};
static const char* PENG_BUSY[] = {
    "(o' 'o)\n\\(|   |)\n |   |",
    "(o' 'o)\n(|   |)/\n |   |",
};
static const char* PENG_ATTENTION[] = {
    "(oO Oo)\n/|   |\\\n | ! |",
    "(oO Oo)\n\\(| ! |)/\n |   |",
};
static const char* PENG_CELEBRATE[] = {
    "\\(o^ ^o)/\n/|   |\\\n | * |",
    "(o* *o)\n\\(|   |)/\n~| * |~",
};
static const char* PENG_DIZZY[] = {
    "(o@ @o)\n/| ~ |\\\n~|   |",
    "(ox @o)\n/|   |\\\n |   |~",
};
static const char* PENG_HEART[] = {
    "(o^ ^o)\n/| v |\\\n |   |",
    "(ou uo)\n/| v |\\\n~|   |~",
};

// === GHOST ===
static const char* GHOST_SLEEP[] = {
    " .----.\n| -  - |\n|      |\n \\/\\/\\/",
    " .----.\n| -  - |\n|      |\n /\\/\\/\\",
};
static const char* GHOST_IDLE[] = {
    " .----.\n| o  o |\n|  --  |\n \\/\\/\\/",
    " .----.\n| -  - |\n|  --  |\n \\/\\/\\/",
};
static const char* GHOST_BUSY[] = {
    " .----.\n| o  o |\n|  oo  |\n \\/\\/\\/",
    "  .----.\n | o  o |\n |  oo  |\n  \\/\\/\\/",
};
static const char* GHOST_ATTENTION[] = {
    " .----.\n| O  O |\n|  !!  |\n \\/\\/\\/",
    " .!!!!.\n| O  O |\n|  OO  |\n \\/\\/\\/",
};
static const char* GHOST_CELEBRATE[] = {
    "*.----.*\n| ^  ^ |\n|  vv  |\n \\/\\/\\/",
    " .----.\n|*^  ^*|\n|  vv  |\n*\\/\\/\\/*",
};
static const char* GHOST_DIZZY[] = {
    " .----.\n| @  @ |\n|  ~~  |\n \\/\\/\\/",
    " .~~~~.\n| x  @ |\n|  --  |\n \\/\\/\\/",
};
static const char* GHOST_HEART[] = {
    " .----.\n| ^  ^ |\n|  uu  |\n \\/\\/\\/",
    " .vvvv.\n| u  u |\n|  ^^  |\n \\/\\/\\/",
};

// === ROBOT ===
static const char* ROBOT_SLEEP[] = {
    " [====]\n |-..-|\n |____|\n d|  |b",
    " [====]\n |-..-|\n |zzzz|\n d|  |b",
};
static const char* ROBOT_IDLE[] = {
    " [====]\n |o  o|\n | -- |\n d|  |b",
    " [====]\n |-  -|\n | -- |\n d|  |b",
};
static const char* ROBOT_BUSY[] = {
    " [====]\n |o  o|\n |>==<|\n d|  |b",
    " [====]\n |o  o|\n |<==<|\n d|  |b",
};
static const char* ROBOT_ATTENTION[] = {
    " [!!!!]\n |O  O|\n |!!!!|\n d|  |b",
    " [====]\n |O  O|\n | !! |\n d|  |b",
};
static const char* ROBOT_CELEBRATE[] = {
    "\\[====]/\n |^  ^|\n | ** |\n d|  |b",
    " [*==*]\n |^  ^|\n |****|\n d|  |b",
};
static const char* ROBOT_DIZZY[] = {
    " [~~~~]\n |@  @|\n | ~~ |\n d|  |b",
    " [====]\n |x  @|\n | -- |\n  |  |b",
};
static const char* ROBOT_HEART[] = {
    " [vvvv]\n |^  ^|\n | <3 |\n d|  |b",
    " [====]\n |u  u|\n |<33>|\n d|  |b",
};

// === BLOB ===
static const char* BLOB_SLEEP[] = {
    " .---.\n( -.- )\n `---'",
    " .---.\n( -.- )\n `~~~'",
};
static const char* BLOB_IDLE[] = {
    " .---.\n( o.o )\n `---'",
    " .---.\n( -.- )\n `---'",
};
static const char* BLOB_BUSY[] = {
    " .---.\n( o.o )>\n `---'",
    " .---.\n<( o.o )\n `---'",
};
static const char* BLOB_ATTENTION[] = {
    " .!!!.\n( O.O )\n `---'",
    " .---.\n( O O )\n `!!!'",
};
static const char* BLOB_CELEBRATE[] = {
    "*.---.*\n( ^.^ )\n `---'",
    " .---.\n(*^.^*)\n `~~~'",
};
static const char* BLOB_DIZZY[] = {
    " .---.\n( @.@ )\n `~~~'",
    " .~~~.\n( x.@ )\n `---'",
};
static const char* BLOB_HEART[] = {
    " .---.\n( ^.^ )\n `vvv'",
    " .vvv.\n( u.u )\n `---'",
};

// === OCTOPUS ===
static const char* OCTO_SLEEP[] = {
    " ,---.\n( -.- )\n/|||||\\",
    " ,---.\n( -.- )\n\\|||||/",
};
static const char* OCTO_IDLE[] = {
    " ,---.\n( o.o )\n/|||||\\",
    " ,---.\n( -.- )\n/|||||\\",
};
static const char* OCTO_BUSY[] = {
    " ,---.\n( o.o )\n/||!||\\",
    " ,---.\n( o.o )\n\\||!||/",
};
static const char* OCTO_ATTENTION[] = {
    " ,---.\n( O.O )!\n/|||||\\",
    " ,!!!.\n( O.O )\n\\|||||/",
};
static const char* OCTO_CELEBRATE[] = {
    "*,---.*\n( ^.^ )\n/|||||\\",
    " ,---.\n(*^.^*)\n\\|*|*|/",
};
static const char* OCTO_DIZZY[] = {
    " ,---.\n( @.@ )\n~|||||~",
    " ,~~~.\n( x.@ )\n/|||||\\",
};
static const char* OCTO_HEART[] = {
    " ,---.\n( ^.^ )\n/||v||\\",
    " ,vvv.\n( u.u )\n/|||||\\",
};

// === CAPYBARA ===
static const char* CAPY_SLEEP[] = {
    " .-----.\n( -__- )\n `-----'~",
    " .-----.\n( -__- )\n~`-----'",
};
static const char* CAPY_IDLE[] = {
    " .-----.\n( o__o )\n `-----'",
    " .-----.\n( -__- )\n `-----'",
};
static const char* CAPY_BUSY[] = {
    " .-----.\n( o__o )\n `-----'>",
    " .-----.\n( o__o )\n<`-----'",
};
static const char* CAPY_ATTENTION[] = {
    " .!!!!.\n( O__O )\n `-----'",
    " .-----.\n( O__O )!\n `-----'",
};
static const char* CAPY_CELEBRATE[] = {
    "*.-----.*\n( ^__^ )\n `-----'~",
    " .-----.\n(*^__^*)\n~`-----'",
};
static const char* CAPY_DIZZY[] = {
    " .-----.\n( @__@ )\n~`-----'",
    " .~~~~~.\n( x__@ )\n `-----'",
};
static const char* CAPY_HEART[] = {
    " .-----.\n( ^__^ )\n `--v--'",
    " .--v--.\n( u__u )\n `-----'",
};

// === DRAGON ===
static const char* DRAG_SLEEP[] = {
    " /\\_\n( -.- )~\n `===='",
    " /\\_\n( -.- )\n~`===='",
};
static const char* DRAG_IDLE[] = {
    " /\\_\n( o.o )~\n `===='",
    " /\\_\n( -.- )~\n `===='",
};
static const char* DRAG_BUSY[] = {
    " /\\_  *\n( o.o )~\n `===='",
    " /\\_ **\n( o.o )~\n `===='",
};
static const char* DRAG_ATTENTION[] = {
    " /\\_  !\n( O.O )~\n `===='",
    " /\\_ !!\n( O.O )~\n `!!=='",
};
static const char* DRAG_CELEBRATE[] = {
    "*/\\_*\n( ^.^ )~\n `===='",
    " /\\_  **\n(*^.^*)~\n*`===='",
};
static const char* DRAG_DIZZY[] = {
    " /\\_\n( @.@ )~\n~`===='",
    " /\\_\n( x.@ )\n `~~~~'",
};
static const char* DRAG_HEART[] = {
    " /\\_  v\n( ^.^ )~\n `===='",
    " /\\_ vv\n( u.u )~\n `===='",
};

// === GOOSE ===
static const char* GOOSE_SLEEP[] = {
    "  ,,\n (-.)\\\n/| |\n d d",
    "  ,,\n (-.)\\\n/| |\nd  d",
};
static const char* GOOSE_IDLE[] = {
    "  ,,\n (o.)\\\n/| |\n d d",
    "  ,,\n (-.)\\\n/| |\n d d",
};
static const char* GOOSE_BUSY[] = {
    "HONK\n  ,,\n (o.)\\>\n/| |\n d d",
    "  ,,\n<(o.)\\\n/| |\n d d",
};
static const char* GOOSE_ATTENTION[] = {
    " !!\n  ,,\n (O.)\\!\n/| |\n d d",
    "!!!\n  ,,\n!(O.)\\\n/| |\n d d",
};
static const char* GOOSE_CELEBRATE[] = {
    "* *\n  ,,\n (^.)\\*\n/| |\n d d",
    "*  *\n  ,,\n*(^.)\\\n/| |\n d d",
};
static const char* GOOSE_DIZZY[] = {
    "  ,,\n (@.)\\~\n/| |\n~d d",
    "  ~~\n~(x.)\\\n/| |\n d d~",
};
static const char* GOOSE_HEART[] = {
    " vv\n  ,,\n (^.)\\v\n/| |\n d d",
    "v\n  ,,\n (u.)\\\n/| |\n d d",
};

// === OWL ===
static const char* OWL_SLEEP[] = {
    " {---}\n (-.-)  \n /)_)\\",
    " {---}\n (-.-)  \n /(_(\\",
};
static const char* OWL_IDLE[] = {
    " {---}\n (O.O)\n /)_)\\",
    " {---}\n (o.o)\n /)_)\\",
};
static const char* OWL_BUSY[] = {
    " {---}\n (O.O)?\n /)_)\\",
    " {---}\n?(O.O)\n /)_)\\",
};
static const char* OWL_ATTENTION[] = {
    " {!!!}\n (O!O)\n /)_)\\",
    " {---}\n (O O)!\n /)_)\\",
};
static const char* OWL_CELEBRATE[] = {
    "*{---}*\n (^.^)\n /)_)\\",
    " {***}\n*(^.^)*\n /)_)\\",
};
static const char* OWL_DIZZY[] = {
    " {~~~}\n (@.@)\n /)_)\\",
    " {---}\n (x.@)\n~/)_)\\",
};
static const char* OWL_HEART[] = {
    " {vvv}\n (^.^)\n /)_)\\",
    " {---}\n (u.u)v\n /)_)\\",
};

// === RABBIT ===
static const char* RABB_SLEEP[] = {
    "(\\_/)\n( -.- )\no(\")(\")",
    "(\\_/)\n( -.- )\no(\")(\")",
};
static const char* RABB_IDLE[] = {
    "(\\_/)\n( o.o )\n(\")(\")",
    "(\\_/)\n( -.- )\n(\")(\")",
};
static const char* RABB_BUSY[] = {
    "(\\_/)\n( o.o )\n>(\")(\")",
    "(\\_/)\n( o.o )\n(\")(\")<",
};
static const char* RABB_ATTENTION[] = {
    "(\\!/)\n( O.O )\n(\")(\")",
    "(\\_/)!\n( O O )\n(\")(\")",
};
static const char* RABB_CELEBRATE[] = {
    "*(\\*/)*\n( ^.^ )\n(\")(\")",
    "(\\_/)\n*( ^.^ )*\n(\")(\")",
};
static const char* RABB_DIZZY[] = {
    "(\\~/)\n( @.@ )\n~(\")(\")",
    "(\\_/)\n( x.@ )\n(\")(\")~",
};
static const char* RABB_HEART[] = {
    "(\\v/)\n( ^.^ )\n(\")(\")",
    "(\\_/)\n( u.u )v\n(\")(\")",
};

// === TURTLE ===
static const char* TURT_SLEEP[] = {
    "  ___\n/(-.-)\\  \n/|===|\\",
    "  ___\n/(-.-)\\  \n/ |===| \\",
};
static const char* TURT_IDLE[] = {
    "  ___\n/(o.o)\\\n/|===|\\",
    "  ___\n/(-.-)\\  \n/|===|\\",
};
static const char* TURT_BUSY[] = {
    "  ___\n/(o.o)\\ >\n/|===|\\",
    "   ___\n< /(o.o)\\\n /|===|\\",
};
static const char* TURT_ATTENTION[] = {
    "  _!_\n/(O.O)\\\n/|===|\\",
    "  ___ !\n/(O O)\\\n/|===|\\",
};
static const char* TURT_CELEBRATE[] = {
    " *___*\n/(^.^)\\\n/|===|\\",
    "  ___\n*/(^.^)\\*\n/|===|\\",
};
static const char* TURT_DIZZY[] = {
    "  ~~~\n/(@.@)\\\n~/ |===|\\",
    "  ___\n/(x.@)\\\n/|===|\\~",
};
static const char* TURT_HEART[] = {
    "  _v_\n/(^.^)\\\n/|===|\\",
    "  ___\n/(u.u)\\v\n/|===|\\",
};

// === SNAIL ===
static const char* SNAIL_SLEEP[] = {
    "  @\n/(-.-)  \n/_____/",
    "  @\n/(-.-)  \n/_____/",
};
static const char* SNAIL_IDLE[] = {
    "  @\n/(o.o)\n/_____/",
    "  @\n/(-.-)  \n/_____/",
};
static const char* SNAIL_BUSY[] = {
    "  @\n/(o.o)\n/_____/>",
    "  @\n/(o.o)\n /_____/",
};
static const char* SNAIL_ATTENTION[] = {
    "  @ !\n/(O.O)\n/_____/",
    "  @\n!/(O O)\n/_____/",
};
static const char* SNAIL_CELEBRATE[] = {
    " *@*\n/(^.^)\n/_____/",
    "  @\n*/(^.^)\n/_____/*",
};
static const char* SNAIL_DIZZY[] = {
    "  ~\n/(@.@)\n~/_____/",
    "  @\n/(x.@)\n/_____/~",
};
static const char* SNAIL_HEART[] = {
    "  @ v\n/(^.^)\n/_____/",
    "  @\n/(u.u)v\n/_____/",
};

// === MUSHROOM ===
static const char* MUSH_SLEEP[] = {
    " .oOo.\n (-.-)  \n  | |",
    " .oOo.\n (-.-)  \n ~| |~",
};
static const char* MUSH_IDLE[] = {
    " .oOo.\n (o.o)\n  | |",
    " .oOo.\n (-.-)  \n  | |",
};
static const char* MUSH_BUSY[] = {
    " .oOo.\n (o.o)\n  | |>",
    " .oOo.\n (o.o)\n <| |",
};
static const char* MUSH_ATTENTION[] = {
    " .o!o.\n (O.O)\n  |!|",
    " .oOo. !\n (O O)\n  | |",
};
static const char* MUSH_CELEBRATE[] = {
    "*.oOo.*\n (^.^)\n  | |",
    " .o*o.\n*(^.^)*\n  | |",
};
static const char* MUSH_DIZZY[] = {
    " .o~o.\n (@.@)\n ~| |",
    " .oOo.\n (x.@)\n  | |~",
};
static const char* MUSH_HEART[] = {
    " .ovo.\n (^.^)\n  | |",
    " .oOo.\n (u.u)v\n  | |",
};

// === CACTUS ===
static const char* CACT_SLEEP[] = {
    " .|.\n (-.-)  \n.|.|.",
    " .|.\n (-.-)  \n.|.|.",
};
static const char* CACT_IDLE[] = {
    " .|.\n (o.o)\n.|.|.",
    " .|.\n (-.-)  \n.|.|.",
};
static const char* CACT_BUSY[] = {
    " .|.\n (o.o)\n.|.|.>",
    " .|.\n (o.o)\n<.|.|.",
};
static const char* CACT_ATTENTION[] = {
    " .|. !\n (O.O)\n.|.|.",
    " .!.\n (O O)!\n.|.|.",
};
static const char* CACT_CELEBRATE[] = {
    "*.|.*\n (^.^)\n.|.|.",
    " .|.\n*(^.^)*\n*.|.|.*",
};
static const char* CACT_DIZZY[] = {
    " .~.\n (@.@)\n~.|.|.",
    " .|.\n (x.@)\n.|.|.~",
};
static const char* CACT_HEART[] = {
    " .|. v\n (^.^)\n.|.|.",
    " .v.\n (u.u)\n.|.|.",
};

// === CHONK ===
static const char* CHONK_SLEEP[] = {
    ".------.\n( -.__.- )\n`------'",
    ".------.\n( -.__.- )\n`~~~~~~'",
};
static const char* CHONK_IDLE[] = {
    ".------.\n( o.__. o)\n`------'",
    ".------.\n( -.__.- )\n`------'",
};
static const char* CHONK_BUSY[] = {
    ".------.\n( o.__. o)\n`------'>",
    ".------.\n( o.__. o)\n<`------'",
};
static const char* CHONK_ATTENTION[] = {
    ".!!!!!!.\n( O.__. O)\n`------'",
    ".------.\n( O .  O)!\n`------'",
};
static const char* CHONK_CELEBRATE[] = {
    "*.------.*\n( ^.__. ^)\n`------'",
    ".------.\n(*^.__. ^)\n`------'*",
};
static const char* CHONK_DIZZY[] = {
    ".~~~~~~.\n( @.__. @)\n~`------'",
    ".------.\n( x.__. @)\n`------'~",
};
static const char* CHONK_HEART[] = {
    ".--vv--.\n( ^.__. ^)\n`------'",
    ".------.\n( u.__. u)\n`--vv--'",
};

// === AXOLOTL ===
static const char* AXOL_SLEEP[] = {
    " ~(-.-)~\n  /|||\\",
    " ~(-.-)~\n  \\|||/",
};
static const char* AXOL_IDLE[] = {
    " ~(o.o)~\n  /|||\\",
    " ~(-.-)~\n  /|||\\",
};
static const char* AXOL_BUSY[] = {
    " ~(o.o)~>\n  /|||\\",
    "<~(o.o)~\n  /|||\\",
};
static const char* AXOL_ATTENTION[] = {
    " ~(O.O)~!\n  /|||\\",
    "!~(O.O)~\n  /|||\\",
};
static const char* AXOL_CELEBRATE[] = {
    "*~(^.^)~*\n  /|||\\",
    " ~(^.^)~\n  /|*|\\",
};
static const char* AXOL_DIZZY[] = {
    " ~(@.@)~\n  /|||\\",
    " ~(x.@)~\n  \\|||/",
};
static const char* AXOL_HEART[] = {
    " ~(^.^)~v\n  /|||\\",
    "v~(u.u)~\n  /|||\\",
};

struct StateAnim {
    const char** frames;
    uint8_t frame_count;
    const uint8_t* seq;
    uint8_t seq_len;
    uint8_t beat_div;  // tick divider (higher = slower)
};

struct SpeciesData {
    const char* name;
    StateAnim states[7];
};

// Helper: simple 2-frame alternating (for species not yet enhanced)
#define ANIM2(f, spd) { f, 2, nullptr, 0, spd }
// Helper: SEQ-based animation
#define ANIM_SEQ(f, fc, s, sl, spd) { f, fc, s, sl, spd }

// === CAT enhanced sequences (ported from original) ===

// Sleep: 6 poses, loaf/breathe/curl cycle
static const char* CAT_SLEEP_F[] = {
    " .-..-.\n( -.- )\n`------`",       // 0: loaf
    " .-..-.\n( -.- )_\n`~------'",     // 1: breathe
    "  .-/\\.\n(  ..  ))\n `~~~~~~`",   // 2: curl
    "  .-/\\.\n(  ..  ))\n `~~~~~~`~",  // 3: curl twitch
    " .-..-.\n( u.u )\n`~------'",      // 4: purr
    " .-..-.\n( o.o )\n`------`",       // 5: dream
};
static const uint8_t CAT_SLEEP_SEQ[] = {
    0,1,0,1,0,1, 4,4,0,1, 2,3,2,3,2,3, 5,5, 0,1,0,1, 3,3,2,2
};

// Idle: 10 poses, sassy micro-actions
static const char* CAT_IDLE_F[] = {
    " /\\_/\\\n( o o )\n(  w  )\n(\")_(\")",    // 0: rest
    " /\\_/\\\n(o   o )\n(  w  )\n(\")_(\")",   // 1: look left
    " /\\_/\\\n( o   o)\n(  w  )\n(\")_(\")",   // 2: look right
    " /\\_/\\\n( -  - )\n(  w  )\n(\")_(\")",   // 3: blink
    " /\\-/\\\n( _  _ )\n(  w  )\n(\")_(\")",   // 4: slow blink
    " <\\_/\\\n( o  o )\n(  w  )\n(\")_(\")",   // 5: ear left
    " /\\_/>\n( o  o )\n(  w  )\n(\")_(\")",    // 6: ear right
    " /\\_/\\\n( o  o )\n(  w  )\n(\")_(\")\x7e", // 7: tail left
    " /\\_/\\\n( o  o )\n(  w  )\n\x7e(\")_(\")", // 8: tail right
    " /\\_/\\\n( ^  ^ )\n(  P  )\n(\")_(\")",   // 9: groom
};
static const uint8_t CAT_IDLE_SEQ[] = {
    0,0,0,3,0,1,0,2,0, 7,8,7,8,7, 0,5,0,6,0, 4,4,0, 9,9,9,0, 0,3,0, 8,7,8,7, 0,0,4,0
};

// Busy: 6 poses, pawing things off table
static const char* CAT_BUSY_F[] = {
    " /\\_/\\\n( o o )\n(  w  )/\n(\")_(\")",   // 0: paw up
    " /\\_/\\\n( o o )\n(  w  )_\n(\")_(\")",   // 1: paw tap
    " /\\_/\\\n( O O )\n(  w  )\n(\")_(\")",    // 2: stare
    " /\\_/\\\n( o o )\n( -w  )\n(\")_(\")",    // 3: nudge
    " /\\_/\\\n( o o )\n(-w   )\n(\")_(\")",    // 4: shove
    " /\\_/\\\n( -  - )\n(  w  )\n(\")_(\")",   // 5: smug
};
static const uint8_t CAT_BUSY_SEQ[] = {
    2,2,2, 0,1,0,1, 3,4,3,4, 5,5, 2,2, 0,1,0,1, 5,2
};

// Attention: 6 poses, ears up dilated pupils
static const char* CAT_ATT_F[] = {
    " /^_^\\\n( O O )\n(  v  )\n(\")_(\")",    // 0: alert
    " /^_^\\\n(O   O )\n(  v  )\n(\")_(\")",   // 1: scan left
    " /^_^\\\n( O   O)\n(  v  )\n(\")_(\")",   // 2: scan right
    " /^_^\\\n( ^  ^ )\n(  v  )\n(\")_(\")",   // 3: scan up
    " /^_^\\\n( O O )\n(  v  )\n/(\")\\_(\")\\", // 4: crouch
    " /^_^\\\n( O O )\n(  >  )\n(\")_(\")",    // 5: hiss
};
static const uint8_t CAT_ATT_SEQ[] = {
    0,4,0,1,0,2,0,3, 4,4,0,1,2,0, 5,0
};

// Celebrate: 6 poses, zoomies
static const char* CAT_CEL_F[] = {
    " /\\_/\\\n( ^ ^ )\n(  W  )\n/(\")_(\")\\" , // 0: crouch
    "\\^ ^/\n /\\_/\\\n( ^ ^ )\n(  W  )",       // 1: jump
    "\\^ ^/\n /\\_/\\\n( * * )\n(  W  )",        // 2: peak
    " /\\_/\\\n( < < )\n(  W  ) /\n~(\")_(\")",  // 3: spin L
    " /\\_/\\\n( > > )\n\\(  W  )\n(\")_(\")\x7e", // 4: spin R
    " \\o/\n /\\_/\\\n( ^ ^ )\n/(  W  )\\",      // 5: pose
};
static const uint8_t CAT_CEL_SEQ[] = {
    0,1,2,1,0, 3,4,3,4, 0,1,2,1,0, 5,5
};

// Dizzy: 5 poses, chasing tail
static const char* CAT_DIZ_F[] = {
    " /\\_/\\\n( @ @ )\n( ~~  )\n(\")_(\")",    // 0: tilt L
    " /\\_/\\\n( @ @ )\n( ~~  )\n(\")_(\")",    // 1: tilt R
    " /\\_/\\\n( x @ )\n(  v  )\n(\")_(\")\x7e", // 2: woozy
    " /\\_/\\\n( @ x )\n(  v  )\n\x7e(\")_(\")", // 3: woozy2
    " /\\_/\\\n( @ @ )\n(  -  )\n/(\")_(\")\\" , // 4: splat
};
static const uint8_t CAT_DIZ_SEQ[] = {
    0,1,0,1, 2,3, 0,1,0,1, 4,4, 2,3
};

// Heart: 5 poses, smitten purr
static const char* CAT_HRT_F[] = {
    " /\\_/\\\n( ^ ^ )\n(  u  )\n(\")_(\")\x7e", // 0: dreamy
    " /\\_/\\\n(#^ ^#)\n(  u  )\n(\")_(\")",    // 1: blush
    " /\\_/\\\n( <3<3 )\n(  u  )\n(\")_(\")\x7e", // 2: heart eyes
    " /\\-/\\\n( ~ ~ )\n(  u  )\n\x7e(\")_(\")~", // 3: purr
    " /\\_/\\\n( ^ - )\n(  u  )\n(\")_(\")",    // 4: head tilt
};
static const uint8_t CAT_HRT_SEQ[] = {
    0,0,1,0, 2,2,0, 1,0,4, 0,0,3,3, 0,1,0,2, 1,0
};

static const SpeciesData SPECIES[] = {
    // Cat — enhanced with full SEQ sequences
    {"Cat", {
        ANIM_SEQ(CAT_SLEEP_F, 6, CAT_SLEEP_SEQ, sizeof(CAT_SLEEP_SEQ), 5),
        ANIM_SEQ(CAT_IDLE_F, 10, CAT_IDLE_SEQ, sizeof(CAT_IDLE_SEQ), 5),
        ANIM_SEQ(CAT_BUSY_F, 6, CAT_BUSY_SEQ, sizeof(CAT_BUSY_SEQ), 5),
        ANIM_SEQ(CAT_ATT_F, 6, CAT_ATT_SEQ, sizeof(CAT_ATT_SEQ), 5),
        ANIM_SEQ(CAT_CEL_F, 6, CAT_CEL_SEQ, sizeof(CAT_CEL_SEQ), 3),
        ANIM_SEQ(CAT_DIZ_F, 5, CAT_DIZ_SEQ, sizeof(CAT_DIZ_SEQ), 4),
        ANIM_SEQ(CAT_HRT_F, 5, CAT_HRT_SEQ, sizeof(CAT_HRT_SEQ), 5),
    }},
    {"Duck", {
        ANIM_SEQ(DUCK_SLEEP_F, 4, DUCK_SLEEP_SEQ, sizeof(DUCK_SLEEP_SEQ), 5),
        ANIM_SEQ(DUCK_IDLE_F, 5, DUCK_IDLE_SEQ, sizeof(DUCK_IDLE_SEQ), 5),
        ANIM_SEQ(DUCK_BUSY_F, 3, DUCK_BUSY_SEQ, sizeof(DUCK_BUSY_SEQ), 5),
        ANIM_SEQ(DUCK_ATT_F, 3, DUCK_ATT_SEQ, sizeof(DUCK_ATT_SEQ), 4),
        ANIM_SEQ(DUCK_CEL_F, 3, DUCK_CEL_SEQ, sizeof(DUCK_CEL_SEQ), 3),
        ANIM_SEQ(DUCK_DIZ_F, 3, DUCK_DIZ_SEQ, sizeof(DUCK_DIZ_SEQ), 4),
        ANIM_SEQ(DUCK_HRT_F, 3, DUCK_HRT_SEQ, sizeof(DUCK_HRT_SEQ), 5),
    }},
    {"Penguin", {
        ANIM2(PENG_SLEEP, 10), ANIM2(PENG_IDLE, 15), ANIM2(PENG_BUSY, 5),
        ANIM2(PENG_ATTENTION, 4), ANIM2(PENG_CELEBRATE, 3), ANIM2(PENG_DIZZY, 4), ANIM2(PENG_HEART, 8),
    }},
    {"Ghost", {
        ANIM2(GHOST_SLEEP, 10), ANIM2(GHOST_IDLE, 15), ANIM2(GHOST_BUSY, 5),
        ANIM2(GHOST_ATTENTION, 4), ANIM2(GHOST_CELEBRATE, 3), ANIM2(GHOST_DIZZY, 4), ANIM2(GHOST_HEART, 8),
    }},
    {"Robot", {
        ANIM2(ROBOT_SLEEP, 10), ANIM2(ROBOT_IDLE, 15), ANIM2(ROBOT_BUSY, 5),
        ANIM2(ROBOT_ATTENTION, 4), ANIM2(ROBOT_CELEBRATE, 3), ANIM2(ROBOT_DIZZY, 4), ANIM2(ROBOT_HEART, 8),
    }},
    {"Blob", {
        ANIM2(BLOB_SLEEP, 10), ANIM2(BLOB_IDLE, 15), ANIM2(BLOB_BUSY, 5),
        ANIM2(BLOB_ATTENTION, 4), ANIM2(BLOB_CELEBRATE, 3), ANIM2(BLOB_DIZZY, 4), ANIM2(BLOB_HEART, 8),
    }},
    {"Octopus", {
        ANIM2(OCTO_SLEEP, 10), ANIM2(OCTO_IDLE, 15), ANIM2(OCTO_BUSY, 5),
        ANIM2(OCTO_ATTENTION, 4), ANIM2(OCTO_CELEBRATE, 3), ANIM2(OCTO_DIZZY, 4), ANIM2(OCTO_HEART, 8),
    }},
    {"Capybara", {
        ANIM2(CAPY_SLEEP, 10), ANIM2(CAPY_IDLE, 15), ANIM2(CAPY_BUSY, 5),
        ANIM2(CAPY_ATTENTION, 4), ANIM2(CAPY_CELEBRATE, 3), ANIM2(CAPY_DIZZY, 4), ANIM2(CAPY_HEART, 8),
    }},
    {"Dragon", {
        ANIM2(DRAG_SLEEP, 10), ANIM2(DRAG_IDLE, 15), ANIM2(DRAG_BUSY, 5),
        ANIM2(DRAG_ATTENTION, 4), ANIM2(DRAG_CELEBRATE, 3), ANIM2(DRAG_DIZZY, 4), ANIM2(DRAG_HEART, 8),
    }},
    {"Goose", {
        ANIM2(GOOSE_SLEEP, 10), ANIM2(GOOSE_IDLE, 15), ANIM2(GOOSE_BUSY, 5),
        ANIM2(GOOSE_ATTENTION, 4), ANIM2(GOOSE_CELEBRATE, 3), ANIM2(GOOSE_DIZZY, 4), ANIM2(GOOSE_HEART, 8),
    }},
    {"Owl", {
        ANIM2(OWL_SLEEP, 10), ANIM2(OWL_IDLE, 15), ANIM2(OWL_BUSY, 5),
        ANIM2(OWL_ATTENTION, 4), ANIM2(OWL_CELEBRATE, 3), ANIM2(OWL_DIZZY, 4), ANIM2(OWL_HEART, 8),
    }},
    {"Rabbit", {
        ANIM2(RABB_SLEEP, 10), ANIM2(RABB_IDLE, 15), ANIM2(RABB_BUSY, 5),
        ANIM2(RABB_ATTENTION, 4), ANIM2(RABB_CELEBRATE, 3), ANIM2(RABB_DIZZY, 4), ANIM2(RABB_HEART, 8),
    }},
    {"Turtle", {
        ANIM2(TURT_SLEEP, 10), ANIM2(TURT_IDLE, 15), ANIM2(TURT_BUSY, 5),
        ANIM2(TURT_ATTENTION, 4), ANIM2(TURT_CELEBRATE, 3), ANIM2(TURT_DIZZY, 4), ANIM2(TURT_HEART, 8),
    }},
    {"Snail", {
        ANIM2(SNAIL_SLEEP, 10), ANIM2(SNAIL_IDLE, 15), ANIM2(SNAIL_BUSY, 5),
        ANIM2(SNAIL_ATTENTION, 4), ANIM2(SNAIL_CELEBRATE, 3), ANIM2(SNAIL_DIZZY, 4), ANIM2(SNAIL_HEART, 8),
    }},
    {"Mushroom", {
        ANIM2(MUSH_SLEEP, 10), ANIM2(MUSH_IDLE, 15), ANIM2(MUSH_BUSY, 5),
        ANIM2(MUSH_ATTENTION, 4), ANIM2(MUSH_CELEBRATE, 3), ANIM2(MUSH_DIZZY, 4), ANIM2(MUSH_HEART, 8),
    }},
    {"Cactus", {
        ANIM2(CACT_SLEEP, 10), ANIM2(CACT_IDLE, 15), ANIM2(CACT_BUSY, 5),
        ANIM2(CACT_ATTENTION, 4), ANIM2(CACT_CELEBRATE, 3), ANIM2(CACT_DIZZY, 4), ANIM2(CACT_HEART, 8),
    }},
    {"Chonk", {
        ANIM2(CHONK_SLEEP, 10), ANIM2(CHONK_IDLE, 15), ANIM2(CHONK_BUSY, 5),
        ANIM2(CHONK_ATTENTION, 4), ANIM2(CHONK_CELEBRATE, 3), ANIM2(CHONK_DIZZY, 4), ANIM2(CHONK_HEART, 8),
    }},
    {"Axolotl", {
        ANIM2(AXOL_SLEEP, 10), ANIM2(AXOL_IDLE, 15), ANIM2(AXOL_BUSY, 5),
        ANIM2(AXOL_ATTENTION, 4), ANIM2(AXOL_CELEBRATE, 3), ANIM2(AXOL_DIZZY, 4), ANIM2(AXOL_HEART, 8),
    }},
};

static const uint8_t NUM_SPECIES = sizeof(SPECIES) / sizeof(SPECIES[0]);

void buddy_pet_init() {
    s_state = 0;
    s_tick = 0;
    s_species = 0;
}

void buddy_pet_set_species(uint8_t idx) {
    if (idx < NUM_SPECIES) s_species = idx;
}

uint8_t buddy_pet_get_species() {
    return s_species;
}

uint8_t buddy_pet_species_count() {
    return NUM_SPECIES;
}

const char* buddy_pet_get_species_name() {
    return SPECIES[s_species].name;
}

void buddy_pet_set_state(uint8_t persona_state) {
    if (persona_state != s_state) {
        s_state = persona_state;
        s_tick = 0;
    }
    s_tick++;
}

const char* buddy_pet_get_frame() {
    if (s_state >= 7) s_state = 1;
    const StateAnim& anim = SPECIES[s_species].states[s_state];
    uint8_t beat = (s_tick / anim.beat_div);

    uint8_t frame_idx;
    if (anim.seq && anim.seq_len > 0) {
        frame_idx = anim.seq[beat % anim.seq_len];
    } else {
        frame_idx = beat % anim.frame_count;
    }

    if (frame_idx >= anim.frame_count) frame_idx = 0;
    return anim.frames[frame_idx];
}
