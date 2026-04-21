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
static const char* DUCK_SLEEP[] = {
    " (__)\n (-.-)\n~(___)~",
    " (__)\n (-.-)\n~~(___)~",
};
static const char* DUCK_IDLE[] = {
    " (__)\n (o o)\n>(___)>",
    " (__)\n (- -)\n>(___)>",
};
static const char* DUCK_BUSY[] = {
    " (__)\n (o o)\n>(___)> !",
    " (__)\n (o o)\n<(___)< .",
};
static const char* DUCK_ATTENTION[] = {
    "  !\n (__)\n (O O)\n>(___)>",
    " !!\n (__)\n (O O)\n<(___)<",
};
static const char* DUCK_CELEBRATE[] = {
    "~\n (__)\n (^ ^)\n>(___)>*",
    "   ~\n (__)\n (* *)\n*<(___)<",
};
static const char* DUCK_DIZZY[] = {
    " (__)\n (@ @)\n~(___)~",
    " (__)\n (x @)\n~(___)~",
};
static const char* DUCK_HEART[] = {
    "  v\n (__)\n (^ ^)\n>(___)>",
    " v\n (__)\n (u u)\n>(___)>",
};

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

struct SpeciesData {
    const char** frames[7];
};

static const SpeciesData SPECIES[] = {
    {{ CAT_SLEEP, CAT_IDLE, CAT_BUSY, CAT_ATTENTION, CAT_CELEBRATE, CAT_DIZZY, CAT_HEART }},
    {{ DUCK_SLEEP, DUCK_IDLE, DUCK_BUSY, DUCK_ATTENTION, DUCK_CELEBRATE, DUCK_DIZZY, DUCK_HEART }},
    {{ PENG_SLEEP, PENG_IDLE, PENG_BUSY, PENG_ATTENTION, PENG_CELEBRATE, PENG_DIZZY, PENG_HEART }},
    {{ GHOST_SLEEP, GHOST_IDLE, GHOST_BUSY, GHOST_ATTENTION, GHOST_CELEBRATE, GHOST_DIZZY, GHOST_HEART }},
    {{ ROBOT_SLEEP, ROBOT_IDLE, ROBOT_BUSY, ROBOT_ATTENTION, ROBOT_CELEBRATE, ROBOT_DIZZY, ROBOT_HEART }},
    {{ BLOB_SLEEP, BLOB_IDLE, BLOB_BUSY, BLOB_ATTENTION, BLOB_CELEBRATE, BLOB_DIZZY, BLOB_HEART }},
    {{ OCTO_SLEEP, OCTO_IDLE, OCTO_BUSY, OCTO_ATTENTION, OCTO_CELEBRATE, OCTO_DIZZY, OCTO_HEART }},
    {{ CAPY_SLEEP, CAPY_IDLE, CAPY_BUSY, CAPY_ATTENTION, CAPY_CELEBRATE, CAPY_DIZZY, CAPY_HEART }},
    {{ DRAG_SLEEP, DRAG_IDLE, DRAG_BUSY, DRAG_ATTENTION, DRAG_CELEBRATE, DRAG_DIZZY, DRAG_HEART }},
    {{ GOOSE_SLEEP, GOOSE_IDLE, GOOSE_BUSY, GOOSE_ATTENTION, GOOSE_CELEBRATE, GOOSE_DIZZY, GOOSE_HEART }},
    {{ OWL_SLEEP, OWL_IDLE, OWL_BUSY, OWL_ATTENTION, OWL_CELEBRATE, OWL_DIZZY, OWL_HEART }},
    {{ RABB_SLEEP, RABB_IDLE, RABB_BUSY, RABB_ATTENTION, RABB_CELEBRATE, RABB_DIZZY, RABB_HEART }},
    {{ TURT_SLEEP, TURT_IDLE, TURT_BUSY, TURT_ATTENTION, TURT_CELEBRATE, TURT_DIZZY, TURT_HEART }},
    {{ SNAIL_SLEEP, SNAIL_IDLE, SNAIL_BUSY, SNAIL_ATTENTION, SNAIL_CELEBRATE, SNAIL_DIZZY, SNAIL_HEART }},
    {{ MUSH_SLEEP, MUSH_IDLE, MUSH_BUSY, MUSH_ATTENTION, MUSH_CELEBRATE, MUSH_DIZZY, MUSH_HEART }},
    {{ CACT_SLEEP, CACT_IDLE, CACT_BUSY, CACT_ATTENTION, CACT_CELEBRATE, CACT_DIZZY, CACT_HEART }},
    {{ CHONK_SLEEP, CHONK_IDLE, CHONK_BUSY, CHONK_ATTENTION, CHONK_CELEBRATE, CHONK_DIZZY, CHONK_HEART }},
};

static const uint8_t FRAME_COUNT = 2;
static const uint8_t SPEEDS[] = { 10, 15, 5, 4, 3, 4, 8 };
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

void buddy_pet_set_state(uint8_t persona_state) {
    if (persona_state != s_state) {
        s_state = persona_state;
        s_tick = 0;
    }
    s_tick++;
}

const char* buddy_pet_get_frame() {
    if (s_state >= 7) s_state = 1;
    uint8_t speed = SPEEDS[s_state];
    uint8_t frame_idx = (s_tick / speed) % FRAME_COUNT;
    return SPECIES[s_species].frames[s_state][frame_idx];
}
