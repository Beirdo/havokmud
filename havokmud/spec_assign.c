
#include "config.h"
#include <stdio.h>

#include "protos.h"

#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif

extern struct index_data *mob_index;
extern struct index_data *obj_index;
void            boot_the_shops();
void            assign_the_shopkeepers();

struct special_proc_entry {
    int             vnum;
    int             (*proc) ();
};

/*
 ********************************************************************
 *  Assignments                                                        *
 ******************************************************************** */

/*
 * murder is disabled for now 
 */
#define MAX_MUTYPE 16
int is_murdervict(struct char_data *ch)
{
#if 0
    int             i;
    int             mutype[MAX_MUTYPE] = {
        3060,                   /* killing these mobs will make the PC a
                                 * murderer */
        3067,
        3069,
        3072,
        3141,
        3661,
        3662,
        3663,
        3682,
        16101,
        17809,
        18215,
        18222,
        18223,
        22601,
        27011
    };

    for (i = 0; i < MAX_MUTYPE; i++) {
        if (mob_index[ch->nr].virtual == mutype[i]) {
            return (TRUE);
        }
    }
#endif
    return (FALSE);
}

/*
 * assign special procedures to mobiles 
 */
void assign_mobiles()
{
    static struct special_proc_entry specials[] = {

        {1, remort_guild},
        {2, Ringwraith},
        {3, tormentor},
#if 0
        {4, Inquisitor},
        {6, AcidBlob },
#endif        
        {25, magic_user},

        {30, MageGuildMaster},
        {31, ClericGuildMaster},
        {32, ThiefGuildMaster},
        {33, WarriorGuildMaster},
        {34, MageGuildMaster},
        {35, ClericGuildMaster},
        {36, ThiefGuildMaster},
        {37, WarriorGuildMaster},
        {38, fighter},          /* armor golem */
        {39, creeping_death},

        {10000, QuestMobProc},  /* TEMP. Assignment!!!! */

        {199, AGGRESSIVE},
        {200, AGGRESSIVE},
        /*
         * Doom Island
         */

        {205, fighter},
        {206, fighter},
        {208, fighter},
        {208, thief},
        {209, fighter},
        {210, cleric},
        {211, magic_user},
        {212, fighter},
        {213, Barbarian},
        {214, thief},
        {215, cleric},
        {216, magic_user},
        {217, fighter},
        {218, fighter},
        /*
         * end doom island 
         */

        /*
         **  D&D Standard MOBS
         */

#if 0
        {210, snake},           /* spider */
        {211, fighter},         /* gnoll */
#endif

        {220, fighter},         /* fighter */
        {221, fighter},         /* bugbear */
        {223, ghoul},           /* ghoul */
        {226, fighter},         /* ogre */
        {236, ghoul},           /* ghast */
        {227, snake},           /* spider */
        {230, BreathWeapon},    /* baby black */
        {232, blink},           /* blink dog */
        {233, BreathWeapon},    /* baby blue */
        {234, cleric},          /* cleric */
        {239, shadow},          /* shadow */
        {240, snake},           /* toad */
        {243, BreathWeapon},    /* teenage white */
        {247, fighter},         /* minotaur */
        {251, CarrionCrawler},
        {261, fighter},
        {262, regenerator},
        {264, fighter},
        {265, magic_user},
        {266, magic_user},
        {267, Devil},
        {269, Demon},
        {271, regenerator},
        {248, snake},           /* snake */
        {249, snake},           /* snake */
        {250, snake},           /* snake */
        {257, magic_user},      /* magic_user */

        {600, DruidChallenger},
        {601, DruidChallenger},
        {602, DruidChallenger},
        {603, DruidChallenger},
        {604, DruidChallenger},
        {605, DruidChallenger},
        {606, DruidChallenger},
        {607, DruidChallenger},
        {608, DruidChallenger},
        {609, DruidChallenger},
        {610, DruidChallenger},
        {611, DruidChallenger},
        {612, DruidChallenger},
        {613, DruidChallenger},
        {614, DruidChallenger},
        {615, DruidChallenger},
        {616, DruidChallenger},
        {617, DruidChallenger},
        {618, DruidChallenger},
        {619, DruidChallenger},
        {620, DruidChallenger},
        {621, DruidChallenger},
        {622, DruidChallenger},
        {623, DruidChallenger},
        {624, DruidChallenger},
        {625, DruidChallenger},
        {626, DruidChallenger},
        {627, DruidChallenger},
        {628, DruidChallenger},
        {629, DruidChallenger},
        {630, DruidChallenger},
        {631, DruidChallenger},
        {632, DruidChallenger},
        {633, DruidChallenger},
        {634, DruidChallenger},
        {635, DruidChallenger},
        {636, DruidChallenger},
        {637, DruidChallenger},
        {638, DruidChallenger},
        {639, DruidChallenger},
        {640, DruidChallenger},
        {641, DruidGuildMaster},
        {642, DruidGuildMaster},

        {651, MonkChallenger},
        {652, MonkChallenger},
        {653, MonkChallenger},
        {654, MonkChallenger},
        {655, MonkChallenger},
        {656, MonkChallenger},
        {657, MonkChallenger},
        {658, MonkChallenger},
        {659, MonkChallenger},
        {660, MonkChallenger},
        {661, MonkChallenger},
        {662, MonkChallenger},
        {663, MonkChallenger},
        {664, MonkChallenger},
        {665, MonkChallenger},
        {666, MonkChallenger},
        {667, MonkChallenger},
        {668, MonkChallenger},
        {669, MonkChallenger},
        {670, MonkChallenger},
        {671, MonkChallenger},
        {672, MonkChallenger},
        {673, MonkChallenger},
        {674, MonkChallenger},
        {675, MonkChallenger},
        {676, MonkChallenger},
        {677, MonkChallenger},
        {678, MonkChallenger},
        {679, MonkChallenger},
        {680, MonkChallenger},
        {681, MonkChallenger},
        {682, MonkChallenger},
        {683, MonkChallenger},
        {684, MonkChallenger},
        {685, MonkChallenger},
        {686, MonkChallenger},
        {687, MonkChallenger},
        {688, MonkChallenger},
        {689, MonkChallenger},
        {690, MonkChallenger},
        {691, monk_master},

        /*
         * frost giant area 
         */

        {9401, fighter},
        {9403, magic_user},
        {9404, fighter},
        {9405, thief},
        {9406, fighter},
        {9407, fighter},
        {9408, fighter},
        {9409, fighter},
        {9410, fighter},
        {9411, fighter},
        {9412, fighter},
        {9413, fighter},
        {9414, fighter},
        {9415, fighter},
        {9416, fido},
        {9418, BreathWeapon},
        {9419, BreathWeapon},
        {9420, fighter},
        {9421, fighter},
        {9424, StormGiant},
        {9426, MonkChallenger},
        {9427, cleric},
        {9428, magic_user},
        {9429, fighter},
        {9430, regenerator},
        {9431, snake},
        {9432, magic_user},
        {9433, fighter},
        {9434, fighter},
        {9435, snake},
        {9436, fido},

        /*
         **   shire
         */
        {1000, magic_user},
        {1010, fighter},
        {1011, fighter},
        {1012, fighter},
        {1014, fighter},
        {1015, fighter},
        {1016, fighter},
        {1017, fighter},
        {1001, fighter},
        {1021, fighter},

        {1023, fighter},
        {1026, fighter},
        {1031, receptionist},
        {1032, fighter},

        {1163, magic_user},     /* Saruman */
        {1164, fighter},        /* orcs */
        {1171, thief},          /* thieves */

        {1701, monk},
        {1702, monk},
        {1703, monk},
        {1704, monk},
        {1705, monk},
        {1706, monk},
        {1707, monk},
        {1708, monk},
        {1709, monk},
        {1710, monk},
        {1711, monk},
        {1712, monk},
        {1713, monk},
        {1714, monk},
        {1715, monk},
        {1716, monk},
        {1717, monk},
        {1718, monk},

        {6001, real_rabbit},
        {6005, real_fox},
        /*
         * prydain 
         */
        {6601, PrydainGuard},
        {6602, PrydainGuard},
        {6604, fighter},
        {6605, PrydainGuard},
        {6606, PrydainGuard},
        {6608, fighter},
        {6613, fighter},
        {6619, PrydainGuard},
        {6620, PrydainGuard},
        {6621, fighter},
        {6622, fighter},
        {6614, PrydainGuard},
        {6609, BreathWeapon},
        {6642, BreathWeapon},
        {6640, jugglernaut},
        {6612, magic_user},
        {6615, magic_user},
        {6616, magic_user},
        {6624, magic_user},
        {6637, magic_user},
        {6634, fighter},
        {6635, BreathWeapon},
        {6625, Demon},
        {6638, StatTeller},

        /*
         * deadhame 
         */
        {24782, fighter},

        /*
         **  G1
         */
        {9200, fighter},
        {9201, fighter},
        {9202, fighter},
        {9203, fighter},
        {9204, fighter},
        {9206, fighter},
        {9207, fighter},
        {9211, fighter},
        {9212, fighter},
        {9216, fighter},
        {9213, CarrionCrawler},
        {9208, cleric},
        {9217, BreathWeapon},
        {9218, fighter},

        /*
         **  Bandits Temple
         */
        {2113, ghoul},
        {2115, ghost},
        {2116, ghost},
        {2111, cleric},
        {2112, cleric},
        {2117, druid_protector},

        /*
         * Astral plane 
         */
        {2715, astral_portal},
        {2716, astral_portal},
        {2717, astral_portal},
        {2718, astral_portal},
        {2719, astral_portal},
        {2720, astral_portal},
        {2721, astral_portal},
        {2722, astral_portal},
        {2723, astral_portal},
        {2724, astral_portal},
        {2725, astral_portal},
        {2726, astral_portal},
        {2727, astral_portal},
        {2728, astral_portal},
        {2729, astral_portal},
        {2730, astral_portal},
        {2731, astral_portal},
        {2732, astral_portal},
        {2733, astral_portal},
        {2734, astral_portal},
        {2735, astral_portal},
        {2736, astral_portal},
        {2737, astral_portal},
        {2738, astral_portal},  /* Ash's new winterfell pool */
        {2703, fighter},
        {2702, magic_user},
        {2701, fighter},
        /*
         **  Valley of the Mage
         */
        {21106, snake},
        {21107, RustMonster},
        {21108, wraith},
        {21111, web_slinger},
        {21112, trapper},
        {21114, troguard},
        {21121, trogcook},
        {21122, shaman},
        {21123, troguard},
        {21124, golgar},
        {21118, troguard},
        {21119, troguard},

        /*
         * I put the guy back ,msw 
         */
        {21130, Valik},

        {21135, regenerator},
        {21138, ghostsoldier},
        {21139, ghostsoldier},
        /*
         * { 21140, keystone}, { 21141, lattimore}, { 21142, guardian}, 
         */
        {21144, troguard},
        {21145, troguard},
        {21146, coldcaster},
        {21147, RustMonster},

        /*
         **  New Thalos
         */
        {3600, MageGuildMaster},
        {3601, ClericGuildMaster},
        {3602, WarriorGuildMaster},
        {3603, ThiefGuildMaster},
        {3604, receptionist},
        {3619, fighter},
        {3620, fighter},
        {3632, fighter},
        {3634, fighter},
        {3636, fighter},
        {3638, fighter},
        {3639, fighter},        /* caramon */
        {3641, cleric},         /* curley g. */
        {3640, magic_user},     /* raist */
        {3656, NewThalosGuildGuard},
        {3657, NewThalosGuildGuard},
        {3658, NewThalosGuildGuard},
        {3659, NewThalosGuildGuard},
        {3661, SultanGuard},    /* wandering */
        {3662, SultanGuard},    /* not */
        {3682, SultanGuard},    /* royal */
        {3670, BreathWeapon},   /* Cryohydra */
        {3674, BreathWeapon},   /* Behir */
        {3675, BreathWeapon},   /* Chimera */
        {3676, BreathWeapon},   /* Couatl */
        {3681, cleric},         /* High priest */
        {3689, NewThalosMayor}, /* Guess */
        {3644, fido},
        {3635, thief},
        /*
         **  Skexie
         */
        {15813, magic_user},
        {15815, magic_user},
        {15820, magic_user},
        {15821, vampire},
        {15844, cleric},
        {15847, fighter},
        {15831, fighter},
        {15832, fighter},
        {15822, fighter},
        {15819, fighter},
        {15805, fighter},
        /*
         **  Challenge
         */
        {15858, BreathWeapon},
        {15861, magic_user},
        {15862, magic_user},
        {15863, fighter},
        {15864, sisyphus},
        {15877, magic_user},
        {15868, snake},
        {15866, magic_user},
        {15810, magic_user},
        {15880, fighter},
        {15879, BreathWeapon},
        {15873, magic_user},
        {15871, magic_user},
        {15852, fighter},
        {15875, cleric},
        {15869, magic_user},

        /*
         **  abyss
         */
        {25000, magic_user},    /* Demi-lich */
        {25001, Keftab},
        {25009, BreathWeapon},  /* hydra */
        {25002, vampire},       /* Crimson */
        {25003, StormGiant},    /* MistDaemon */
        {25006, StormGiant},    /* Storm giant */
        {25014, StormGiant},    /* DeathKnight */
        {25009, BreathWeapon},  /* hydra */
        {25017, AbyssGateKeeper},       /* Abyss Gate Keeper */
        {25013, fighter},       /* kalas */
        {25008, magic_user},    /* efreeti */
        {25025, acid_monster},
        {25026, acid_monster},
        {25034, fighter},       /* marilith */
        {25035, magic_user},    /* balor */

        /*
         **  Paladin's guild
         */
        {3028, PaladinGuildGuard},
        {21363, PaladinGuildmaster},

        /*
         **  Abyss Fire Giants
         */
        {25500, fighter},
        {25501, fighter},
        {25502, fighter},
        {25505, fighter},
        {25504, BreathWeapon},
        {25503, cleric},

        /*
         **  Temple Labrynth
         */

        {10900, temple_labrynth_liar},
        {10901, temple_labrynth_liar},
        {10902, temple_labrynth_sentry},

        /*
         **  Gypsy Village
         */

        {16106, fido},
        {16107, CaravanGuildGuard},
        {16108, CaravanGuildGuard},
        {16109, CaravanGuildGuard},
        {16110, CaravanGuildGuard},
        {16111, WarriorGuildMaster},
        {16112, MageGuildMaster},
        {16113, ThiefGuildMaster},
        {16114, ClericGuildMaster},
        {16122, receptionist},
        {16105, StatTeller},

        /*
         **  Draagdim
         */

        {2500, PrisonGuard},    /* jailer */
        /*
         **  mordilnia
         */
        {18200, magic_user},
        {18205, receptionist},
        {18206, MageGuildMaster},
        {18207, ClericGuildMaster},
        {18208, ThiefGuildMaster},
        {18209, WarriorGuildMaster},
        {18210, MordGuildGuard},        /* 18266 3 */
        {18211, MordGuildGuard},        /* 18276 1 */
        {18212, MordGuildGuard},        /* 18272 0 */
        {18213, MordGuildGuard},        /* 18256 2 */
        {18215, MordGuard},
        {18216, janitor},
        {18217, fido},
        {18218, fighter},
        {18221, fighter},
        {18222, MordGuard},
        {18223, MordGuard},

        /*
         **  Graecia:
         */

        {13706, fighter},
        {13709, fighter},
        {13711, fighter},
        {13714, fighter},
        {13721, fighter},
        {13722, fighter},
        {13726, LightningBreather},
        {13732, snake},
        {13762, fighter},
        {13764, fighter},
        {13765, acid_monster},
        {13766, fighter},
        {13769, fighter},
        {13771, fighter},
        {13775, fighter},

        {13779, magic_user},
        {13784, magic_user},
        {13785, magic_user},
        {13787, magic_user},
        {13789, magic_user},
        {13791, magic_user},
        {13793, magic_user},
        {13795, magic_user},
        {13797, magic_user},
        {13843, fighter},

        /*
         **  Eastern Path
         */

        {16001, fighter},
        {16006, fighter},
        {16007, fighter},
        {16008, fighter},
        {16009, fighter},
        {16016, fighter},
        {16017, fighter},
        {16020, snake},
        {16021, cleric},
        {16022, fighter},
        {16023, thief},
        {16027, fighter},
        {16033, fighter},
        {16034, fighter},
        {16035, fighter},
        {16036, magic_user},
        {16043, fighter},
        {16050, fighter},
        {16052, fighter},
        {16048, fighter},
        {16056, cleric},
        {16057, magic_user},
        {16037, DwarvenMiners},
        {16014, magic_user},
        {16039, Tyrannosaurus_swallower},

        /*
         **  undercaves.. level 1
         */
        {16201, magic_user},
        {16204, magic_user},
        {16205, magic_user},
        {16206, magic_user},
        {16210, fighter},
        {16211, magic_user},
        {16213, acid_monster},
        {16214, fighter},
        {16215, fighter},
        {16216, cleric},
        {16217, magic_user},
        {16219, death_knight},

        /*
         ** Sauria
         */
        {21803, Tyrannosaurus_swallower},
        {21810, Tyrannosaurus_swallower},

        /*
         **  Bay Isle
         */
        {16610, Demon},
        {16620, BreathWeapon},
        {16640, cleric},
        {16650, cleric},

#if 0
        /*
         **  King's Mountain
         */
        {16700, BreathWeapon},
        {16702, shadow},
        {16703, magic_user},
        {16709, vampire},
        {16710, Devil},
        {16711, Devil},
        {16712, Devil},
        {16713, ghoul},
        {16714, ghoul},
        {16715, wraith},
        {16717, fighter},
        {16720, Devil},
        {16721, Devil},
        {16724, Devil},
        {16725, magic_user},
        {16726, cleric},
        {16727, Devil},
        {16728, Devil},
        {16730, Devil},
        {16731, Devil},
        {16732, Demon},
        {16733, Demon},
        {16734, Demon},
        {16735, Demon},
        {16736, cleric},
        {16738, BreathWeapon},
#endif

        /*
         **  Sewer Rats
         */
        {7002, attack_rats},
        {2531, DragonHunterLeader},
        {3063, HuntingMercenary},

        /*
         **  Mages Tower
         */
        {1500, shadow},
        {1504, magic_user},
        {1506, magic_user},
        {1507, magic_user},
        {1508, magic_user},
        {1510, magic_user},
        {1514, magic_user},
        {1515, magic_user},
        {1516, magic_user},
        {1517, magic_user},
        {1518, magic_user},
        {1520, magic_user},
        {1521, magic_user},
        {1522, magic_user},
        {1523, magic_user},
        {1524, magic_user},
        {1525, magic_user},
        {1526, magic_user},
        {1527, magic_user},
        {1528, magic_user},
        {1529, magic_user},
        {1530, magic_user},
        {1531, magic_user},
        {1532, magic_user},
        {1533, magic_user},
        {1534, magic_user},
        {1537, magic_user},
        {1538, magic_user},
        {1540, magic_user},
        {1541, magic_user},
        {1548, magic_user},
        {1549, magic_user},
        {1552, magic_user},
        {1553, magic_user},
        {1554, magic_user},
        {1556, magic_user},
        {1557, magic_user},
        {1559, magic_user},
        {1560, magic_user},
        {1562, magic_user},
        {1564, magic_user},
        {1565, magic_user},
        /*
         ** Wert's stuff
         */

        {12000, thief},
        {12001, thief},
        {12002, thief},
        {12003, thief},
        {12004, thief},
        {12006, thief},
        {12007, fighter},
        {12008, fighter},
        {12009, thief},
        {12010, FireBreather},
        {12011, FireBreather},
        /*
         **     Alun's stuff
         */

        {6241, TreeThrowerMob},
        /*
         ** Celestian's Specials
         */
        {1943, mage_specialist_guildmaster},    /* specialist gm */
        {1901, PostMaster},     /* PostMaster for mail/send/receive/ */
        {1900, archer},

        {1904, fighter},        /* chief */
        {1905, fighter},        /* villagers */
        {1907, cleric},         /* doctor */
        {1908, village_princess},
        {1921, fighter},        /* lizardmen */
        {1922, lizardman_shaman},
        {1944, fighter},        /* low priest */
        {1945, cleric},         /* mid priest */
        {1946, cleric},         /* high priest */
        {1947, snake_avt},      /* snake god avatar */
        {1948, snake_guardians},        /* snake guard */
        {1949, snake},          /* pet snake */
        {1950, virgin_sac},     /* virgin sac person */
        {1951, snake},          /* flying snake */
        {1953, fighter},        /* jungle giant */
        {1953, magic_user},     /* second god_avt */
        {2012, fighter},        /* fresh water hag */
        {21366, PsiGuildmaster},
        {21367, RangerGuildmaster},

        /*
         ***            Spider Haunt, Celestians
         */
        {9601, goblin_sentry},
        {9602, goblin_sentry},
        {9605, fighter},        /* sleepers */
        {9604, fighter},        /* general goblin */
        {9606, fighter},        /* goblin king */
        {9608, fighter},        /* half-giant worker */
        {9607, fighter},        /* forger goblin */
        {9609, fighter},        /* female cook */
        {9611, snake},          /* bitch of a spider in the woods */

        /*
         * Celestian's Menzborranzan
         */

        {9700, Drow},           /* guard */
        {9701, Drow},           /* wanderer */
        {9702, Drow},           /* high priest */
        {9703, Drow},           /* wandering female */
        {9704, Drow},           /* briza */
        {9705, Drow},           /* matron */
        {9706, Drow},           /* rizzen */
        {9707, Drow},           /* vierna */
        {9708, Drow},           /* maya */
        {9709, Drow},           /* dinin */
        {9710, Drow},           /* zaknafien */
        {9711, Drow},           /* drizzit */
        {9712, Drow},           /* wandering mage */
        {9713, Drow},           /* worker */
        {9733, Drow},           /* 2nd malice?? ah well, let's get rid of
                                 * spam here, and assign her. */

        /*
         * Blackmouths Tower 
         */

        {24007, magic_user},
        {24009, magic_user},
        {24010, magic_user},
        {16738, FireBreather},
        {5209, raven_iron_golem},
        {24003, magic_user},
        {24004, magic_user},
        {24005, magic_user},
        {24006, magic_user},
        {24000, snake},

        {25012, fighter_mage},

        /*
         ** Celestian's Ravenloft Area
         */

        {30000, strahd_zombie},
        {30113, strahd_vampire},
        {30004, banshee},       /* BANSHEE */
        {30005, fighter},       /* guardian */
        {30006, fighter},       /* accountant lief */
        {30007, fighter},       /* werewolf */
        {30100, magic_user},    /* witch one */
        {30101, magic_user},    /* witch two */
        {30111, magic_user},    /* witch 3 */
        {30102, vampire},       /* helga */
        {30103, wraith},        /* wraith */
        {30105, mad_gertruda},  /* gertruda */
        {30107, raven_iron_golem},      /* iron golem */
        {30108, mad_cyrus},     /* cyrus belview */
        {30109, fighter},       /* zombie normal */
        {30106, wraith},        /* familiar cats */
        {30112, Demon},         /* shadow demon */
        {30114, wraith},        /* elf corpse */
        {30115, ghoul},         /* roo corpse */
        {30116, vampire},       /* spectre */
        {30117, ghost},         /* ghost */
        {30118, shadow},        /* lost souls */

        /*
         ***     Ator's Mobs
         */

        {3402, ghoul},
        {3404, wraith},
        {3405, fighter},
        {3406, fighter},
        {3407, fighter},
        {3408, ghost},
        {3409, ghoul},
        {3410, FireBreather},
        {3411, lich_church},
        {3412, medusa},
        {3413, fighter},
        {3414, magic_user},
        {3415, FireBreather},
        {3416, vampire},
        {3417, regenerator},
        {3418, fighter},
        {3424, Slavalis},

        /*
         **  Forest of Rhowyn
         */

        {13901, ThrowerMob},

        /*
         **  Quikland
         */
        {6202, fighter},
        {6204, magic_user},
        {6206, fighter},
        {6207, fighter},
        {6208, fighter},
#if 0
        /*
         * Dwarf Village, commented out after revamp
         */
         {6500, fighter},
         {6501, fighter},
         {6502, wraith},
         {6506, fighter},
         {6507, fighter},
         {6508, fighter},
         {6514, fighter},
         {6516, fighter},
         {6516, snake},
         {6517, snake},
#endif         
        /*
         **  Lycanthropia
         */
        {16901, fighter},
        {16902, fighter},
        {16903, fighter},
        {16904, fighter},
        {16905, fighter},
        {16906, fighter},
        {16907, magic_user},
        {16908, fighter},
        {16910, fighter},
        {16911, fighter},

        /*
         **  Main City
         */

        {3000, magic_user},
        {3060, MidgaardCityguard},
        {3067, MidgaardCityguard},
        {3061, janitor},
        {3062, fido},
        {3066, fido},
        {3005, receptionist},
        {3020, MageGuildMaster},
        {3021, ClericGuildMaster},
        {3022, ThiefGuildMaster},
        {3023, WarriorGuildMaster},
        {3051, SorcGuildMaster},
        {3052, NecromancerGuildMaster},

        {3007, sailor},         /* Sailor */
        {3024, guild_guard},
        {3025, guild_guard},
        {3026, guild_guard},
        {3027, guild_guard},
        {3070, RepairGuy},
        {3071, RepairGuy},
        {3069, MidgaardCityguard},      /* post guard */
        {3068, ninja_master},
        {3073, loremaster},
        {3074, hunter},
        {3076, archer_instructor},
        {3077, barbarian_guildmaster},

        /*
         **  Lower city
         */
        {99, QuestorGOD},
#if 0        
        { 3047, DogCatcher },
#endif        
        {3143, mayor},
        {7009, MidgaardCityguard},

        /*
         **  MORIA
         */
        {4000, snake},
        {4001, snake},
        {4053, snake},

        {4103, thief},
        {4100, magic_user},
        {4101, regenerator},
        {4102, snake},

        /*
         **  Pyramid
         */

        {5308, RustMonster},
        {5303, vampire},

        /*
         **  Arctica
         */
        {6800, fighter},
        {6803, fighter},
        {6801, BreathWeapon},
        {6802, BreathWeapon},
        {6815, magic_user},
        {6821, snake},
        {6824, BreathWeapon},
        {6825, thief},

        /*
         ** SEWERS
         */
        {7009, fighter},
        {7006, snake},
        {7008, snake},
        {7042, magic_user},     /* naga */
        {7040, BreathWeapon},   /* Red */
        {7041, magic_user},     /* sea hag */
        {7200, magic_user},     /* mindflayer */
        {7201, magic_user},     /* senior */
        {7202, magic_user},     /* junior */

        /*
         ** FOREST
         */

        {6111, magic_user},     /* tree */
        {6113, snake},
        {6114, snake},
        {6112, BreathWeapon},   /* green */
        {6910, magic_user},

        /*
         **  Great Eastern Desert
         */
        {5000, thief},          /* rag. dervish */
        {5002, snake},          /* coral snake */
        {5003, snake},          /* scorpion */
        {5004, acid_monster},   /* purple worm */
        {5014, cleric},         /* myconoid */
        {5005, BreathWeapon},   /* brass */

        /*
         **  Drow (edition 1)
         */
        {5010, magic_user},     /* dracolich */
        {5104, cleric},
        {5103, magic_user},     /* drow mage */
        {5107, cleric},         /* drow mat. mot */
        {5108, magic_user},     /* drow mat. mot */
        {5109, cleric},         /* yochlol */
        {5101, Drow},
        {5102, Drow},
        {5105, Drow},
        {5106, Drow},

        /*
         **   Thalos
         */
        {5200, Beholder},       /* beholder */

        /*
         **  Zoo
         */
        {9021, snake},          /* Gila Monster */

        /*
         **  Castle Python
         */
        {11001, fighter},       /* lord python */
        {11002, fighter},
        {11004, fighter},
        {11005, fighter},
        {11006, fighter},
        {11007, fighter},
        {11016, receptionist},
        {11017, NudgeNudge},
        {11039, magic_user},
        {11026, fighter},

        /*
         **  miscellaneous
         */
        {9061, vampire},        /* vampiress */

        /*
         **  White Plume Mountain
         */

        {17004, magic_user},    /* gnyosphinx */
        {17017, magic_user},    /* ogre magi */
        {17014, ghoul},         /* ghoul */
        {17009, geyser},        /* geyser */
        {17011, vampire},       /* vampire Amelia */
        {17002, wraith},        /* wight */
        {17005, shadow},        /* shadow */
        {17010, green_slime},   /* green slime */

        /*
         **  Arachnos
         */

        {20001, snake},         /* Young (large) spider */
        {20003, snake},         /* wolf (giant) spider */
        {20005, snake},         /* queen wasp */
        {20006, snake},         /* drone spider */
        {20010, snake},         /* bird spider */
        {20009, magic_user},    /* quasit */
        {20014, magic_user},    /* Arachnos */
        {20015, magic_user},    /* Ki Rin */

        {20002, BreathWeapon},  /* Yevaud */
        {20017, BreathWeapon},  /* Elder */
        {20016, BreathWeapon},  /* Baby */
        {20012, fighter},       /* donjonkeeper */

        /*
         **  Sunsor's elf area
         */
        {22602, archer},        /* archer warrior */
        {22605, timnus},        /* timnus */
        {22604, baby_bear},     /* mother bear */
        {22624, baby_bear},     /* baby bears. */

#if 0
        /*
         **   The Darklands
         */

        {24050, cleric},
        {24052, magic_user2},
        {24053, magic_user2},
        {24054, magic_user2},
        {24055, magic_user2},
        {24056, magic_user2},
        {24057, magic_user2},
        {24058, magic_user2},
        {24059, magic_user2},
#endif

#if 0

        /*
         **   Abbarach
         */

        {27001, magic_user},
        {27002, magic_user},
        {27003, magic_user},
        {27004, magic_user},
        {27005, magic_user},
        {27006, Tytan},
        {27007, replicant},
        {27016, AbbarachDragon},
        {27014, magic_user},
        {27017, magic_user},
        {27018, magic_user},
        {27019, magic_user},
        {27025, Samah},
#endif

        /*
         * roo/Land down under 
         */

        {27401, fighter},
        {27403, fighter},
        {27407, fighter},
        {27408, fighter},
        {27409, fighter},
        {27411, fighter},
        {27415, fighter},
        {27416, fighter},
        {27417, fighter},
        {27418, fighter},
        {27419, fighter},
        {27420, fighter},
        {27404, magic_user},
        {27405, magic_user},
        {27422, magic_user},
        {27413, cleric},
        {27414, cleric},
        {27429, AGGRESSIVE},
        {27430, AGGRESSIVE},

        {7526, winger},
        {7522, magic_user},
        {7531, magic_user},
        {7510, fighter},
        {7514, fighter},
        {7515, fighter},
        {7516, fighter},
        {7527, fighter},
        {7528, fighter},
        {7530, fighter},

        /*
         * Menzoberanza Zone 
         */
        {9727, BreathWeapon},

        /*
         * Clan hall stuff 
         */
        {22704, receptionist},
        {22720, receptionist},
        {22727, ninja_master},
        {22732, PostMaster},    /* Order */
        {22731, receptionist},  /* Order */

        /*
         * Thikahnus's zone 
         */
        {37203, FrostBreather}, /* Iceberg */
        {37208, shadow},        /* Ministraal */
        {37209, vampire},       /* Joshua von Richten */
        {37210, ghost},         /* spectre guard */
        {37227, shadow},        /* spirit priest */
        {37228, ghoul},         /* Spirit high priest */
        {37229, fido},          /* Stray dog */
        {37230, ghost},         /* Spectre roaming */
        /*
         * The Realms of Delbrandor 
         */
        {28038, receptionist},  /* Sealissa */
        {28050, ABShout},       /* Avatar of Blibdoolpoolp */
        {28042, AvatarPosereisn},       /* Avatar of Posereisn */
        {28011, ghost},         /* Vampric Mist */
        {28022, BreathWeapon},  /* Coral Dragon */

        /*
         * sundhaven (Banon) 
         */
        {11400, sund_earl},     /* Earl of Sundhaven */
        {11401, MidgaardCityguard},
        {11464, PostMaster},
        {11456, guild_guard},
        {11455, guild_guard},
        {11458, guild_guard},
        {11457, guild_guard},
        {11416, ClericGuildMaster},
        {11419, MageGuildMaster},
        {11417, ThiefGuildMaster},
        {11418, WarriorGuildMaster},
        {11459, MidgaardCityguard},
        {11460, MidgaardCityguard},
        {11406, fido},          /* Smoke rat */
        {11407, thief},
        {11402, hangman},
        {11466, stu},
        {11448, butcher},
        {11461, blinder},
        {11437, silktrader},
        {11415, idiot},
        {11453, athos},

        /*
         * Cthol (Xenon) 
         */
        {40343, FireBreather},  /* White Cthol Dragon */

        {45108, DispellerIncMob},       /* Heximals Dispeller/Incernary
                                         * cloud proc */

        /*
         * Beginning of Thunder Mountain Keep
         */

        {45401, tmk_guard},
        {45402, tmk_guard},
        {45406, braxis_swamp_dragon},
        {45409, nadia},
        {45410, mime_jerry},
        {45413, zork},
        {45414, LightningBreather},     /* Mairden bronze dragon */
        {45415, FrostBreather}, /* Gavmorak silver dragon */
        {45416, FireBreather},  /* Tarius gold dragon */
        {45417, elamin},
        {45421, FrostBreather}, /* merick white dragon */
        {45422, VaporBreather}, /* Viska mist dragon */
        {45423, DehydBreather}, /* Korack topaz dragon */
        {45424, SoundBreather}, /* Katsulas emerald dragon */
        {45425, GasBreather},   /* Skaer deep dragon */
        {45426, ShardBreather}, /* Barack crystal dragon */
        {45427, SleepBreather}, /* Nevarock brass dragon */
        {45429, AcidBreather},  /* Rhyder copper dragon */
        {45430, SoundBreather}, /* Irie sapphire dragon */
        {45431, LightBreather}, /* Symon mercury dragon */
        {45432, AcidBreather},  /* Ragnorak black dragon */
        {45433, ShardBreather}, /* Sleigh amethyst dragon */
        {45434, FrostBreather}, /* Tegellen cloud dragon */
        {45435, LightningBreather},     /* Haddox blue dragon */
        {45437, FireBreather},  /* Tynan red dragon */
        {45439, DarkBreather},  /* Rank shadow dragon */
        {45440, starving_man},
        {45443, goblin_chuirgeon},
        {45463, cronus_pool},
        {45600, FireBreather},  /* Cronus gold dragon */
        {45564, cleric_specialist_guildmaster},

        /*
         * Start Leaves of Silver 
         */
        {47951, Jessep},        /* Jessep */
        {47975, Tysha},         /* Tysha */
        {47905, Vaelhar},       /* Vaelhar */

        /*
         * Ash's New Zones 
         */
        {31811, MageGuildMaster},       /* Maeister gm guildmaster mage */
        {31825, PsiGuildmaster},        /* Psikill psionist gm guildmaster 
                                         */
        {31827, ThiefGuildMaster},      /* Littlefinger thief gm
                                         * guildmaster */
        {31814, ClericGuildMaster},     /* Cirelic cleric gm guildmaster */
        {31815, PaladinGuildmaster},    /* Justiciar paladin gm
                                         * guildmaster */
        {31806, barbarian_guildmaster}, /* Cabal guildmaster gm barbarian */
        {31807, WarriorGuildMaster},    /* Raegar gm guildmaster warrior */
        {31900, DruidGuildMaster},      /* Winter druid guildmaster */
        {31940, monk_master},   /* Hands monk guildmaster */
        {31801, receptionist},  /* winterfell receptionist */
        {31803, PostMaster},    /* winterfell scribe mage */
        {33038, RangerGuildmaster},     /* Ranger?? */

        {31877, QPSalesman},    /* Questor Demigod (Quest Point Shop...) */
        {31886, RepairGuy},     /* Ned armorer (repair like super repair) */
        {32001, Etheral_post},  /* (in room 32000, to Winterfell/room
                                 * 31804) */
        {32004, Etheral_post},  /* (in room 32004, to High Seas/room
                                 * 33180) */
        {32009, Etheral_post},  /* (in room 32009, to Sewers/room 32300) */
        {32011, Etheral_post},  /* in room 32011, to Great Northern
                                 * Keep/room 32600) */
        {32030, Etheral_post},  /* in room 32030, to Desolate Caves/room
                                 * 32800) */
        {32024, Etheral_post},  /* ethereal post (in room 32024, to
                                 * Karsinya/room 3014) */
        {32032, Etheral_post},  /* ethereal post (in room 32032, to
                                 * Abyss/room 25002) */

        {33180, Etheral_post},  /* 2270 high seas ethereal post */
        {31804, Etheral_post},  /* 2264 winterfell ethereal post */
        {32600, Etheral_post},  /* 2317 keep ethereal post */
        {32300, Etheral_post},  /* 2339 sewers ethereal post */
        {32801, Etheral_post},  /* 2340 desolate caves ethereal post */
        {32803, Etheral_post},  /* 2342 Karsinya Ethereal Post */
        {32802, Etheral_post},  /* 2341 Abyss Ethereal Post */

        {31818, StatTeller},    /* Damage teller */
        {31931, citizen},       /* for guards and clerics shouting for
                                 * help */
        {31913, citizen},       /* for guards and clerics shouting for
                                 * help */
#if 0        
        {31886, RepairGuy },
#endif        
        {33185, board_ship},    /* note.. Not sure which corsair does waht 
                                 */
        {32034, CorsairPush},
        {33188, Tyrannosaurus_swallower},
        {23200, FightingGuildMaster},

        /*
         * Tmeple of Sin 
         */
        {51826, sinpool},       /* Lennyas pool proc */
        {51843, pridemirror},   /* Lennyas mirror proc */
        {51803, sin_spawner},
        {51821, trinketlooter},
        {51822, trinketlooter},
        {51823, trinketlooter},
        {51824, trinketlooter},
        {51805, lust_sinner},
        {51806, lust_sinner},
        {51812, Tyrannosaurus_swallower},       /* Hambre, Guardian of
                                                 * Gluttony */

        /*
         * mermaid lagoon 
         */
        {3907, mermaid},        /* Melodia Merqueen */
        {3953, mermaid},        /* Sirens */
        {3959, mermaid},        /* roaming mermaids */

        /*
         * City State of Tarantis 
         */
        {49600, nightwalker},
        {49635, master_smith},
        {49702, portal_regulator},      /* mob to stick the portal proc on 
                                         */
        {49611, receptionist},

        /*
         * Heximal's new zone 
         */
        {50998, Deshima},
#if 0
        /* Myron  - removed by Beirdo*/
        {23201, TrainingGuild},
        {23202, WeaponsMaster},
#endif

        /*
         * King's Grove 
         */
        {52850, DruidGuildMaster},
        {52853, gnome_collector},

        /* Talesian mob procedures */
        {37802, sageactions},
        {37803, guardianextraction},
        {37804, ghastsmell},
        {37805, ghoultouch},
        {37806, shadowtouch},
        {37807, moldexplosion},
        {37809, boneshardbreather},
        {37810, mistgolemtrap},
        {37811, confusionmob},

        {50806, MageGuildMaster},
        {50808, PsiGuildmaster},
        {50805, ThiefGuildMaster},
        {50807, ClericGuildMaster},
        {50801, PaladinGuildmaster},
        {50804, barbarian_guildmaster},
        {50803, WarriorGuildMaster},
        {50810, DruidGuildMaster},
        {50809, monk_master},
        {50890, receptionist},
        {50802, RangerGuildmaster},

        /*
         * Sentinel's The Estate 
         */
        {51300, janaurius},

        {-1, NULL},
    };

    int             i,
                    rnum;
    char            buf[MAX_STRING_LENGTH];

    for (i = 0; specials[i].vnum >= 0; i++) {
        rnum = real_mobile(specials[i].vnum);
        if (rnum < 0) {
            sprintf(buf, "mobile_assign: Mobile %d not found in database.",
                    specials[i].vnum);
            Log(buf);
        } else {
            mob_index[rnum].func = specials[i].proc;
        }
    }

    boot_the_shops();
    assign_the_shopkeepers();
}

/*
 * assign special procedures to objects 
 */
void assign_objects()
{
    static struct special_proc_entry specials[] = {
        {15, SlotMachine},
        {30, scraps},
        {23, ships_helm},   
        {31, portal},
        {3097, board},
        {3098, board},
        {3099, board},
        {40100, board},
        {25015, BerserkerItem},
        {22730, board},

        {21122, nodrop},
        {21130, soap},
        {27038, BerserkerItem},

#if 0    /*EGO*/
        {11447, marbles},
        {11509, marbles},
        {35000, EvilBlade},
        {35001, GoodBlade},
        {35002, NeutralBlade},
#endif

        /*
         * Thunder Mountain Keep Objects
         */
        {45470, thunder_sceptre_one},
        {45481, thunder_sceptre_two},
        {45492, thunder_black_pill},
        {45493, thunder_blue_pill},
        {45504, grayswandir},

        /*
         * Temple of Sin 
         */
        {51831, altarofsin},
        {51827, applepie},
        {51832, trinketcount},
        /*
         * King's Grove 
         */
        {27, qp_potion},
        
        /* Talesian object procedures */
        {37821, mirrorofopposition},

        {51152, chestproc},     /* royal rumble proc */

        {-1, NULL},
    };

    int             i,
                    rnum;
    char            buf[MAX_STRING_LENGTH];

    for (i = 0; specials[i].vnum >= 0; i++) {
        rnum = real_object(specials[i].vnum);
        if (rnum < 0) {
            sprintf(buf, "object_assign: Object %d not found in database.",
                    specials[i].vnum);
            Log(buf);
        } else {
            obj_index[rnum].func = specials[i].proc;
        }
    }
}

/*
 * assign special procedures to rooms 
 */
void assign_rooms()
{

    static struct special_proc_entry specials[] = {
        {99, Donation},
        {13510, Donation},
        {500, druid_challenge_prep_room},
        {501, druid_challenge_room},
        {550, monk_challenge_prep_room},
        {551, monk_challenge_room},
        {3030, dump},
        {13547, dump},
        {3054, pray_for_items},

        {2188, Magic_Fountain},
        {2189, Magic_Fountain},
        {3005, Fountain},       /* Meeting SQ */
        {13518, Fountain},
        {11014, Fountain},
        {5234, Fountain},
        {3120, Fountain},
        {1833, Fountain},
        {3141, Fountain},
        {13406, Fountain},
        {22642, Fountain},
        {22644, Fountain},
        {22646, Fountain},
        {22648, Fountain},
        {26010, Fountain},
        {26109, Fountain},      /* Stormcloak fountain */
        {22739, Fountain},      /* {O}rder fountain */
        {2000, bank},
        {13521, bank},
        {18224, bank},
        {22738, bank},          /* Order Bank */
        /*
         * sundhaven bank 
         */
        {11558, bank},
        {3422, ChurchBell},
        {37334, Fountain},
        {39900, arena_prep_room},
        {40053, arena_fireball_trap},
        {39954, arena_arrow_dispel_trap},
        {40017, arena_dispel_trap},

        /*
         * Temple of Sin 
         */
        {51827, sinbarrel},
        {51936, pride_remover_one},
        {51935, pride_remover_two},
        {51934, pride_remover_three},
        {51933, pride_remover_four},
        {51932, pride_remover_five},
        {51931, pride_remover_six},
        {51930, pride_remover_seven},
        {51836, pride_disabler},
        {51835, pride_disabler},
        {51837, greed_disabler},
        {51838, greed_disabler},
        {51839, greed_disabler},
        {51840, greed_disabler},
        {51841, greed_disabler},
        {51842, greed_disabler},
        {51843, greed_disabler},
        {51844, greed_disabler},
        {51845, greed_disabler},
        {51846, greed_disabler},
        {51847, greed_disabler},
        {51848, greed_disabler},
        {51849, greed_disabler},
        {51850, greed_disabler},
        {51851, greed_disabler},
        {51852, greed_disabler},
        {51853, greed_disabler},
        {51854, greed_disabler},
        {51855, greed_disabler},
        {51856, greed_disabler},
        {51857, greed_disabler},
        {51858, greed_disabler},
        {51859, greed_disabler},
        {51860, greed_disabler},
        {51861, greed_disabler},
        {51862, greed_disabler},
        {51863, greed_disabler},
        {51864, greed_disabler},
        {51865, greed_disabler},
        {51866, greed_disabler},
        {51867, greed_disabler},
        {51868, greed_disabler},
        {51869, greed_disabler},
        {51870, greed_disabler},
        {51871, greed_disabler},
        {51872, greed_disabler},
        {51873, greed_disabler},
        {51874, greed_disabler},
        {51875, greed_disabler},
        {51876, greed_disabler},
        {51877, greed_disabler},
        {51878, greed_disabler},
        {51879, greed_disabler},
        {51914, lag_room},
        {51915, lag_room},
        {51916, lag_room},
        {51917, lag_room},
        {51918, lag_room},
        {51919, lag_room},
        {51920, lag_room},
        {51921, lag_room},
        {51922, lag_room},
        {51923, lag_room},
        {51924, lag_room},
        {51925, lag_room},

        /*
         * Heximal's riddles 
         */
        {50962, riddle_exit},
        {50963, riddle_exit},
        {50964, riddle_exit},

        /*
         * The King's Grove 
         */
        {52857, gnome_home},
        {52866, climb_room},
        {52868, pick_acorns},
        {52877, legendfountain},
        {52881, pick_berries},

        /*
         * Sentinel's Zone 
         */
        {51195, cog_room},
        {51204, cog_room},
        {51231, cog_room},
        {51258, cog_room},
        {51300, cog_room},

        /*
         * Citystate of Tarantis 
         */
        {49593, monk_challenge_prep_room},
        {49599, druid_challenge_prep_room},

        /*
         * Rocky's Zone 
         */
        {17429, close_doors},
        {17430, close_doors},
        {17431, close_doors},
        {17432, close_doors},

        /*
         * Cthol (Xenon) 
         */
        {40233, dispel_room},
#if 0        
        {40285, fiery_alley},
        {40287, fiery_alley},
#endif
        /*
         * Thunder Mountain Keep 
         */
        {45450, Thunder_Fountain},
        {45409, rope_room},
        {46378, bahamut_home},

        /*
         * Winterfell 
         */
        {31943, pet_shops},
        {31858, bank},          /* Giamina bank teller (room# 31858 -
                                 * bank) */
#if 0        
        /* Ash's Monk and Druid challenge rooms  */
         { 500,druid_challenge_prep_room}, 
         { 501, druid_challenge_room}, 
         { 550, monk_challenge_prep_room}, 
         { 551, monk_challenge_room}, 
#endif
        {44129, Magic_Pool},    /* in proc_assign and add to protos */
        {44114, Read_Room},     /* in proc_assign and add to protos.. */
        {51151, preperationproc},       /* Royal rumble proc. */

        {53025, Fountain},      /* Luna's fountain */

        /* Talesian room procedures */
        {37840, traproom},
        {37823, guardianroom},
        {37857, trapjawsroom},
        {37839, ventroom},

        {11353, knockproc},
        {-1, NULL},
    };
    int             i;
    struct room_data *rp;

    for (i = 0; specials[i].vnum >= 0; i++) {
        rp = real_roomp(specials[i].vnum);
        if (rp == NULL) {
            Log("assign_rooms: unknown room");
        } else
            rp->funct = specials[i].proc;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
