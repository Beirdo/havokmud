/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "structs.h"
#include "limits.h"
#include "trap.h"
#include "race.h"
#include "spells.h"

const char     *spell_wear_off_msg[] = {
    "RESERVED DB.C",
    "You feel less protected.",
    "!Teleport!",
    "You feel less righteous.",
    "You feel a cloak of blindness dissolve.",
    "!Burning Hands!",
    "!Call Lightning",
    "You feel more in control of your destiny",
    "You feel stronger now",
    "!Clone!",
    "!Color Spray!",            /* 10 */
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "You feel better.",
    "You sense the red in your vision disappear.",
    "The detect invisible wears off.",
    "The detect magic wears off.",      /* 20 */
    "The detect poison wears off.",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "You feel exposed.",
    "!Lightning Bolt!",         /* 30 */
    "!Locate object!",
    "!Magic Missile!",
    "You feel less sick.",
    "You feel less protected from evil denizens.",
    "!Remove Curse!",
    "The white aura around your body fades.",
    "!Shocking Grasp!",
    "You feel less tired.",
    "You don't feel as strong.",
    "!Summon!",
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "You feel less aware of your suroundings.",
    "",                         /* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "!Identify!",               /* 53 */
    "You feel disoriented as you lose your infravision.",
    "!cause light!",
    "!cause crit!",
    "!flamestrike!",
    "!dispel good!",
    "You feel somewhat stronger now...",
    "!dispel magic!",
    "!knock!",
    "!know alignment!",
    "!animate dead!",
    "You feel freedom of movement.",
    "!remove paralysis!",
    "!fear!",
    "!acid blast!",
    "You feel a tightness at your throat. ",
    "You feel heavier now, your flying ability is gone.",
    "spell70, please report.",  /* 70 */
    "spell71, please report.",
    "spell72, please report",
    "Your magic shield fades away happily.",
    "spell74, please report",
    "spell75, please report.",
    "spell76, please report.",
    "spell77, please report.",
    "spell78, please report.",
    "spell79, please report.",
    "spell80, please report.",  /* 80 */
    "The red glow around your body fades",
    "spell82, please report.",
    "spell83, please report.",
    "spell84, please report.",
    "spell85, please report.",
    "spell86, please report.",
    "spell87, please report.",
    "spell88, please report.",
    "spell89, please report.",
    "spell90, please report.",  /* 90 */
    "spell91, please report.",
    "Your skin returns to normal.",
    "spell93, please report.",
    "Your clarity of vision dissapears",
    "spell95, please report",
    "The pink glow around your body fades.",    /* 96 */
    "spell 97, please report.",
    "spell 98 please report.",
    "spell 99 please report.",
    "spell 100 please report.",
    "spell 101 please report.",
    "spell 102 please report.",
    "spell 103 please report.",
    "You feel heavier, your flying spell is leaving you.",
    "spell 105 please report.",
    "You feel freedom of movement.",
    "You lose your tracking ability.",
    "Your tracking ability fades away.",        /* 108 */
    "spell 109 please report.",
    "",
    "spell 111 please report.",
    "spell 112 please report.",
    "spell 113 please report.",
    "spell 114 please report.",
    "spell 115 please report.",
    "spell 116 please report.",
    "spell 117 please report.",
    "You don't feel so aided anymore",
    "spell 119 please report.",
    "",
    "You feel a lot smarter.",
    "spell 122 please report.",
    "spell 123 please report.",
    "spell 124 please report.",
    "You shrink down to your original size.",
    "spell 126 please report.",
    "Your head clears and you feel you can shape your spell energies again.",
    "spell 128 please report.",
    "The bond breaks, and your animal friends go their own way.",
    "The bond breaks, and your animal friends go their own way.",
    "The bond breaks, and your animal friends go their own way.",
    "You lose your link to the Elemental Plane of Fire.",
    "You lose your link to the Elemental Plane of Earth.",
    "You lose your link to the Elemental Plane of Water.",
    "You lose your link to the Elemental Plane of Air.",
    "spell 136 please report.",
    "spell 137 please report.",
    "spell 138 please report.",
    "spell 139 please report.",
    "spell 140 please report.",
    "You don't feel so in touch with the trees anymore",
    "You do not feel so fleet of foot anymore.",
    "You feel less intouch with animals.",
    "",
    "spell 145 please report.",
    "You are free of your entanglements",
    "You are free of the snare!",
    "spell 148 please report.",
    "Your barkskin fades away.",
    "spell 150 please report.",
    "spell 151 please report.",
    "Your equipment finally cools down!",
    "You feel less aware of possible traps.",
    "spell 154 please report.",
    "You feel slower!",
    "You feel faster!",
    "",
    "spell 158 please report.",
    "spell 159 please report.",
    "spell 160 please report.",
    "You feel louder.",
    "spell 162 please report.",
    "spell 163 please report.",
    "spell 164 please report.",
    "You feel less in control of dragons",
    "spell 166 please report.",
    "Affected by energy restore, please report.",
    "You sense the white in your vision disappear.",
    "You feel less protected from good beings.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "You feel like you might be able to swim again.",   /* 188 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 197 */
    "",
    "",
    "",                         /* 200 geyser */
    "Your wings finally cramp and give out.",   /* 201 Wings Fly */
    "Your wings stop feeling sore and tired.",  /* 202 Wings Tired */
    "At long last your wings have healed enough to allow flight.",
    /* 203 Wings Burned */
    "You feel less protected from good beings.",
    "You no longer feel big.",
    "",
    "!berserk!",                /* 207 */
    "!tan!",
    "!avoid backattack!",
    "!find food!",
    "!Find Water!",
    "You feel guilty knowing you have not prayed in a day",     /* 212 */
    "You stop memorizing and meditating.",
    "!bellow!",
    "The dark globe about your fades away",
    "The protective globe about your body fizzles out",
    "The protective globe about your body flares and is suddenly gone",
    "You feel less protected from draining",
    "You feel less protected from dragons",
    "The antimagic shell about you flickers and is gone",
    "",
    "",
    "",
    "",
    "spell 225 please report.", /* 225 */
    "",
    "",
    "",
    "",
    "",
    "You feel you have meditated all that you can now.",        /* 231 */
    "",
    "",
    "",
    "",
    "",
    "",
    "Your deity is prepared to grant you another boon.",
    "",
    "",
    "Your psionic shield flickers and then vanishes.",  /* 241 */
    "You feel less protected from evil.",       /* 242 */
    "spell 243 report",
    "spell 244 report",
    "spell 245 report",
    "spell 246 report",
    "spell 247 report",
    "spell 248 report",
    "spell 249 report",
    "spell 250 report",
    "spell 251 report",
    "spell 252 report",
    "spell 253 report",
    "You finally have to stop listening to others thoughts",
    "You feel your ability to understand other languages fade",
    "You feel fearful of fire again",
    "You feel more fearful of cold again",
    "You feel more fearful of energy again",
    "You feel more fearful of electricity again",
    "",
    "",
    "You think it would be best to avoid fire breathing dragons now",   /* 262
                                                                         */
    "You think it would be best to avoid frost breathing dragons now",
    "You think it would be best to avoid electric breathing dragons now",
    "You think it would be best to avoid acid breathing dragons now",
    "You think it would be best to avoid gas breathing dragons now",
    "Your wizardeye fades away with a *pop*",
    "",                         /* mind burn */
    "You no longer can concentrate on distant sights.",
    "You can no longer sense dangers nearby.",
    "",
    "",
    "You feel yourself gently floating to the ground as the levitation wears off.",
    "",
    "",
    "You feel your mentally enhanced strength fade away.",
    "You get a grumbling in your stomach.",
    "",
    "",
    "",
    "",
    "",
    "You feel your tower of protection fall apart.",
    "You can no longer blank your mind",
    "",
    "",
    "Your attributes revert to normal as you lower your intensify powers.",
    "",                         /* spot */
    "",                         /* 289 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 300 */
    "",
    "",
    "",
    "",
    "",                         /* start necro spells, 305 - cold light */
    "As the last traces of illness are purged from your body, you feel better.",
    "You slowly fade back into the undead's view.",
    "",
    "The bones reinfusing your armor fall apart.",
    "The battlescarred armor surrounding you winks out of existence.",/* 310 */
    "The cloak of darkness lift, and you can see once more.",
    "You feel in control of yourself once more.",
    "",
    "Your spectral strength modification slowly fades away.",
    "You fade back into plain view.",
    "",
    "",
    "Your skin loses its cold-resistant quality.",
    "",
    "",                         /* 320 */
    "",
    "Your body halts the process of decay.",
    "",
    "",
    "",
    "",
    "",
    "The eye of the dead ceases to aid you.",
    "",
    "",                         /* 330 */
    "Your agreement with your Lord has come to an end.",
    "",
    "The negative aura surrounding your hands blinks out.",
    "",
    "",
    "",
    "Your mental reserves have restored enough to raise another flesh golem.",
    "The aura of freezing flames that surrounds you melts.",    /* 338 */
    "wall of thought",
    "mind tap",
    "Your blade barrier winks out of existence.",
    "Mana no longer protects you.",     /* 342 SPELL_MANA_SHIELD */
    "Your iron skins rust and fall off.",       /* 343 SKILL_IRON_SKINS */
    "",
    "",
    "",
    "",                         /* 347 SKILL_FLURRY */
    "", "\n"
};

const char     *spell_wear_off_room_msg[] = {
    "RESERVED DB.C",
    "$n's mystic armor fades slowly away",
    "!Teleport!",
    "$n looks less blessed",
    "$n blinks $s eyes.",
    "!Burning Hands!",
    "!Call Lightning",
    "$n shivers and shakes",
    "$n seems a bit stronger",
    "!Clone!",
    "!Color Spray!",            /* 10 */
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "$n looks better",
    "$n blinks $s eyes, the reddish hue vanishes",
    "$n blinks $s eyes, the yellowish hue vanishes",
    "$n blinks $s eyes, the blueish hue vanishes",      /* 20 */
    "$n blinks $s eyes, the whiteish hue vanishes",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "$n slowly fades into existence",
    "!Lightning Bolt!",         /* 30 */
    "!Locate object!",
    "!Magic Missile!",
    "$n looks healthier",
    "$n seems less holier than thou",   /* prtct frm evil */
    "!Remove Curse!",
    "The white aura around $n's body fades.",
    "!Shocking Grasp!",
    "$n murmurs and shakes in $s sleep.",
    "$n looks weaker.",
    "!Summon!",
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "$n stops looking around so much",
    "",                         /* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "!Identify!",
    "$n stumbles, and blinks $s eyes, the reddish hue fades",
    "!cause light!",
    "!cause crit!",
    "!flamestrike!",
    "!dispel magic!",
    "$n looks stronger!",
    "!dispel good!",
    "!knock!",
    "!know alignment!",
    "!animate dead!",
    "$n jerks out of $s paralyzed state",
    "!remove paralysis!",
    "!fear!",
    "!acid blast!",
    "$n frantically sucks for air.",
    "The magical forces holding $n aloft vanish",
    "spell1, please report.",   /* 70 */
    "spell2, please report.",
    "spell72, please report.",
    "$n's magic shield fades away",
    "spell74, please report.",
    "spell6, please report.",
    "spell7, please report.",
    "spell8, please report.",
    "spell9, please report.",
    "spell10, please report.",
    "spell11, please report.",  /* 80 */
    "The red glow around $n's body fades",
    "spell82, please report.",
    "spell83, please report.",
    "spell84, please report.",
    "spell85, please report.",
    "spell86, please report.",
    "spell87, please report.",
    "spell88, please report.",
    "spell89, please report.",
    "spell90, please report.",  /* 90 */
    "spell91, please report.",
    "$n's skin loses its stoney appearance.",
    "spell93, please report.",
    "$n blinks rapidly, as the silvery hue fades from $s eyes",
    "spell95, please report",
    "The pink glow around $n's body fades.",    /* 96 */
    "spell 97, please report.",
    "spell 98 please report.",
    "spell 99 please report.",
    "spell 100 please report.",
    "spell 101 please report.",
    "spell 102 please report.",
    "spell 103 please report.",
    "The magical shield protecting $n fades away sadly.",
    "spell 105 please report.",
    "$n seems to be free of the webs that hold $m",
    "$n looks confused.",
    "$n looks REALLY confused", /* 108 */
    "spell 109 please report.",
    "",
    "spell 111 please report.",
    "spell 112 please report.",
    "spell 113 please report.",
    "spell 114 please report.",
    "spell 115 please report.",
    "spell 116 please report.",
    "spell 117 please report.",
    "$n seems less aided",
    "spell 119 please report.",
    "",
    "$n stops drooling on $mself.",
    "spell 122 please report.",
    "spell 123 please report.",
    "spell 124 please report.",
    "$n shrinks down to $s original size.",     /* animal growth */
    "spell 126 please report.",
    "",                         /* no message for creep wearing off */
    "spell 128 please report.",
    "",                         /* Animal Summon One */
    "",                         /* Animal Summon Two */
    "",                         /* animal summon three */
    "",                         /* Fire Servant */
    "",                         /* Earth Servant */
    "",                         /* Water Servant */
    "",                         /* Wind Servant */
    "spell 136 please report.",
    "spell 137 please report.",
    "spell 138 please report.",
    "spell 139 please report.",
    "spell 140 please report.",
    "",
    "$n seems to be a bit less fleet of foot.",
    "",
    "",
    "spell 145 please report.",
    "$n wrenches free of $s entanglements",
    "$n manages to get out of a magical snare",
    "spell 148 please report.",
    "$n's skin stops resembling bark",
    "spell 150 please report.",
    "spell 151 please report.",
    "$n's equipment stops burning $m",
    "$n eyes loose a shimmering glimmer.",
    "spell 154 please report.",
    "$n seems slower",
    "$n seems faster",
    "",
    "spell 158 please report.",
    "spell 159 please report.",
    "spell 160 please report.",
    "spell 161 please report.",
    "spell 162 please report.",
    "spell 163 please report.",
    "spell 164 please report.",
    "",
    "spell 166 please report.",
    "Affected by Energy Restore please report.",
    "$n blinks $s eyes, the white hue vanishes",
    "$n looks a bit less unholy.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 200 */
    "$n's wings falter and give out from exhaustion.",  /* 201 Wings Fly */
    "",
    "",
    "$n looks a bit less unholy.",
    "$n shrinks in size.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 212 */
    "$n stops meditating and closes $s spell book.",
    "",
    "The dark globe around $n fades away",      /* 215 */
    "The protective globe about $n fizzles out",
    "The protective globe about $n flares and is gone",
    "$n looks more fearful of vampires",
    "$n looks more fearful of dragons",
    "The shimmering shield around $n sparks brightly and fades away",
    "",
    "",
    "",
    "",
    "spell 225 please report.", /* 225 */
    "",
    "",
    "",
    "",
    "",
    "$n stops humming and looks a bit more relaxed now.",       /* 231 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "$n's psionic shield flickers then fades away.",    /* 241 */
    "$n looks fearful of evil creatures again.",
    "spell 243",
    "spell 244",
    "spell 245",
    "spell 246",
    "spell 247",
    "spell 248",
    "spell 249",
    "spell 250",
    "spell 251",
    "spell 252 report",
    "spell 253 report",
    "",                         /* 254 */
    "",                         /* 255 */
    "$n looks less protected from fire",
    "$n looks less protected from cold",
    "$n looks less protected from energy",
    "$n looks less protected from electricity now",
    "",
    "",
    "$n's fire breath protection fades away",   /* 262 */
    "$n's frost breath protection fades away",
    "$n's electric breath protection fades away",
    "$n's acid breath protection fades away",
    "$n's gas breath protection fades away",
    "$n's wizardeye fades away with a *pop*",
    "",
    "",
    "",
    "",
    "",
    "$n floats slowly to the ground.",
    "",
    "",
    "$n looks a bit weaker.",
    "",
    "$n begins to look hungry.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* spot 288 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* start necro spells, 305 - cold light */
    "$n looks better, now that the last traces of illness have disappeared.",
    "",
    "",
    "$n's magical suit of bones crumbles to dust.",
    "The battlescarred armor surrounding $n winks out of existence.", /* 310 */
    "The cloak of darkness lifts from $n's eyes.",
    "$n seems to be in control of $s own actions once more..",
    "",
    "",
    "The shadows surrounding $n disappear, and $e fades into view.",
    "",
    "",
    "$n's skin loses its palue hue.",
    "",
    "",                         /* 320 */
    "",
    "The stench of decay disappears as $n's body heals up.",
    "",
    "",
    "",
    "",
    "",
    "The eerie blue light in $n's eye dims.",
    "",
    "",                         /* 330 */
    "",
    "",
    "The negative aura surrounding $n's hands blinks out.",
    "",
    "",
    "",
    "",
    "The aura of freezing flames surrounding $n melts.",        /* 338 */
    "wall of thought",
    "mind tap",
    "$n's blade barrier winks out of existence.",
    "$n's mana shield exhausts itself", /* 342 SPELL_MANA_SHIELD */
    "$n's iron skins rust away.",       /* 343 SPELL_IRON_SKINS */
    "",
    "",
    "",
    "",                         /* 347 SKILL_FLURRY */
    "", "\n"
};

const char     *spell_wear_off_soon_msg[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "You are starting to gain back your will",
    "You feel somewhat stronger",
    "!Clone!",
    "!Color Spray!",
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "",
    "",
    "",
    "",
    "",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "",
    "!Lightning Bolt!",
    "!Locate object!",
    "!Magic Missile!",
    "",
    "",
    "!Remove Curse!",
    "The white aura around your body flickers slightly.",
    "!Shocking Grasp!",
    "You feel more awake.",
    "You feel a bit weaker",
    "!Summon!",
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "",
    "",                         /* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "!Identify!",
    "Your infravision begins to fade",
    "!cause light!",
    "!cause crit!",
    "!flamestrike!",
    "!dispel good!",
    "You feel a bit stronger",
    "!dispel magic!",
    "!knock!",
    "!know alignment!",
    "!animate dead!",
    "",
    "!remove paralysis!",
    "!fear!",
    "!acid blast!",
    "Your throat feels tight, your water breathing spell is leaving you ",
    "You feel heavier now, your flying ability is leaving you.",
    "spell1, please report.",   /* 70 */
    "spell2, please report.",
    "spell3, please report.",
    "Your shield spell starts to flicker",
    "spell5, please report.",
    "spell6, please report.",
    "spell7, please report.",
    "spell8, please report.",
    "spell9, please report.",
    "spell10, please report.",
    "spell11, please report.",  /* 80 */
    "The red glow around your body flickers",
    "spell82, please report.",
    "spell83, please report.",
    "spell84, please report.",
    "spell85, please report.",
    "spell86, please report.",
    "spell87, please report.",
    "spell88, please report.",
    "spell89, please report.",
    "spell90, please report.",  /* 90 */
    "spell91, please report.",
    "",
    "spell93, please report.",
    "",
    "spell95, please report",
    "The pink glow around your body flickers",  /* 96 */
    "spell 97, please report.",
    "spell 98 please report.",
    "spell 99 please report.",
    "spell 100 please report.",
    "spell 101 please report.",
    "spell 102 please report.",
    "spell 103 please report.",
    "The magical shield around your body flickers",
    "spell 105 please report.",
    "The webs seem a bit less strong.",
    "",
    "",                         /* 108 */
    "spell 109 please report.",
    "",
    "spell 111 please report.",
    "spell 112 please report.",
    "spell 113 please report.",
    "spell 114 please report.",
    "spell 115 please report.",
    "spell 116 please report.",
    "spell 117 please report.",
    "",
    "spell 119 please report.",
    "",
    "Things begin to make a little more sense. Then you wet yourself.",
    "spell 122 please report.",
    "spell 123 please report.",
    "spell 124 please report.",
    "Maybe size isn't all that matters.",       /* animal growth */
    "spell 126 please report.",
    "Your head begins to clear, but your mind isn't up to shaping spell energies yet.",
    "spell 128 please report.",
    "You feel your bond to the animals begin to slip.",
    "You feel your bond to the animals begin to slip.",
    "You feel your bond to the animals begin to slip.",
    /* servants */
    "Your link to the Elemental Plane of Fire is becoming tenuous.",
    "Your link to the Elemental Plane of Earth is becoming tenuous.",
    "Your link to the Elemental Plane of Water is becoming tenuous.",
    "Your link to the Elemental Plane of Air is becoming tenuous.",
    "spell 136 please report.",
    "spell 137 please report.",
    "spell 138 please report.",
    "spell 139 please report.",
    "spell 140 please report.",
    "You feel less in common with trees.",
    "You feel less fleet of foot for a moment.",
    "",
    "",
    "spell 145 please report.",
    "",
    "",
    "spell 148 please report.",
    "Your barkskin is starting to fade",
    "spell 150 please report.",
    "spell 151 please report.",
    "Your armor cools down a bit",
    "You feel your sense of traps starting to fade",
    "spell 154 please report.",
    "You feel the world speeding up a bit",
    "You feel the world slowing down a bit",
    "",
    "spell 158 please report.",
    "spell 159 please report.",
    "spell 160 please report.",
    "spell 161 please report.",
    "spell 162 please report.",
    "spell 163 please report.",
    "spell 164 please report.",
    "",
    "spell 166 please report.",
    "spell Energy Restore please report.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 200 */
    "Your wings begin to get very sore... you'd better land soon.", /* 201 */
    "You stretch your wings.  They should support you again soon.",
    "",
    "Your protection from good wavers briefly.",
    "",                         /* 205 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 212 */
    "You flip through the final pages of the spell.",
    "",
    "You sense your globe of darkness fading",  /* 215 */
    "The minor globe about you shimmers briefly",
    "The major globe about you flickers briefly",
    "",                         /* 218 prot_energy */
    "",                         /* 219 prot_dragon */
    "The antimagic shell around you sparks",
    "",
    "",
    "",
    "",
    "",                         /* 225 */
    "",
    "",
    "",
    "You start recovering from Psionic Blast",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 235 */
    "",
    "",
    "Your deity is prepared to grant a boon soon.",
    "spell 239 report",
    "spell 240 report",
    "Your psionic shield flickers briefly",     /* 241 */
    "Your protection from evil wavers briefly", /* prot from evil 10ft */
    "spell 243",
    "spell 244",
    "spell 245",
    "spell 246",
    "spell 247",
    "spell 248",
    "spell 249",
    "spell 250",
    "spell 251",
    "spell 252",
    "spell 253 REPORT",
    "You feel your mind sensing ability waver briefly",
    "You sense your ability to understand languages waver briefly",
    "Your protection from fire wavers for a moment",
    "Your protection from cold wavers for a moment",
    "Your protection from energy wavers for a moment",
    "Your protection from electricity wavers for a moment",
    "",
    "",
    "Your globe of fire breath protection shimmers for a moment "
        "before pulsing strongly",       /* 262 */
    "Your globe of frost breath protection shimmers for a moment "
        "before pulsing strongly",
    "Your globe of electricity breath protection shimmers for a moment "
        "before pulsing strongly",
    "Your globe of acid breath protection shimmers for a moment "
        "before pulsing strongly",
    "Your globe of gas breath protection shimmers for a moment "
        "before pulsing strongly",
    "You lose control of your wizardeye for a moment",
    "",
    "",
    "",
    "",
    "",
    "You feel a falling sensation briefly.",    /* 273 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 288 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* start necro spells, 305 - cold light */
    "You feel a bit better, but there's still traces of corruption in your blood.",
    "Your protection from undead eyes wavers for a moment.",
    "",
    "",
    "",                         /* 310 */
    "The cloak of darkness draped over your vision seems to shift slightly..",
    "You feel a bit more in control of yourself.",
    "",
    "",
    "The shadows surrounding your person seem to shift slightly.",
    "",
    "",
    "The white hue of your skin seems to tan slightly..",
    "",
    "",                         /* 320 */
    "",
    "Your body has done much in halting the process of decay.",
    "",
    "",
    "",
    "",
    "",
    "Your eye of the dead seems to stray a bit.",
    "",
    "",                         /* 330 */
    "Your agreement with your Lord is close to coming to an end.",
    "",
    "The negative aura surrounding your hands begins to waver.",
    "",
    "",
    "",
    "",
    "Your aura of freezing flames hisses softly.",      /* 338 */
    "wall of thought",
    "mind tap",
    "Your blade barrier wavers a moment.",
    "Your mana protection weakens some.",       /* SPELL_MANA_SHIELD */
    "Your iron skins are nearly rusted away.",  /* SPELL_IRON_SKINS */
    "",
    "",
    "",
    "",
    "",
    "\n"
};

const char     *spell_wear_off_soon_room_msg[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "$n seems more in control of $mself",
    "",
    "!Clone!",
    "!Color Spray!",
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "",
    "",
    "",
    "",
    "",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "",
    "!Lightning Bolt!",
    "!Locate object!",
    "!Magic Missile!",
    "",
    "",
    "!Remove Curse!",
    "The white aura around $n's body flickers slightly.",
    "!Shocking Grasp!",
    "$n seems a bit more awake",
    "$n seems a bit weaker",
    "!Summon!",
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "",
    "",                         /* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "!Identify!",
    "",
    "!cause light!",
    "!cause crit!",
    "!flamestrike!",
    "!dispel good!",
    "$n's eyes stop looking so red.  Must have used Visine",
    "!dispel magic!",
    "!knock!",
    "!know alignment!",
    "!animate dead!",
    "",
    "!remove paralysis!",
    "!fear!",
    "!acid blast!",
    "$n gasps for air for a moment or two",
    "The magic force keeping $n aloft flickers slightly, it will vanish soon.",
    "spell1, please report.",   /* 70 */
    "spell2, please report.",
    "spell3, please report.",
    "$n's shield of force flickers slightly",
    "spell5, please report.",
    "spell6, please report.",
    "spell7, please report.",
    "spell8, please report.",
    "spell9, please report.",
    "spell10, please report.",
    "spell11, please report.",  /* 80 */
    "The red glow around $n's body flickers",
    "spell82, please report.",
    "spell83, please report.",
    "spell84, please report.",
    "spell85, please report.",
    "spell86, please report.",
    "spell87, please report.",
    "spell88, please report.",
    "spell89, please report.",
    "spell90, please report.",  /* 90 */
    "spell91, please report.",
    "",
    "spell93, please report.",
    "",
    "spell95, please report",
    "The pink glow around $n's body flickers",  /* 96 */
    "spell 97, please report.",
    "spell 98 please report.",
    "spell 99 please report.",
    "spell 100 please report.",
    "spell 101 please report.",
    "spell 102 please report.",
    "spell 103 please report.",
    "$n's shield starts to fade away",
    "spell 105 please report.",
    "The webs surrounding $n seem to lose their grip a bit.",
    "",
    "",                         /* 108 */
    "spell 109 please report.",
    "",
    "spell 111 please report.",
    "spell 112 please report.",
    "spell 113 please report.",
    "spell 114 please report.",
    "spell 115 please report.",
    "spell 116 please report.",
    "spell 117 please report.",
    "",
    "spell 119 please report.",
    "",
    "You see a flicker of intelligence in $n's eyes, then its gone.",
    "spell 122 please report.",
    "spell 123 please report.",
    "spell 124 please report.",
    "",                         /* animal growth */
    "spell 126 please report.",
    "",                         /* no message for creeping death */
    "spell 128 please report.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "spell 136 please report.",
    "spell 137 please report.",
    "spell 138 please report.",
    "spell 139 please report.",
    "spell 140 please report.",
    "",
    "",
    "",
    "",
    "spell 145 please report.",
    "$n is entangled by leaves and bushes",
    "$n is ensnared",
    "spell 148 please report.",
    "$n's barklike skin starts to go away",
    "spell 150 please report.",
    "spell 151 please report.",
    "",
    "",
    "spell 154 please report.",
    "",
    "",
    "",
    "spell 158 please report.",
    "spell 159 please report.",
    "spell 160 please report.",
    "spell 161 please report.",
    "spell 162 please report.",
    "spell 163 please report.",
    "spell 164 please report.",
    "",
    "spell 166 please report.",
    "spell Energy Restore please report.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 200 */
    "$n's wings falter momentarily, but continue to keep $s aloft.",/* 201 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 212 */
    "$n flips through the pages in $s spell book intently reading.",
    "",
    "",                         /* 215 globe_dark */
    "The protective globe around $n shimmers briefly",
    "The protective globe around $n flickers slightly",
    "",                         /* prot_energy */
    "",                         /* prot_dragon */
    "The protective globe about $n sparks suddenly",
    "",
    "",
    "",
    "",
    "",                         /* 225 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 235 */
    "",
    "",
    "",
    "",
    "",
    "$n's protective shield flickers briefly",  /* 241 */
    "",                         /* prot evil 10ft */

    "spell 243",
    "spell 244",
    "spell 245",
    "spell 246",
    "spell 247",
    "spell 248",
    "spell 249",
    "spell 250",
    "spell 251",
    "spell 252",
    "spell 253",
    "",                         /* 254 */
    "",
    "",
    "$n's fire protection wavers briefly",      /* 256 */
    "$n's cold protection wavers briefly",
    "$n's energy protection wavers briefly",
    "$n's electric protection wavers briefly",
    "",                         /* 260 */
    "",
    "$n's fire breath globe of protection shimmers for a moment", /* 262 */
    "$n's frost breath globe of protection shimmers for a moment",
    "$n's electricity breath globe of protection shimmers for a moment",
    "$n's acid breath globe of protection shimmers for a moment",
    "$n's gas breath globe of protection shimmers for a moment",
    "",                         /* no wizardeye message */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 288 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* start necro spells, 305 - cold light */
    "",
    "",
    "",
    "",
    "",                         /* 310 */
    "",
    "$n seems a bit more in control of $mself.",
    "",
    "",
    "",
    "",
    "",
    "The white hue of $n's skin seems to tan slightly..",
    "",
    "",                         /* 320 */
    "",
    "The stench of decay seems to abate a bit.",
    "",
    "",
    "",
    "",
    "",
    "The cold blue light in $n's eye pulses a moment.",
    "",
    "",                         /* 330 */
    "",
    "",
    "The negative aura surrounding $n's hands begins to waver.",
    "",
    "",
    "",
    "",
    "$n's aura of freezing flames hisses softly.",      /* 338 */
    "wall of thought",
    "mind tap",
    "$n's blade barrier wavers a moment.",
    "$n's mana protection fades momentarily.",  /* SPELL_MANA_SHIELD */
    "$n's skin seems to rust a bit.",   /* SPELL_IRON_SKINS */
    "",
    "",
    "",
    "",
    "",
    "\n"
};

const int       rev_dir[] = {
    2,
    3,
    0,
    1,
    5,
    4
};

const int       TrapDir[] = {
    TRAP_EFF_NORTH,
    TRAP_EFF_EAST,
    TRAP_EFF_SOUTH,
    TRAP_EFF_WEST,
    TRAP_EFF_UP,
    TRAP_EFF_DOWN
};

const int       movement_loss[] = {
    1,                          /* Inside */
    2,                          /* City */
    2,                          /* Field */
    3,                          /* Forest */
    4,                          /* Hills */
    6,                          /* Mountains */
    8,                          /* Swimming */
    10,                         /* Unswimable */
    2,                          /* Flying */
    20,                         /* Submarine */
    4,                          /* Desert */
    1,                          /* Tree */

    15,                         /* SECT_SEA 12 New ascii sea */
    0,                          /* SECT_BLANK 13 */
    6,                          /* SECT_ROCK_MOUNTAIN 14 */
    7,                          /* SECT_SNOW_MOUNTAIN 15 */
    3,                          /* SECT_RUINS 16 */
    4,                          /* SECT_JUNGLE 17 */
    4,                          /* SECT_SWAMP 18 */
    7,                          /* SECT_LAVA 19 */
    2,                          /* SECT_ENTRANCE 20 */
    3,                          /* SECT_FARM 21 */
    0                           /* SECT_EMPTY 22 */
};

const char     *exits[] = {
    "North",
    "East ",
    "South",
    "West ",
    "Up   ",
    "Down "
};
                /*
                 * used in listing exits for this room
                 */
const char     *listexits[] = {
    "$c0010North",
    "$c0011East",
    "$c0012South",
    "$c0013West",
    "$c0014Up",
    "$c0015Down"
};

const char     *dirs[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"
};

const char     *ItemDamType[] = {
    "burned",
    "frozen",
    "electrified",
    "crushed",
    "corroded",
    "burned",
    "frozen",
    "shredded",
    "\n"
};

const char     *weekdays[7] = {
    "the Day of the Sword",
    "the Day of the Bow",
    "the Day of the Axe",
    "the Day of the Mace",
    "the Day of Battle",
    "the day of Grace",
    "the Day of Peace"
};

const char     *month_name[17] = {
    "Month of Winter",          /* 0 */
    "Month of the Winter Wolf",
    "Month of the Frost Giant",
    "Month of the Old Forces",
    "Month of the Grand Struggle",
    "Month of the Spring",
    "Month of Nature",
    "Month of Fertility",
    "Month of the Dragon",
    "Month of the Sun",
    "Month of the Heat",
    "Month of the Battle",
    "Month of the God Wars",
    "Month of the Shadows",
    "Month of the Long Shadows",
    "Month of the Ancient Darkness",
    "Month of the Great Evil"
};

const int       sharp[] = {
    0,
    0,
    0,
    1,                          /* Slashing */
    0,
    0,
    0,
    0,                          /* Bludgeon */
    0,
    0,
    0,
    0
};                              /* Pierce */

const char     *where[] = {
    "$c000B<$c000wused as light$c000B>$c000w      :",
    "$c000B<$c000wworn on finger$c000B>$c000w     :",
    "$c000B<$c000wworn on finger$c000B>$c000w     :",
    "$c000B<$c000wworn around neck$c000B>$c000w   :",
    "$c000B<$c000wworn around neck$c000B>$c000w   :",
    "$c000B<$c000wworn on body$c000B>$c000w       :",
    "$c000B<$c000wworn on head$c000B>$c000w       :",
    "$c000B<$c000wworn on legs$c000B>$c000w       :",
    "$c000B<$c000wworn on feet$c000B>$c000w       :",
    "$c000B<$c000wworn on hands$c000B>$c000w      :",
    "$c000B<$c000wworn on arms$c000B>$c000w       :",
    "$c000B<$c000wworn as shield$c000B>$c000w     :",
    "$c000B<$c000wworn about body$c000B>$c000w    :",
    "$c000B<$c000wworn about waist$c000B>$c000w   :",
    "$c000B<$c000wworn around wrist$c000B>$c000w  :",
    "$c000B<$c000wworn around wrist$c000B>$c000w  :",
    "$c000B<$c000wwielded$c000B>$c000w            :",
    "$c000B<$c000wheld$c000B>$c000w               :",
    "$c000B<$c000wworn on back$c000B>$c000w       :",
    "$c000B<$c000wworn in right ear$c000B>$c000w  :",
    "$c000B<$c000wworn in left ear$c000B>$c000w   :",
    "$c000B<$c000wworn on eyes$c000B>$c000w       :",
    "$c000B<$c000wnotched$c000B>$c000w            :"
};

const char     *drinks[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "coca cola",
    "\n"
};

const char     *drinknames[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "cola",
    "\n"
};
#if 1

const int       RacialMax[MAX_RACE + 1][MAX_CLASS] = {

    /*
     * m c f t d k b s p r psi nec
     */
    /*
     * mutt
     */
     {25, 25, 25, 25, 25, 25, 10, 25, 0, 0, 0, 0},
    /*
     * Hum
     */
     {51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51},
    /*
     * moonelf
     */
     {51, 30, 20, 51, 51, 20, 0, 51, 40, 51, 40, 0},
    /*
     * dwrf
     */
     {0, 51, 51, 35, 0, 0, 0, 0, 45, 30, 0, 0},
    /*
     * 1/2
     */
     {20, 40, 30, 51, 51, 20, 0, 20, 0, 0, 0, 0},
    /*
     * r_gnm
     */
     {51, 51, 30, 40, 20, 20, 0, 51, 0, 0, 0, 0},
    /*
     * rep
     */
     {20, 30, 40, 50, 51, 40, 0, 20, 0, 0, 0, 0},
    /*
     * Myst
     */
     {50, 50, 50, 1, 51, 1, 0, 50, 0, 0, 0, 0},
    /*
     * were
     */
     {30, 30, 51, 50, 40, 20, 0, 30, 0, 0, 0, 0},
    /*
     * drag
     */
     {51, 20, 51, 30, 20, 51, 0, 51, 0, 0, 0, 0},
    /*
     * unded
     */
     {35, 35, 35, 35, 35, 35, 0, 35, 0, 0, 0, 0},
    /*
     * ork
     */
     {20, 30, 51, 45, 0, 0, 40, 0, 0, 0, 0, 0},
    /*
     * insct
     */
     {30, 20, 40, 51, 50, 30, 10, 30, 0, 0, 0, 0},
    /*
     * arach
     */
     {20, 30, 40, 51, 50, 30, 10, 20, 0, 0, 0, 0},
    /*
     * saur
     */
     {20, 30, 51, 40, 50, 30, 10, 20, 0, 0, 0, 0},
    /*
     * fish
     */
     {20, 40, 30, 50, 51, 30, 10, 20, 0, 0, 0, 0},
    /*
     * bird
     */
     {40, 30, 30, 50, 51, 20, 10, 40, 0, 0, 0, 0},
    /*
     * giant
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * pred
     */
     {40, 30, 51, 20, 30, 50, 10, 40, 0, 0, 0, 0},
    /*
     * para
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * slime
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * demon
     */
     {51, 30, 40, 50, 30, 20, 10, 51, 0, 0, 0, 0},
    /*
     * snake
     */
     {40, 30, 30, 51, 50, 20, 10, 40, 0, 0, 0, 0},
    /*
     * herb
     */
     {30, 40, 20, 30, 51, 50, 10, 30, 0, 0, 0, 0},
    /*
     * tree
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * vegan
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * elmnt
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * planr
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * devil
     */
     {50, 51, 40, 20, 30, 40, 10, 50, 0, 0, 0, 0},
    /*
     * ghost
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * gobln
     */
     {20, 30, 50, 51, 0, 0, 30, 0, 0, 0, 0, 50},
    /*
     * troll
     */
     {0, 35, 51, 35, 0, 0, 45, 0, 0, 0, 0, 0},
    /*
     * vegmn
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * mndf
     */
     {51, 40, 30, 30, 30, 50, 10, 51, 0, 0, 0, 0},
    /*
     * prim
     */
     {30, 30, 40, 50, 20, 51, 10, 30, 0, 0, 0, 0},
    /*
     * enfn
     */
     {50, 30, 51, 40, 20, 30, 10, 50, 0, 0, 0, 0},
    /*
     * drow
     */
     {50, 51, 30, 40, 20, 30, 0, 50, 40, 51, 40, 40},
    /*
     * golem
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * skexi
     */
     {50, 30, 40, 51, 30, 20, 10, 50, 0, 0, 0, 0},
    /*
     * trog
     */
     {30, 40, 50, 51, 30, 20, 10, 30, 0, 0, 0, 0},
    /*
     * patrn
     */
     {51, 30, 50, 30, 20, 40, 10, 51, 0, 0, 0, 0},
    /*
     * labrn
     */
     {50, 30, 51, 40, 20, 30, 10, 50, 0, 0, 0, 0},
    /*
     * sartn
     */
     {50, 51, 30, 20, 40, 30, 10, 50, 0, 0, 0, 0},
    /*
     * tytn
     */
     {50, 30, 51, 30, 40, 20, 10, 50, 0, 0, 0, 0},
    /*
     * smrf
     */
     {10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0},
    /*
     * roo
     */
     {40, 30, 51, 30, 20, 50, 10, 40, 0, 0, 0, 0},
    /*
     * horse
     */
     {30, 40, 20, 30, 51, 50, 10, 30, 0, 0, 0, 0},
    /*
     * drgdm
     */
     {51, 30, 20, 51, 40, 20, 10, 51, 0, 0, 0, 0},
    /*
     * astrl
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * god
     */
     {50, 50, 50, 50, 51, 50, 50, 50, 50, 50, 50, 50},
    /*
     * giant hill
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * giant frost
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * giant fire
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * giant cloud
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * giant storm
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * giant stone
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0, 0},
    /*
     * drag red
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag black
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag green
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag white
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag blue
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag silver
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag gold
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag bronze
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag cooper
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * drag brass
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0, 0},
    /*
     * unded vamp
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded lich
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded wigh
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded ghas
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded spec
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded zomb
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded skel
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * unded ghou
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0, 0},
    /*
     * halfelf
     */
     {30, 35, 35, 51, 35, 30, 0, 30, 51, 51, 40, 0},
    /*
     * half-ogre
     */
     {0, 30, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * half-orc
     */
     {0, 30, 45, 51, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * half-gian
     */
     {0, 0, 51, 0, 0, 0, 45, 0, 0, 0, 0, 0},
    /*
     * lizardman
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0, 50},
    /*
     * dar-dwrf
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0, 40},
    /*
     * dep-gnome
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0, 50},
    /*
     * gnoll
     */
     {20, 40, 51, 50, 30, 30, 0, 20, 0, 0, 0, 0},
    /*
     * goldelf
     */
     {51, 35, 15, 51, 51, 20, 0, 51, 45, 51, 40, 0},
    /*
     * wildelf
     */
     {30, 35, 51, 51, 51, 0, 0, 30, 0, 51, 40, 0},
    /*
     * sea-elf
     */
     {51, 30, 30, 51, 51, 20, 0, 51, 40, 51, 40, 0},
    /*
     * for-gnm
     */
     {30, 50, 40, 45, 50, 0, 0, 30, 0, 50, 0, 0},
    /*
     * avariel
     */
     {45, 40, 50, 0, 0, 0, 0, 45, 0, 0, 0}
    /*
     * m c f t d k b s p r psi ne
     */

};

#else

const int       RacialMax[MAX_RACE + 1][MAX_CLASS] = {

    /*
     * m c f t d k b s p r psi
     */
    /*
     * mutt
     */
     {25, 25, 25, 25, 25, 25, 10, 25, 0, 0, 0, 0},
    /*
     * Hum
     */
     {51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51},
    /*
     * moonelf
     */
     {51, 30, 20, 51, 51, 20, 0, 51, 40, 51, 40},
    /*
     * dwrf
     */
     {0, 51, 51, 35, 0, 0, 0, 0, 45, 30, 0},
    /*
     * 1/2
     */
     {20, 40, 30, 51, 51, 20, 0, 20, 0, 0, 0},
    /*
     * r_gnm
     */
     {51, 51, 30, 40, 20, 20, 0, 51, 0, 0, 0},
    /*
     * rep
     */
     {20, 30, 40, 50, 51, 40, 0, 20, 0, 0, 0},
    /*
     * Myst
     */
     {50, 50, 50, 1, 51, 1, 0, 50, 0, 0, 0},
    /*
     * were
     */
     {30, 30, 51, 50, 40, 20, 0, 30, 0, 0, 0},
    /*
     * drag
     */
     {51, 20, 51, 30, 20, 51, 0, 51, 0, 0, 0},
    /*
     * unded
     */
     {35, 35, 35, 35, 35, 35, 0, 35, 0, 0, 0},
    /*
     * ork
     */
     {20, 30, 51, 45, 0, 0, 40, 0, 0, 0, 0},
    /*
     * insct
     */
     {30, 20, 40, 51, 50, 30, 10, 30, 0, 0, 0},
    /*
     * arach
     */
     {20, 30, 40, 51, 50, 30, 10, 20, 0, 0, 0},
    /*
     * saur
     */
     {20, 30, 51, 40, 50, 30, 10, 20, 0, 0, 0},
    /*
     * fish
     */
     {20, 40, 30, 50, 51, 30, 10, 20, 0, 0, 0},
    /*
     * bird
     */
     {40, 30, 30, 50, 51, 20, 10, 40, 0, 0, 0},
    /*
     * giant
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * pred
     */
     {40, 30, 51, 20, 30, 50, 10, 40, 0, 0, 0},
    /*
     * para
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * slime
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * demon
     */
     {51, 30, 40, 50, 30, 20, 10, 51, 0, 0, 0},
    /*
     * snake
     */
     {40, 30, 30, 51, 50, 20, 10, 40, 0, 0, 0},
    /*
     * herb
     */
     {30, 40, 20, 30, 51, 50, 10, 30, 0, 0, 0},
    /*
     * tree
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * vegan
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * elmnt
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * planr
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * devil
     */
     {50, 51, 40, 20, 30, 40, 10, 50, 0, 0, 0},
    /*
     * ghost
     */
     {0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * gobln
     */
     {20, 30, 50, 51, 0, 0, 30, 0, 0, 0, 0},
    /*
     * troll
     */
     {0, 35, 51, 35, 0, 0, 45, 0, 0, 0, 0},
    /*
     * vegmn
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * mndf
     */
     {51, 40, 30, 30, 30, 50, 10, 51, 0, 0, 0},
    /*
     * prim
     */
     {30, 30, 40, 50, 20, 51, 10, 30, 0, 0, 0},
    /*
     * enfn
     */
     {50, 30, 51, 40, 20, 30, 10, 50, 0, 0, 0},
    /*
     * drow
     */
     {50, 51, 30, 40, 20, 30, 0, 50, 40, 51, 40},
    /*
     * golem
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * skexi
     */
     {50, 30, 40, 51, 30, 20, 10, 50, 0, 0, 0},
    /*
     * trog
     */
     {30, 40, 50, 51, 30, 20, 10, 30, 0, 0, 0},
    /*
     * patrn
     */
     {51, 30, 50, 30, 20, 40, 10, 51, 0, 0, 0},
    /*
     * labrn
     */
     {50, 30, 51, 40, 20, 30, 10, 50, 0, 0, 0},
    /*
     * sartn
     */
     {50, 51, 30, 20, 40, 30, 10, 50, 0, 0, 0},
    /*
     * tytn
     */
     {50, 30, 51, 30, 40, 20, 10, 50, 0, 0, 0},
    /*
     * smrf
     */
     {10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0},
    /*
     * roo
     */
     {40, 30, 51, 30, 20, 50, 10, 40, 0, 0, 0},
    /*
     * horse
     */
     {30, 40, 20, 30, 51, 50, 10, 30, 0, 0, 0},
    /*
     * drgdm
     */
     {51, 30, 20, 51, 40, 20, 10, 51, 0, 0, 0},
    /*
     * astrl
     */
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * god
     */
     {50, 50, 50, 50, 51, 50, 50, 50, 50, 50, 50},
    /*
     * giant hill
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * giant frost
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * giant fire
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * giant cloud
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * giant storm
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * giant stone
     */
     {20, 40, 51, 50, 30, 30, 10, 20, 0, 0, 0},
    /*
     * drag red
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag black
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag green
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag white
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag blue
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag silver
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag gold
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag bronze
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag cooper
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * drag brass
     */
     {51, 20, 51, 30, 20, 51, 10, 51, 0, 0, 0},
    /*
     * unded vamp
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded lich
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded wigh
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded ghas
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded spec
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded zomb
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded skel
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * unded ghou
     */
     {35, 35, 35, 35, 35, 35, 10, 35, 0, 0, 0},
    /*
     * halfelf
     */
     {30, 35, 35, 51, 35, 30, 0, 30, 51, 51, 40},
    /*
     * half-ogre
     */
     {0, 30, 51, 0, 0, 0, 0, 0, 0, 0, 0},
    /*
     * half-orc
     */
     {0, 30, 45, 51, 0, 0, 0, 0, 0, 0, 0},
    /*
     * half-gian
     */
     {0, 0, 51, 0, 0, 0, 45, 0, 0, 0, 0},
    /*
     * lizardman
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0},
    /*
     * dar-dwrf
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0},
    /*
     * dep-gnome
     */
     {20, 51, 51, 35, 15, 40, 0, 20, 45, 30, 0},
    /*
     * gnoll
     */
     {20, 40, 51, 50, 30, 30, 0, 20, 0, 0, 0},
    /*
     * goldelf
     */
     {51, 35, 15, 51, 51, 20, 0, 51, 45, 51, 40},
    /*
     * wildelf
     */
     {30, 35, 51, 51, 51, 0, 0, 30, 0, 51, 40},
    /*
     * sea-elf
     */
     {51, 30, 30, 51, 51, 20, 0, 51, 40, 51, 40},
    /*
     * for-gnm
     */
     {30, 50, 40, 45, 50, 0, 0, 30, 0, 50, 0},
    /*
     * avariel
     */
     {45, 40, 50, 0, 0, 0, 0, 45, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  /* New Races */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

    /*
     * m c f t d k b s p r psi
     */

};

#endif

/*
 * fire cold elec blow acid
 */

int             ItemSaveThrows[22][5] = {
    {15, 2, 10, 10, 10},
    {19, 2, 16, 2, 7},
    {11, 2, 2, 13, 9},
    {7, 2, 2, 10, 8},
    {6, 2, 2, 7, 13},
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {6, 2, 2, 7, 13},           /* treasure */
    {6, 2, 2, 7, 13},           /* armor */
    {7, 6, 2, 20, 5},           /* potion */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {19, 2, 2, 16, 7},
    {7, 6, 2, 20, 5},           /* drinkcon */
    {6, 2, 2, 7, 13},
    {6, 3, 2, 3, 10},
    {6, 2, 2, 7, 13},           /* treasure */
    {11, 2, 2, 13, 9},
    {7, 2, 2, 10, 8}
};

const int       drink_aff[][3] = {
    {0, 1, 10},                 /* Water */
    {3, 2, 5},                  /* beer */
    {5, 2, 5},                  /* wine */
    {2, 2, 5},                  /* ale */
    {1, 2, 5},                  /* ale */
    {6, 1, 4},                  /* Whiskey */
    {0, 1, 8},                  /* lemonade */
    {10, 0, 0},                 /* firebr */
    {3, 3, 3},                  /* local */
    {0, 4, -8},                 /* juice */
    {0, 3, 6},
    {0, 1, 6},
    {0, 1, 6},
    {0, 2, -1},
    {0, 1, -2},
    {0, 1, 5},
    {0, 0, 0}
};

const char     *color_liquid[] = {
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "black",
    "\n"
};

const char     *fullness[] = {
    "less than half ",
    "about half ",
    "more than half ",
    ""
};

/*
 * new level titles supplied by alfred
 */
const struct title_type titles[MAX_CLASS][ABS_MAX_LVL] = {
    {{"Man", "Woman", 0},       /* Mage */
     {"Supplicant", "Supplicant", 1},   /* Revision by Naril */
     {"Apprentice", "Apprentice", 2500},        /* Huge Revision by Rocky */
     {"Student", "Student", 5000},
     {"Scholar", "Scholar", 10000},
     {"Trickster", "Trickster", 20000},
     {"Illusionist", "Illusionist", 40000},
     {"Cabalist", "Cabalist", 60000},
     {"Apparitionist", "Apparitionist", 90000},
     {"Medium", "Gypsy", 135000},
     {"Scribe", "Scribe", 250000},      /* 10 */
     {"Sage", "Sage", 375000},
     {"Seer", "Seeress", 750000},
     {"Divinater", "Divinatress", 1250000},
     {"Alchemist", "Alchemist", 1500000},
     {"Evoker", "Evoker", 1875000},
     {"Necromancer", "Necromancer", 2250000},
     {"Abjurer", "Abjuress", 2625000},
     {"Invoker", "Invoker", 3000000},
     {"Enchanter", "Enchantress", 3375000},
     {"Conjurer", "Conjuress", 3750000},        /* 20 */
     {"Summoner", "Summoner", 4150000},
     {"Magician", "Magician", 4575000},
     {"Spiritualist", "Spiritualist", 5050000},
     {"Savant", "Savanti", 5500000},
     {"Shaman", "Shamaness", 6250000},
     {"Mystic", "Mystic", 7000000},
     {"Wiccan", "Wiccan", 7750000},
     {"Mentalist", "Mentalist", 8550000},
     {"Mnemonicist", "Mnemonicist", 9350000},
     {"Neuromancer", "Neuromancer", 10200000},  /* 30 */
     {"Spellbinder", "Spellbinder", 11200000},
     {"Synergist", "Synergist", 12200000},
     {"Occulist", "Occulist", 13200000},
     {"Thaumaturgist", "Thaumaturgist", 14500000},
     {"SpellMaster", "Spell Mistress", 16000000},
     {"Dispeller", "Dispeller", 18500000},
     {"Warlock", "War Witch", 20000000},
     {"Sorcerer", "Sorceress", 23000000},
     {"Wizard", "Wizardess", 26000000},
     {"Wizard", "Wizardess", 30000000}, /* 40 */
     {"Magus", "Incantrix", 34000000},
     {"Serpent Mage", "Serpent Magess", 38000000},
     {"Lich Mage", "Lich Magess", 42000000},
     {"Dragon Mage", "Dragon Magess", 46000000},
     {"High Wizard", "High Wizardess", 51000000},
     {"Grand Wizard", "Grand Wizardess", 56000000},
     {"High Mage", "High Magess", 61000000},
     {"Grand Mage", "Grand Magess", 66000000},
     {"Arch Magi", "Majestrix", 72000000},
     {"Master of Magic", "Mistress of Magic", 80000000},        /* 50 */
     {"Immortal Warlock", "Immortal Enchantress", 200000000},
     {"Immortal Warlock", "Immortal Enchantress", 231000000},
     {"Immortal Warlock", "Immortal Enchantress", 232000000},
     {"Immortal Warlock", "Immortal Enchantress", 233000000},
     {"Immortal Warlock", "Immortal Enchantress", 234000000},
     {"Immortal Warlock", "Immortal Enchantress", 235000000},
     {"Avatar of Magic", "Empress of Magic", 236000000},
     {"God of magic", "Goddess of magic", 237000000},
     {"Implementor", "Implementrix", 238000000},
     {"Implementor", "Implementrix", 239000000} /* 60 */
     },

    {{"Man", "Woman", 0},       /* Cleric */
     {"Layman", "Laywoman", 1},
     {"Seeker", "Seeker", 1500},
     {"Believer", "Believer", 3000},
     {"Novice", "Novice", 6000},
     {"Initiate", "Initiate", 13000},
     {"Attendant", "Attendant", 27500},
     {"Acolyte", "Acolyte", 55000},
     {"Minion", "Minion", 110000},
     {"Adept", "Adept", 225000},
     {"Seminarian", "Seminarian", 450000},      /* 10 */
     {"Missionary", "Missionary", 675000},
     {"Levite", "Levitess", 900000},
     {"Curate", "Curate", 1250000},
     {"Chaplain", "Chaplain", 1350000},
     {"Padre", "Matron", 1575000},
     {"Canon", "Canon", 1800000},
     {"Vicar", "Vicaress", 2025000},
     {"Deacon", "Deaconess", 2250000},
     {"Shaman", "Shamaness", 2475000},
     {"Speaker", "Speaker", 2700000},   /* 20 */
     {"Confessor", "Confessor", 3000000},
     {"Expositer", "Expositress", 3300000},
     {"Mystic", "Mystic", 3700000},
     {"Hermit", "Hermitess", 4100000},
     {"Faith Healer", "Faith Healer", 4600000},
     {"Healer", "Healer", 5100000},
     {"Lay Healer", "Lay Healer", 5600000},
     {"Illuminator", "Illuminatrix", 6100000},
     {"Evangelist", "Evangelist", 6600000},
     {"Voice", "Voice", 7200000},       /* 30 */
     {"Templar", "Templar", 7800000},
     {"Hospitalier", "Hospitalier", 8400000},
     {"Chuirgeon", "Chuirgeoness", 9000000},
     {"Chuirgeon General", "Chuirgeon General", 9700000},
     {"Inquisitor", "Inquisitrix", 10500000},
     {"Abbot", "Mother Superior", 11300000},
     {"Reverend", "Reverend Mother", 12200000},
     {"Bishop", "Bishop", 13200000},
     {"Arch Bishop", "Arch Bishop", 14300000},
     {"Cardinal", "Mother Superior", 15500000}, /* 40 */
     {"Patriarch", "Matriarch", 17500000},
     {"Grand Inquisitor", "Grand Inquisitrix", 20500000},
     {"Oracle", "Oracle", 24500000},
     {"Elder", "Elder", 28500000},
     {"Pastor", "Pastor", 33500000},
     {"Intercessor", "Intercessor", 38500000},
     {"Cleric", "Cleric", 44500000},
     {"High Cleric", "High Cleric", 50500000},
     {"Priest", "Priestess", 57500000},
     {"High Priest", "High Priestess", 65000000},       /* 50 */
     {"Immortal Cardinal", "Immortal Priestess", 200000000},
     {"Immortal Cardinal", "Immortal Priestess", 231000000},
     {"Immortal Cardinal", "Immortal Priestess", 232000000},
     {"Immortal Cardinal", "Immortal Priestess", 233000000},
     {"Immortal Cardinal", "Immortal Priestess", 234000000},
     {"Immortal Cardinal", "Immortal Priestess", 235000000},
     {"Inquisitor", "Inquisitress", 236000000},
     {"God", "Goddess", 237000000},
     {"Implementor", "Implementress", 238000000},
     {"Implementor", "Implementress", 239000000}},

    {{"Man", "Woman", 0},       /* Warrior */
     {"Swordpupil", "Swordpupil", 1},
     {"Recruit", "Recruit", 2000},
     {"Runner", "Runner", 4000},
     {"Sentry", "Sentry", 8000},
     {"Man-at-arms", "Woman-at-arms", 16000},
     {"Mercenary", "Mercenary", 32000},
     {"Scout", "Scout", 64000},
     {"Fighter", "Fighter", 125000},
     {"Armor Bearer", "Armor Bearer", 250000},
     {"Swordsman", "Swordswoman", 500000},      /* 10 */
     {"Fencer", "Fence", 750000},
     {"Combatant", "Combatrix", 1000000},
     {"Protector", "Protector", 1250000},
     {"Defender", "Defender", 1500000},
     {"Warder", "Warder", 1750000},
     {"Guardian", "Guardian", 2000000},
     {"Veteran", "Veteran", 2250000},
     {"Hero", "Heroine", 2500000},
     {"Swashbuckler", "Swashbuckler", 2750000},
     {"Myrmidon", "Amazon", 3000000},   /* 20 */
     {"Esquire", "Esquire", 3475000},
     {"Blademaster", "Blademistress", 3825000},
     {"Reeve", "Reeve", 4175000},
     {"Lieutenant", "Lieutenant", 4575000},
     {"Captain", "Captain", 4975000},
     {"Raider", "Raider", 5400000},
     {"Champion", "Lady Champion", 5900000},
     {"Dragoon", "Lady Dragoon", 6400000},
     {"Armiger", "Armigress", 6900000},
     {"Scrutifer", "Scrutifer", 7500000},       /* 30 */
     {"Lancer", "Lancer", 8100000},
     {"Banneret", "Banneret", 8700000},
     {"Chevalier", "Chevaliere", 9400000},
     {"Knight Errant", "Valkyrie", 10100000},
     {"General", "General", 10800000},
     {"Marshall", "Lady Marshall", 11500000},
     {"Keitar", "Lady Keitar", 12200000},
     {"Warrior", "Warrior", 12900000},
     {"Justiciar", "Justictrix", 13700000},
     {"Grand Marshall", "Grand Marshall", 14500000},    /* 40 */
     {"Knight", "Knight", 15500000},
     {"Lord Knight", "Lady Knight", 16500000},
     {"Grand Knight", "Grand Knight", 18000000},
     {"Wolf Warrior", "Wolf Warrior", 20000000},
     {"Bear Warrior", "Bear Warrior", 22000000},
     {"Lich Warrior", "Lich Warrior", 25000000},
     {"Dragon Warrior", "Dragon Warrior", 28000000},
     {"King's Defender", "Queen's Defender", 32000000},
     {"Lord Warrior", "Lady Warrior", 37000000},
     {"Warrior Lord", "Warrior Lady", 45000000},        /* 50 */
     {"Immortal", "Immortal", 200000000},
     {"Immortal", "Immortal", 231000000},
     {"Immortal", "Immortal", 232000000},
     {"Immortal", "Immortal", 233000000},
     {"Immortal", "Immortal", 234000000},
     {"Immortal", "Immortal", 235000000},
     {"Extirpator", "Queen", 236000000},
     {"God", "Goddess", 237000000},
     {"Implementor", "Implementrix", 238000000},
     {"Implementor", "Implementrix", 239000000}},

    {{"Man", "Woman", 0},       /* Thief */
     {"Delinquent", "Delinquent", 1},
     {"Miscreant", "Miscreant", 1250},
     {"Footpad", "Footpad", 2500},
     {"Pilferer", "Pilferess", 5000},
     {"Filcher", "Filcheress", 10000},
     {"Pincher", "Pincheress", 20000},
     {"Snatcher", "Snatcheress", 40000},
     {"Ninja", "Ninja", 70000},
     {"Pickpocket", "Pickpocket", 110000},
     {"Cut-Purse", "Cut-Purse", 160000},        /* 10 */
     {"Sharper", "Sharper", 220000},
     {"Burgler", "Burgler", 440000},
     {"Robber", "Robber", 660000},
     {"Mugger", "Mugger", 880000},
     {"Magsman", "Magswoman", 1100000},
     {"Bandito", "Bandita", 1320000},
     {"Highwayman", "Bandit", 1540000},
     {"Brigand", "Brigand", 1760000},
     {"Agent", "Agent", 1980000},
     {"Outlaw", "Outlaw", 2200000},     /* 20 */
     {"Blade", "Blade", 2600000},
     {"Quickblade", "Quickblade", 2900000},
     {"Knifer", "Knifer", 3200000},
     {"Sneak", "Sneak", 3500000},
     {"Thief", "Thief", 3800000},
     {"Special Agent", "Special Agent", 4150000},
     {"Collector", "Collector", 4500000},
     {"Hand", "Hand", 4850000},
     {"Unseen", "Unseen", 5200000},
     {"Thug", "Thug", 5550000}, /* 30 */
     {"Cut-Throat", "Cut-Throat", 5900000},
     {"Grand Thief", "Grand Thief", 6300000},
     {"Repossessor", "Repossessor", 6700000},
     {"Killer", "Killer", 7100000},
     {"Secret Agent", "Secret Agent", 7500000},
     {"Renegade", "Renegade", 7900000},
     {"Murderer", "Murderess", 8300000},
     {"Butcher", "Butcheress", 8750000},
     {"Slayer", "Slayer", 9200000},
     {"Executioner", "Executioner", 9650000},   /* 40 */
     {"Assassin", "Assassin", 10075000},
     {"Shadow Walker", "Shadow Walker", 10575000},
     {"Spy", "Spy", 11075000},
     {"Backstabber", "Backstabber", 11600000},
     {"Death Dancer", "Death Dancer", 12200000},
     {"Lock Master", "Lock Mistress", 12900000},
     {"Piercing Master", "Piercing Mistress", 13700000},
     {"Invisible Thief", "Invisible Thief", 14600000},
     {"Master Assassin", "Master Assassin", 17000000},
     {"Master Thief", "Master Thief", 20000000},        /* 50 */
     {"Immortal Assasin", "Immortal Assasin", 220000000},
     {"Immortal Assasin", "Immortal Assasin", 230000000},
     {"Immortal Assasin", "Immortal Assasin", 230500000},
     {"Immortal Assasin", "Immortal Assasin", 231000000},
     {"Immortal Assasin", "Immortal Assasin", 232000000},
     {"Immortal Assasin", "Immortal Assasin", 233000000},
     {" Demi God", "Demi Goddess", 241000000},
     {" God", "Goddess", 242000000},
     {"Implementor", "Implementrix", 243000000},
     {"Implementor", "Implementrix", 244000000}},

    {{"Man", " Woman", 0},
     {"Aspirant", "Aspirant", 1},       /* Druid */
     {"Ovate", "Ovate", 2000},
     {"Sprout", "Sprout", 4000},
     {"Courser", "Courser", 7500},
     {"Tracker", "Tracker", 12500},
     {"Guide", "Guide", 20000},
     {"Pathfinder", "Pathfinder", 35000},
     {"Keeper", "Keeper", 60000},
     {"Warder", "Warder", 90000},
     {"Planter", "Planter", 125000},    /* 10 */
     {"Watcher", "Watcher", 200000},
     {"Woodsman", "Woodsman", 300000},
     {"Pict", "Pict", 500000},
     {"Tree Hugger", "Tree Hugger", 750000},
     {"Celt", "Celt", 1000000},
     {"Animist", "Animist", 1350000},
     {"Hedge", "Hedge", 1700000},
     {"Tender", "Tender", 2100000},
     {"Strider", "Strider", 2500000},
     {"Druid", "Druidess", 3000000},    /* 20 */
     {"Aquarian", "Aquarian", 3500000},
     {"Arbolist", "Arbolist", 4000000},
     {"Dionysian", "Dionysian", 4500000},
     {"Herbalist", "Herbalist", 5000000},
     {"Naturalist", "Naturalist", 5500000},
     {"Silvian", "Silvian", 6000000},
     {"Forrestal", "Forrestal", 6500000},
     {"Forrestal", "Forrestal", 7000000},
     {"Ancient", "Ancient", 7500000},
     {"Guardian Druid", "Guardian Druidess", 8000000},  /* 30 */
     {"Protector Druid", "Protector Druidess", 8600000},
     {"Master of Fire", "Mistress of Fire", 9200000},
     {"Master of Stone", "Mistress of Stone", 9800000},
     {"Master of Air", "Mistress of Air", 10450000},
     {"Master of Water", "Mistress of Water", 11125000},
     {"Hierophant Initiate", "Hierophant Initiate", 11800000},
     {"Hierophant Druid", "Heirophant Druidess", 12475000},
     {"Hierophant Adept", "Heirophant Adept", 13175000},
     {"Heirophant Master", "Heirophant Master", 14000000},
     {"Numinous Hierophant", "Numinous Hierophant", 15000000},  /* 40 */
     {"Mystic Hierophant", "Mystic Hierophant", 16500000},
     {"Arcane Hierophant", "Arcane Hierophant", 18000000},
     {"Cabal Hierophant", "Cabal Hierophant", 19750000},
     {"Forest Druid", "Forest Druidess", 21500000},
     {"Ash Druid", "Ash Druidess", 23250000},
     {"Oak Druid", "Oak Druidess", 25120000},
     {"Elite Druid", "Elite Druidess", 27150000},
     {"Master Druid", "Master Druidess", 29500000},
     {"Champion Druid", "Champion Druidess", 35000000},
     {"Druid Lord", "Druidess Lady", 40000000}, /* 50 */
     {"Immortal Hierophant", "Immortal Hierophant", 201000000},
     {"Immortal Hierophant", "Immortal Hierophant", 236000000},
     {"Immortal Hierophant", "Immortal Hierophant", 237000000},
     {"Immortal Hierophant", "Immortal Hierophant", 238000000},
     {"Immortal Hierophant", "Immortal Hierophant", 239000000},
     {"Immortal Hierophant", "Immortal Hierophant", 240000000},
     {"Demi God", "Demi Goddess", 241000000},
     {"God", "Goddess", 242000000},
     {"Implementor", "Implementrix", 243000000},
     {"Implementor", "Implementrix", 244000000}
     },

    {{"Man", " Woman", 0},
     {"White Belt", "White Belt", 1},
     {"Initiate", "Initiate", 1000},
     {"Brother", "Sister", 2000},
     {"Layman", "Laywoman", 4250},
     {"Student", "Student", 8500},
     {"Practitioner", "Practitioner", 17000},
     {"Exponent", "Exponent", 34000},
     {"Adept", "Adept", 68000},
     {"Monk", "Monk", 98000},
     {"Shodan", "Shodan", 200000},      /* 10 */
     {"Shinobi", "Shinobi", 325000},
     {"Genin", "Kuniochi", 450000},
     {"Disciple", "Disciple", 575000},
     {"Chunin", "Chunin", 725000},
     {"Nidan", "Nidan", 925000},
     {"Expert", "Expert", 1200000},
     {"Jonin", "Jonin", 1500000},
     {"Hwarang", "Hwarang", 2100000},
     {"Sandan", "Sandan", 2500000},
     {"Sabom", "Sabom", 2900000},       /* 20 */
     {"Sensei", "Sensei", 3300000},
     {"Sifu", "Sifu", 3800000},
     {"Guru", "Guru", 4400000},
     {"Pendakar", "Pendakar", 5000000},
     {"Yodan", "Yodan", 5600000},
     {"Master", "Master", 6200000},
     {"Superior Master", "Superior Master", 6900000},
     {"Ginsu Master", "Ginsu Master", 7600000},
     {"Godan", "Godan", 8300000},
     {"Leopard Master", "Leopard Master", 8900000},     /* 30 */
     {"Tiger Master", "Tiger Master", 9700000},
     {"Snake Master", "Snake Master", 10500000},
     {"Crane Master", "Crane Master", 11300000},
     {"Dragon Master", "Dragon Master", 12100000},
     {"Rokudan", "Rokudan", 13000000},
     {"Master of Seasons", "Master of Seasons", 13900000},
     {"Master of the Winds", "Master of the Winds", 14800000},
     {"Master of Harmony", "Master of Harmony", 15700000},
     {"Shogun", "Shogun", 16700000},
     {"Shichidan", "Shichidan", 17700000},      /* 40 */
     {"Hachidan", "Hachidan", 18700000},
     {"Kudan", "Kudan", 20000000},
     {"Kai", "Kai", 22000000},
     {"Kai Lord", "Kai Lady", 23500000},
     {"Kai Master", "Kai Mistress", 25500000},
     {"Monk of Fists", "Monk of Fists", 27500000},
     {"Monk Graduate", "Monk Graduate", 30000000},
     {"Senior Monk", "Senior Monk", 37500000},
     {"Master Monk", "Mistress Monk", 45000000},
     {"Monk Grand Master", "Monk Grand Mistress", 55000000},    /* 50 */
     {"Immortal Grand Master", "Immortal Grand Master", 200000000},
     {"Immortal Grand Master", "Immortal Grand Master", 231000000},
     {"Immortal Grand Master", "Immortal Grand Master", 232000000},
     {"Immortal Grand Master", "Immortal Grand Master", 233000000},
     {"Immortal Grand Master", "Immortal Grand Master", 234000000},
     {"Immortal Grand Master", "Immortal Grand Master", 235000000},
     {"Demi God", "Demi Goddess", 236000000},
     {"God", "Goddess", 237000000},
     {"Implementor", "Implementrix", 238000000},
     {"Implementor", "Implementrix", 239000000}
     },

    {
     {"Man", "Woman", 0},
     {"Grunt", "Grunt", 1},
     {"Brawler", "Brawler", 6000},
     {"Smasher", "Smasher", 12000},
     {"Basher", "Basher", 24000},
     {"Horseman", "Horsewoman", 48000},
     {"Horsemaster", "Horsemaster", 80000},
     {"Competent", "Competent", 150000},
     {"Pillager", "Pillager", 275000},
     {"Raider", "Raider", 500000},
     {"Destroyer", "Destroyer", 1000000},       /* 10 */
     {"Berserker", "Berserker", 1500000},
     {"Raging Berserker", "Raging Berserker", 2000000},
     {"Sacker Of Villages", "Sacker Of Villages", 2750000},
     {"Sacker Of Towns", "Sacker Of Towns", 3550000},
     {"Sacker Of Cities", "Sacker Of Cities", 4000000},
     {"Chieftain", "Chieftess", 4350000},
     {"Hordling", "Hordling", 4750000},
     {"Hordesman", "Hordeswoman", 5000000},
     {"Hordesmaster", "Hordesmaster", 5350000},
     {"Brute", "Brute", 5750000},       /* 20 */
     {"Marauder", "Marauder", 6000000},
     {"Marauder", "Marauder", 6350000},
     {"Savage Warrior", "Savage Warrior", 6750000},
     {"Savage Warrior", "Savage Warrior", 7000000},
     {"Rugged Blade", "Rugged Blade", 7350000},
     {"Rugged Blade", "Rugged Blade", 7750000},
     {"Wild Man", "Wild Woman", 8250000},
     {"Fierce Crier", "Fierce Crier", 9000000},
     {"Stoney Face", "Stoney Face", 9750000},
     {"Frenzy Fighter", "Frenzy Fighter", 10500000},    /* 30 */
     {"Frenzy Fighter", "Frenzy Fighter", 11250000},
     {"Harsh Fighter", "Harsh Fighter", 12000000},
     {"Harsh Fighter", "Harsh Fighter", 12750000},
     {"Storming Brute", "Storming Brute", 13500000},
     {"Formidable Foe", "Formidable Foe", 14250000},
     {"Heavy Hitter", "Heavy Hitter", 15250000},
     {"Hammer Hands", "Hammer Hands", 16250000},
     {"Barbarian", "Barbarian", 17250000},
     {"Strong Barbarian", "Strong Barbarian", 18500000},
     {"Burly Barbarian", "Burly Barbarian", 20000000},  /* 40 */
     {"Frenzied Barbarian", "Frenzied Barbarian", 22000000},
     {"Conquering Barbarian", "Conquering Barbarian", 24000000},
     {"Senior Barbarian", "Senior Barbarian", 26500000},
     {"Master Barbarian", "Master Barbarian", 29000000},
     {"Elite Barbarian", "Elite Barbarian", 32000000},
     {"Great Barbarian", "Great Barbarian", 35000000},
     {"Grand Barbarian", "Grand Barbarian", 40000000},
     {"Chief Barbarian", "Chieftess Barbarian", 45000000},
     {"Conan", "Sonja", 50000000},
     {"Khan", "Hun", 60000000}, /* 50 */
     {"Immortal", "Immortal", 200555000},
     {"Immortal", "Immortal", 231500000},
     {"Immortal", "Immortal", 233000000},
     {"Immortal", "Immortal", 234500000},
     {"Immortal", "Immortal", 235500000},
     {"Immortal", "Immortal", 236500000},
     {"Extirpator", "Queen", 237500000},
     {"God", "Goddess", 238500000},
     {"Implementor", "Implementrix", 239500000},
     {"Implementor", "Implementrix", 241000000}
     },
    /*
     * sorcerer
     */
    {
     {"Man", "Woman", 0},
     {"SpellWeaver", "SpellWeaver", 1},
     {"Bookworm", "Bookworm", 2500},
     {"BookStudent", "BookStudent", 5000},
     {"Scholar", "Scholar", 10000},
     {"Soothsayer", "Soothsayer", 20000},
     {"Illusionist", "Illusionist", 40000},
     {"Warlock", "Witch", 60000},
     {"Apparitionist", "Apparitionist", 90000},
     {"Medium", "Gypsy", 135000},
     {"Scribe", "Scribe", 250000},      /* 10 */
     {"Sage", "Sage", 375000},
     {"Seer", "Seeress", 750000},
     {"Divinater", "Divinatress", 1125000},
     {"Alchemist", "Alchemist", 1500000},
     {"Evoker", "Evoker", 1875000},
     {"Necromancer", "Necromancer", 2250000},
     {"Abjurer", "Abjuress", 2625000},
     {"Invoker", "Invoker", 3000000},
     {"Enchanter", "Enchantress", 3375000},
     {"Conjurer", "Conjuress", 3750000},        /* 20 */
     {"Summoner", "Summoner", 4150000},
     {"Magician", "Magician", 4575000},
     {"Spiritualist", "Spiritualist", 5050000},
     {"Savant", "Savanti", 5500000},
     {"Shaman", "Shamaness", 6250000},
     {"Mystic", "Mystic", 7000000},
     {"Wiccen", "Wicca", 7750000},
     {"Mentalist", "Mentalist", 8550000},
     {"Mnemonicist", "Mnemonicist", 9350000},
     {"Nueromancer", "Nueromancer", 10200000},  /* 30 */
     {"Spellbinder", "Spellbinder", 11200000},
     {"Synergist", "Synergist", 12200000},
     {"Oculist", "Oculist", 13200000},
     {"Thaumaturgist", "Thaumaturgist", 14500000},
     {"SpellMaster", "Spell Mistress", 16000000},
     {"Dispeller", "Dispeller", 18500000},
     {"Warlock", "War Witch", 20000000},
     {"Sorcerer", "Sorceress", 23000000},
     {"Wizard", "Wizardess", 26000000},
     {"High Wizard", "High Wizardess", 30000000},       /* 40 */
     {"Magus", "Incantrix", 34000000},
     {"Serpent Mage", "Serpent Mage", 38000000},
     {"Wind Master", "Wind Mistress", 42000000},
     {"Water Master", "Water Mistress", 46000000},
     {"Earth Master", "Earth Mistress", 51000000},
     {"Fire Master", "Fire Mistress", 56000000},
     {"Ice Master", "Ice Mistress", 61000000},
     {"Grand Wizard", "Grand Wizardess", 66000000},
     {"Spell Master", "Spell Mistress", 72000000},
     {"High Sorcerer", "High Sorceress", 80000000},     /* 50 */
     {"Immortal Warlock", "Immortal Enchantress", 200000000},
     {"Immortal Warlock", "Immortal Enchantress", 231000000},
     {"Immortal Warlock", "Immortal Enchantress", 232000000},
     {"Immortal Warlock", "Immortal Enchantress", 233000000},
     {"Immortal Warlock", "Immortal Enchantress", 234000000},
     {"Immortal Warlock", "Immortal Enchantress", 235000000},
     {"Avatar of Magic", "Empress of Magic", 236000000},
     {"God of magic", "Goddess of magic", 237000000},
     {"Implementor", "Implementrix", 238000000},
     {"Implementor", "Implementrix", 239000000} /* 60 */
     },

    /*
     * paladin
     */
    {
     {"Man", "Woman", 0},
     {"Aspirant", "Aspirant", 1},
     {"Aspirant", "Aspirant", 2250},
     {"Apprentice", "Apprentice", 4500},
     {"Blade Bearer", "Blade Bearer", 9000},
     {"Holy Page", "Holy Page", 18000},
     {"Holy Page", "Holy Page", 36000},
     {"Holy Squire", "Holy Squire", 75000},
     {"Holy Squire", "Holy Squire", 150000},
     {"Holy Footman", "Holy Footman", 300000},
     {"Holy Footman", "Holy Footman", 600000},  /* 10 */
     {"Holy Warrior", "Holy Warrior", 900000},
     {"Holy Warrior", "Holy Warrior", 1200000},
     {"Templar Acolyte", "Templar Acolyte", 1500000},
     {"Templar Acolyte", "Templar Acolyte", 1800000},
     {"Junior Templar", "Junior Templar", 2100000},
     {"Junior Templar", "Junior Templar", 2400000},
     {"Holy Templar", "Holy Templar", 2700000},
     {"Holy Templar", "Holy Templar", 3000000},
     {"Templar Knight", "Templar Knight", 3300000},
     {"Templar Knight", "Templar Knight", 3600000},     /* 20 */
     {"Holy Knight", "Holy Knight", 3900000},
     {"Holy Knight", "Holy Knight", 4000000},
     {"Crusader", "Crusader", 4300000},
     {"Crusader", "Crusader", 4600000},
     {"Avenger", "Avenger", 5000000},
     {"Avenger", "Avenger", 5400000},
     {"Defender", "Defender", 5700000},
     {"Defender", "Defender", 6000000},
     {"Guardian", "Guardian", 6500000},
     {"Guardian", "Guardian", 7500000}, /* 30 */
     {"Protector", "Protector", 8500000},
     {"Protector", "Protector", 9000000},
     {"Pure Knight", "Pure Knight", 9500000},
     {"Pure Knight", "Pure Knight", 10750000},
     {"Knight of the Cross", "Knight of the Cross", 12000000},
     {"Knight of the Cross", "Knight of the Cross", 13250000},
     {"Knight of the Sword", "Knight of the Sword", 15000000},
     {"Knight of the Crown", "Knight of the Crown", 16750000},
     {"Knight of the Crown", "Knight of the Crown", 18500000},
     {"Knight of the Rose", "Knight of the Rose", 20250000},    /* 40 */
     {"Queen's Knight", "Queen's Knight", 23500000},
     {"King's Knight", "King's Knight", 27000000},
     {"Paladin", "Paladin", 30500000},
     {"Paladin Templar", "Paladin Templar", 35000000},
     {"Paladin Knight", "Paladin Knight", 40000000},
     {"Master Paladin", "Master Paladin", 45000000},
     {"Senior Paladin", "Senior Paladin", 50000000},
     {"High Paladin", "High Paladin", 55000000},
     {"Champion Paladin", "Champion Paladin", 60000000},
     {"Lord High Paladin", "Lady High Paladin", 70000000},      /* 50 */
     {"Immortal Paladin", "Immortal Paladin", 200000000},
     {"Immortal Paladin", "Immortal Paladin", 231000000},
     {"Immortal Paladin", "Immortal Paladin", 232000000},
     {"Immortal Paladin", "Immortal Paladin", 233000000},
     {"Immortal Paladin", "Immortal Paladin", 234000000},
     {"Immortal Paladin", "Immortal Paladin", 235000000},
     {"Immortal Paladin", "Immortal Paladin", 236000000},
     {"Immortal Paladin", "Immortal Paladin", 237000000},
     {"Immortal Paladin", "Immortal Paladin", 238000000},
     {"Immortal Paladin", "Immortal Paladin", 239000000}        /* 60 */
     },

    /*
     * ranger
     */
    {
     {"Man", "Woman", 0},
     {"Woodsman", "Woodswoman", 1},
     {"Brother", "Sister", 2500},
     {"Woodland Brother", "Woodland Sister", 4500},
     {"Forest Watcher", "Forest Watcher", 9000},
     {"Forest Protector", "Forest Protector", 18000},
     {"Forest Guardian", "Forest Guardian", 36000},
     {"Pathseeker", "Pathseeker", 75000},
     {"Pathfinder", "Pathfinder", 150000},
     {"Pathmaker", "Pathmaker", 300000},
     {"Ranger", "Ranger", 600000},      /* 10 */
     {"Wanderer", "Wanderer", 900000},
     {"Forester", "Forester", 1200000},
     {"Gatherer", "Gatherer", 1500000},
     {"Guardian", "Guardian", 1800000},
     {"Searcher", "Searcher", 2100000},
     {"Freeman", "Freewoman", 2400000},
     {"Trapper", "Trapper", 2700000},
     {"Huntsman", "Huntswoman", 3000000},
     {"Frontiersman", "Frontierswoman", 3300000},
     {"Forest Brother", "Forest Sister", 3600000},      /* 20 */
     {"Bounty Hunter", "Bounty Hunter", 3900000},
     {"Animal Tamer", "Animal Tamer", 4000000},
     {"Scout", "Scout", 4300000},
     {"Explorer", "Explorer", 4600000},
     {"Archer", "Archer", 5000000},
     {"Walker", "Walker", 5400000},
     {"Strider", "Strider", 5700000},
     {"Bushman", "Bushwoman", 6000000},
     {"Runner", "Runner", 6500000},
     {"Tracker", "Tracker", 7750000},   /* 30 */
     {"Courser", "Courser", 8500000},
     {"Guide", "Guide", 9000000},
     {"Woodland Tracker", "Woodland Tracker", 9500000},
     {"Senior Tracker", "Senior Tracker", 10750000},
     {"Forest Tracker", "Forest Tracker", 12000000},
     {"Master Tracker", "Master Tracker", 13250000},
     {"Trailblazer", "Trailblazer", 15000000},
     {"Raven", "Raven", 16750000},
     {"Fox", "Fox", 18500000},
     {"Wolf", "Wolf", 20250000},        /* 40 */
     {"Bear", "Bear", 23500000},
     {"Kestrel", "Kestrel", 27000000},
     {"Hawk", "Hawk", 30500000},
     {"Eagle", "Eagle", 35000000},
     {"Animal Friend", "Animal Friend", 40000000},
     {"Forest Friend", "Forest Friend", 45000000},
     {"Forest Protector", "Forest Protector", 50000000},
     {"Forest Master", "Forest Mistress", 55000000},
     {"Forest Ranger", "Forest Ranger", 60000000},
     {"Mountain Ranger", "Mountain Ranger", 70000000},  /* 50 */
     {"Immortal Ranger", "Immortal Ranger", 200000000},
     {"Immortal Ranger", "Immortal Ranger", 231000000},
     {"Immortal Ranger", "Immortal Ranger", 232000000},
     {"Immortal Ranger", "Immortal Ranger", 233000000},
     {"Immortal Ranger", "Immortal Ranger", 234000000},
     {"Immortal Ranger", "Immortal Ranger", 235000000},
     {"Immortal Ranger", "Immortal Ranger", 236000000},
     {"Immortal Ranger", "Immortal Ranger", 237000000},
     {"Immortal Ranger", "Immortal Ranger", 238000000},
     {"Immortal Ranger", "Immortal Ranger", 239000000}  /* 60 */
     },
    /*
     * psionist
     */
    {
     {"Man", "Woman", 0},
     {"Brain", "Brain", 1},
     {"Seeker", "Seeker", 2200},
     {"Finder", "Finder", 4400},
     {"Psycho", "Psycho", 8800},
     {"Psychic", "Psychic", 16500},
     {"Telepath", "Telepath", 30000},
     {"Controller", "Controller", 55000},
     {"Mind Bender", "Mind Bender", 100000},
     {"Palm Reader", "Palm Reader", 200000},
     {"Fortune Teller", "Fortune Teller", 400000},      /* 10 */
     {"Sensor", "Sensor", 600000},
     {"Divinator", "Divinator", 800000},
     {"Clairsentient", "Clairsentient", 1000000},
     {"Firestarter", "Firestarter", 1200000},
     {"Metapsionic", "Metapsionic", 1500000},
     {"Psionic Student", "Psionic Student", 1800000},
     {"Psionic Tutor", "Psionic Tutor", 2100000},
     {"Psionic Instructor", "Psionic Instructor", 2400000},
     {"Psionic Teacher", "Psionic Teacher", 2700000},
     {"Empath", "Empath", 3000000},     /* 20 */
     {"Ego Student", "Ego Student", 3300000},
     {"Ego Controller", "Ego Controller", 3750000},
     {"Ego Warrior", "Ego Warrior", 4200000},
     {"Intellectual", "Intellectual", 4650000},
     {"Intellectual", "Intellectual", 5150000},
     {"Will of Brass", "Will of Brass", 5650000},
     {"Will of Iron", "Will of Iron", 6250000},
     {"Will of Steel", "Will of Steel", 7000000},
     {"Mental Shield", "Mental Shield", 7750000},
     {"Ambassador", "Ambassador", 8500000},     /* 30 */
     {"Ambassador", "Ambassador", 9300000},
     {"Ascetic", "Ascetic", 10000000},
     {"Ascetic", "Ascetic", 10900000},
     {"Animal Talker", "Animal Talker", 11700000},
     {"Animal Tamer", "Animal Tamer", 12600000},
     {"Beast Master", "Beast Master", 13500000},
     {"Beast Master", "Beast Master", 14500000},
     {"Cerebral Warrior", "Cerebral Warrior", 15500000},
     {"Cerebral Knight", "Cerebral Knight", 16500000},
     {"Cerebral Gladiator", "Cerebral Gladiator", 17500000},    /* 40 */
     {"Dreamer", "Dreamer", 18500000},
     {"Dream Master", "Dream Master", 20000000},
     {"Mystic", "Mystic", 22000000},
     {"Ethereal Mystic", "Ethereal Mystic", 25000000},
     {"Hermit", "Hermitess", 29000000},
     {"Psionist", "Psionist", 33000000},
     {"Ego Psionist", "Ego Psionist", 37000000},
     {"Disciplined Psionist", "Disciplined Psionist", 41000000},
     {"High Psionist", "High Psionist", 45000000},
     {"Psionist Master", "Psionist Mistress", 55000000},        /* 50 */
     {"Immortal Psionist", "Immortal Psionist", 200000000},
     {"Immortal Psionist", "Immortal Psionist", 231000000},
     {"Immortal Psionist", "Immortal Psionist", 232000000},
     {"Immortal Psionist", "Immortal Psionist", 233000000},
     {"Immortal Psionist", "Immortal Psionist", 234000000},
     {"Immortal Psionist", "Immortal Psionist", 235000000},
     {"Immortal Psionist", "Immortal Psionist", 236000000},
     {"Immortal Psionist", "Immortal Psionist", 237000000},
     {"Immortal Psionist", "Immortal Psionist", 238000000},
     {"Immortal Psionist", "Immortal Psionist", 239000000}      /* 60 */
     },

#if 1
    {{"Man", "Woman", 0},       /* Necromancer */
     {"Grave Digger", "Grave Digger", 1},       /* */
     {"Grave Robber", "Grave Robber", 2500},    /* */
     {"Balmer", "Balmer", 5000},
     {"Undertaker", "Undertaker", 10000},
     {"Summoner", "Summoner", 20000},
     {"Life Tap", "Life Tap", 40000},
     {"Acolyte", "Acolyte", 60000},
     {"Occultist", "Occultist", 90000},
     {"Heretic", "Heretic", 135000},
     {"Bone Mender", "Bone Mender", 250000},    /* 10 */
     {"Minion", "Minion", 375000},
     {"Corruptor", "Corruptor", 750000},
     {"Bat", "Bat", 1250000},
     {"Defiler", "Defiler", 1500000},
     {"Chosen", "Chosen", 1875000},
     {"Binder", "Binder", 2250000},
     {"Enslaver", "Enslaver", 2625000},
     {"Reanimator", "Reanimator", 3000000},
     {"Deathbringer", "Deathbringer", 3375000},
     {"Servant of Chaos", "Servant of Chaos", 3750000}, /* 20 */
     {"Shadow Collector", "Shadow Collector", 4150000},
     {"Undead Raiser", "Undead Raiser", 4575000},
     {"Acolyte of Evil", "Acolyte of Evil", 5050000},
     {"Acolyte of Chaos", "Acolyte of Chaos", 5500000},
     {"Acolyte of Death", "Acolyte of Death", 6250000},
     {"Minor Invoker", "Minor Invoker", 7000000},
     {"Soul Gatherer", "Soul Gatherer", 7750000},
     {"Soul Stealer", "Soul Stealer", 8550000},
     {"Soul Possessor", "Soul Possessor", 9350000},
     {"Major Invoker", "Major Invoker", 10200000},      /* 30 */
     {"Death Eater", "Death Eater", 11200000},
     {"Death Stalker", "Death Stalker", 12200000},
     {"Corpse Tracer", "Corpse Tracer", 13200000},
     {"Herald of Decay", "Herald of Decay", 14500000},
     {"Gatherer of Shadows", "Gatherer of Shadows", 16000000},
     {"Dark Warlock", "Dark Warwitch", 18500000},
     {"Vampiric Adept", "Vampiric Adept", 20000000},
     {"Herald of Darkness", "Herald of Darkness", 23000000},
     {"Shadow Walker", "Shadow Walker", 26000000},      /* 40 */
     {"Necromancer", "Necromancer", 30000000},
     {"Vampiric Master", "Vampiric Mistress", 34000000},
     {"Rune Caster", "Rune Caster", 38000000},
     {"Master Reanimator", "Mistress Reanimator", 42000000},
     {"Spreader of Disease", "Spreader of Disease", 46000000},
     {"Spreader of Decay", "Spreader of Decay", 51000000},
     {"Brace of Death", "Brace of Death", 56000000},
     {"Harbringer of Evil", "Harbringer of Evil", 61000000},
     {"Master of Binding", "Mistress of Binding", 66000000},
     {"Lord of Liches", "Lady of Liches", 72000000},
     {"Death Mage", "Death Magess", 80000000},  /* 50 */
     {"Immortal Necromancer", "Immortal Necromancer", 200000000},
     {"Immortal Necromancer", "Immortal Necromancer", 231000000},
     {"Immortal Necromancer", "Immortal Necromancer", 232000000},
     {"Immortal Necromancer", "Immortal Necromancer", 233000000},
     {"Immortal Necromancer", "Immortal Necromancer", 234000000},
     {"Immortal Necromancer", "Immortal Necromancer", 235000000},
     {"Immortal Necromancer", "Immortal Necromancer", 236000000},
     {"Immortal Necromancer", "Immortal Necromancer", 237000000},
     {"Implementor", "Implementrix", 238000000},
     {"Implementor", "Implementrix", 239000000} /* 60 */
     }

#endif

};

const char     *RaceName[] = {
    "Half-Breed",
    "Human",
    "Moon-Elf",
    "Dwarven",
    "Halfling",
    "Rock Gnome",
    "Reptilian",
    "Mysterion",
    "Lycanthropian",
    "Draconian",
    "Undead",
    "Orcish",
    "Insectoid",
    "Arachnoid",
    "Saurian",
    "Icthyiod",
    "Avian",
    "Giant",
    "Carnivororous",
    "Parasitic",
    "Slime",
    "Demonic",
    "Snake",
    "Herbivorous",
    "Tree",
    "Vegan",
    "Elemental",
    "Planar",
    "Diabolic",
    "Ghostly",
    "Goblinoid",
    "Trollish",
    "Vegman",
    "Mindflayer",
    "Primate",
    "Enfan",
    "Dark-Elf",
    "Golem",
    "Skexie",
    "Troglodyte",
    "Patryn",
    "Labrynthian",
    "Sartan",
    "Tytan",
    "Smurf",
    "Kangaroo",
    "Horse",
    "Ratperson",
    "Astralion",
    "God",
    "Hill Giant",
    "Frost Giant",
    "Fire Giant",
    "Cloud Giant",
    "Storm Giant",
    "Stone Giant",
    "Red Dragon",
    "Black Dragon",
    "Green Dragon",
    "White Dragon",
    "Blue Dragon",
    "Silver Dragon",
    "Gold Dragon",
    "Bronze Dragon",
    "Copper Dragon",
    "Brass Dragon",
    "Undead Vampire",
    "Undead Lich",
    "Undead Wight",
    "Undead Ghast",
    "Undead Spectre",
    "Undead Zombie",
    "Undead Skeleton",
    "Undead Ghoul",
    "Half-Elf",
    "Half-Ogre",
    "Half-Orc",
    "Half-Giant",
    "Lizardman",
    "Dark-Dwarf",
    "Deep-Gnome",
    "Gnoll",
    "Gold-Elf",
    "Wild-Elf",
    "Sea-Elf",
    "Forest-Gnome",
    "Avariel",
    "Ettin",
    "Cockatrice",
    "Cocodile",
    "Basilisk",
    "Gargoyle",
    "Drider",
    "Displacer Beast",
    "Griffon",
    "Hell Hound",
    "Hydra",
    "Margoyle",
    "Mastodon",
    "Medusa",
    "Minotaur",
    "Mobat",
    "Otyugh",
    "Neo-Otyugh",
    "Remorhaz",
    "Purple Worm",
    "Phase Spider",
    "Sphinx",
    "Warg",
    "Wyvern",
    "Umber Hulk",
    "Slug",
    "Bulette",
    "Carrion Crawler",
    "Dracolisk",
    "Banshee",
    "Beholder",
    "Death Tyrant",
    "Dracolich",
    "Rakshasa",
    "Bane Minion",
    "Ankhegs",
    "Bugbears",
    "Efreet",
    "Harpies",
    "Manticores",
    "Scrags",
    "Owlbears",
    "Squids",
    "Tiger",
    "Wights",
    "Yeti",
    "\n"
};

const char     *item_types[] = {
    "UNDEFINED",                /* 0 */
    "LIGHT",
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",                   /* 5 */
    "FIRE WEAPON",
    "MISSILE",
    "TREASURE",
    "ARMOR",
    "POTION",                   /* 10 */
    "WORN",
    "OTHER",
    "TRASH",
    "TRAP",
    "CONTAINER",                /* 15 */
    "NOTE",
    "LIQUID CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",                    /* 20 */
    "PEN",
    "BOAT",
    "AUDIO",
    "BOARD",
    "TREE",                     /* 25 */
    "ROCK",
    "PORTAL",                   /* 27 */
    "INSTRUMENT",
    "SHIPS HELM",
    "\n"
};

const char     *wear_bits[] = {
    "TAKE",
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAIST",
    "WRIST",
    "WIELD",
    "HOLD",
    "THROW",
    "LIGHT-SOURCE",
    "BACK",
    "EARS",
    "EYES",
    "\n"
};

const char     *extra_bits[] = {
    "GLOW",
    "HUM",
    "METAL",
    "MINERAL",
    "ORGANIC",
    "INVISIBLE",
    "MAGIC",
    "NODROP",
    "ANTI-NECROMANCER",
    "ANTI-GOOD",
    "ANTI-EVIL",
    "ANTI-NEUTRAL",
    "ANTI-CLERIC",
    "ANTI-MAGE",
    "ANTI-THIEF",
    "ANTI-WARRIOR",
    "BRITTLE",
    "RESISTANT",
    "ARTIFACT",
    "ANTI-MEN",
    "ANTI-WOMEN",
    "ANTI-SUN",
    "ANTI-BARBARIAN",
    "ANTI-RANGER",
    "ANTI-PALADIN",
    "ANTI-PSIONIST",
    "ANTI-MONK",
    "ANTI-DRUID",
    "ONLY-CLASS",
    "unused",
    "RARE",
    "QUEST",
    "UNIQUE",
    "\n"
};

const char     *room_bits[] = {
    "DARK",
    "DEATH",
    "NO_MOB",
    "INDOORS",
    "PEACEFUL",
    "NOSTEAL",
    "NO_SUM",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "SILENCE",
    "LARGE",
    "NO_DEATH",
    "SAVE_ROOM",
    "ARENA_ROOM",
    "NO_FLY",
    "REGEN_ROOM",
    "FIRE_ROOM",
    "ICE_ROOM",
    "WIND_ROOM",
    "EARTH_ROOM",
    "ELECTRIC_ROOM",
    "WATER_ROOM",
    "MOVE_ROOM",
    "MANA_ROOM",
    "NO_FLEE",
    "NO_SPY",
    "EVER_LIGHT",
    "ROOM_MAP",
    "\n"
};

const char     *exit_bits[] = {
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "SECRET",
    "RSLOCKED",
    "PICKPROOF",
    "CLIMB",
    "\n"
};

const char     *sector_types[] = {
    "Inside",
    "City",
    "Field",
    "Forest",
    "Hills",
    "Mountains",
    "Water Swim",
    "Water NoSwim",
    "Air",
    "Underwater",
    "Desert",
    "Tree",
    "Sea",
    "Blank",
    "Rocky Mountain",
    "Snowy Mountain",
    "Ruins",
    "Jungle",
    "Swamp",
    "Lava",
    "Entrance",
    "Farm",
    "Empty",
    "\n"
};

const char     *sector_char[] = {
    "$c000W%",
    "$c000W#",
    "$c000G\\",
    "$c000g+",
    "$c000G^",
    "$c000y^",
    "$c000B:",
    "$c000b:",
    "$c000C%",
    "$c000B~",
    "$c000w.",
    "$c000G+",
    "$c000B~",
    " ",
    "$c000X^",
    "$c000W^",
    "$c000X#",
    "$c000g&",
    "$c000X%",
    "$c000r\\",
    "$c000W@",
    "$c000y%",
    "?",
    "\n"
};

const char     *equipment_types[] = {
    "Special",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waist",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "Worn on back",
    "Worn in right ear",
    "Worn in left ear",
    "Worn on face",
    "Load in weapon",
    "\n"
};

const char     *affected_bits[] = { "Blind",
    "Invisible",
    "Detect Evil",
    "Detect Invisible",
    "Detect Magic",
    "Sense Life",
    "Hold",
    "Sanctuary",
    "Dragon Ride",
    "Growth",
    "Curse",
    "Flying",
    "Poison",
    "Tree Travel",
    "Paralysis",
    "Infravision",
    "Water Breath",
    "Sleep",
    "Travelling",
    "Sneak",
    "Hide",
    "Silence",
    "Charm",
    "Darkness",
    "Protect from Evil",
    "True Sight",
    "Scrying",
    "Fireshield",
    "Group",
    "Telepathy",
    "Chillshield",
    "Blade barrier",
    "\n"
};

const char     *affected_bits2[] = {
    "Invisible to animals",
    "Heat Stuff",
    "Logging",
    "Berserk",
    "Group-Order",
    "AWAY-FROM-KEYBOARD",
    "Detect Good",
    "Protection from Good",
    "Follow",
    "Haste",
    "Slow",
    "Wings burned",
    "Style Berserked",
    "Quest",
    "No Outdoor Logs",
    "Wings tired",
    "Invis to Undead",
    "Guardian Angel",
    "Song of the Wanderer",
    "Sneak Skill",
    "\n"
};

const char     *special_user_flags[] = {
    "NO-DELETE",
    "USE-ANSI",
    "RACE-WARRIOR",
    "Undef-2",
    "SHOW-EXITS",
    "MURDER-1",
    "STOLE-1",
    "MURDER-2",
    "STOLE-2",
    "MURDER-X",
    "STOLE-X",
    "PAGE-PAUSE",
    "OBJECT-EDITOR-OK",
    "MOBILE-EDITOR-OK",
    "FAST-EDIT",
    "PKILLING",
    "CLOAKED",
    "ZONE-SOUNDS",
    "CHAR-PRIVATE",
    "CLAN-LEADER",
    "NEW-USER",
    "HERO-STATUS",
    "GHOST",
    "FAST-MAP",
    "\n"
};
const char     *immunity_names[] = { "Fire",
    "Cold",
    "Electricity",
    "Energy",
    "Blunt",
    "Pierce",
    "Slash",
    "Acid",
    "Poison",
    "Drain",
    "Sleep",
    "Charm",
    "Hold",
    "Non-Magic",
    "+1",
    "+2",
    "+3",
    "+4",
    "\n"
};

const char     *apply_types[] = {
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "CHR",
    "SEX",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MANA",
    "HIT",
    "MOVE",
    "GOLD",
    "SPELL AFF2",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "SAVING_ALL",
    "RESISTANCE",
    "SUSCEPTIBILITY",
    "IMMUNITY",
    "SPELL AFFECT",
    "WEAPON SPELL",
    "EAT SPELL",
    "BACKSTAB",
    "KICK",
    "SNEAK",
    "HIDE",
    "BASH",
    "PICK",
    "STEAL",
    "TRACK",
    "HIT-N-DAM",
    "SPELLFAIL",
    "ATTACKS",
    "HASTE",
    "SLOW",
    "OTHER",
    "FIND-TRAPS",
    "RIDE",
    "RACE-SLAYER",
    "ALIGN-SLAYER",
    "MANA-REGEN",
    "HIT-REGEN",
    "MOVE-REGEN",
    "MOD-THIRST",
    "MOD-HUNGER",
    "MOD-DRUNK",
    "T_STR",
    "T_INT",
    "T_DEX",
    "T_WIS",
    "T_CON",
    "T_CHR",
    "T_HPS",
    "T_MOVE",
    "T_MANA",
    "\n"
};

const char     *pc_class_types[] = {
    "(Magic User)",
    "(Cleric)",
    "(Warrior)",
    "(Thief)",
    "(Druid)",
    "(Monk)",
    "(Barbarian)",
    "(Sorcerer)",
    "(Paladin)",
    "(Ranger)",
    "(Psionist)",
    "(Necromancer)",
    "(empty)",
    "\n"
};

const char     *class_names[] = {
    "Magic User",
    "Cleric",
    "Warrior",
    "Thief",
    "Druid",
    "Monk",
    "Barbarian",
    "Sorcerer",
    "Paladin",
    "Ranger",
    "Psionist",
    "Necromancer",
    "empty",
    "\n"
};

const char     *npc_class_types[] = {
    "Normal",
    "Undead",
    "\n"
};

const char     *system_flag_types[] = {
    "NO-PORTAL",
    "NO-ASTRAL",
    "NO-SUMMON",
    "NO-KILL",
    "LOG-ALL",
    "ECLIPSE",
    "NO-DNS",
    "REQ-APPROVAL",
    "NO-COLOR",
    "WIZLOCKED",
    "NO_POLY",
    "NO-OOC",
    "NO-LOCOBJ",
    "WLD-ARENA",
    "NO-DEINIT",
    "NO-TWEAK",
    "ZONE-LOCATE",
    "\n"
};

const char     *action_bits[] = {
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "NICE-THIEF",
    "AGGRESSIVE",
    "STAY-ZONE",
    "WIMPY",
    "ANNOYING",
    "HATEFUL",
    "AFRAID",
    "IMMORTAL",
    "HUNTING",
    "DEADLY",
    "POLYMORPHED",
    "META_AGGRESSIVE",
    "GUARDING",
    "NECROMANCER",
    "HUGE",
    "SCRIPT",
    "GREET",
    "MAGIC-USER",
    "WARRIOR",
    "CLERIC",
    "THIEF",
    "DRUID",
    "MONK",
    "BARBARIAN",
    "PALADIN",
    "RANGER",
    "PSIONIST",
    "\n"
};

const char     *procedure_bits[] = {
    "None",
    "Shopkeeper",
    "Guildmaster",
    "Swallower",
    "Drainer",
    "Quest",
    "Old Breath Weapon",
    "Fire Breather",
    "Gas Breather",
    "Frost Breather",
    "Acid Breather",
    "Electric Breather",
    "Dehydration Breather",
    "Vapor Breather",
    "Sound Breather",
    "Shard Breather",
    "Sleep Breather",
    "Light Breather",
    "Dark Breather",
    "Receptionist",
    "Repair Guy",

    "\n"
};

const char     *player_bits[] = {
    "Brief",
    "NoShout",
    "Compact",
    "DontSet",
    "Wimpy",
    "NoHassle",
    "Stealth",
    "Hunting",
    "Deaf",
    "Echo",
    "NoYell",
    "NoAuction",
    "NONE",                     /* Old NODIMD flag it seems */
    "NoOoc",
    "NoShout",
    "Freeze",
    "NoTell",
    "NoFly",
    "WizNoOoc",                 /* Doesn't seem to work TEB */
    "WizReport",
    "NoBeep",
    "HaveRoom",
    "Mailing",
    "NoOutdoor",
    "Autoassist",
    "Autoexit",
    "Autoloot",
    "Autosac",
    "Autogold",
    "Autosplit",
    "ClanLeader",
    "Legend",
    "Empty12",
    "\n"
};

const char     *position_types[] = {
    "Dead",
    "Mortally wounded",
    "Incapacitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "\n"
};

const char     *fight_styles[] = {
    "Standard",
    "Berserked",
    "Aggressive",
    "Defensive",
    "Evasive",
    "\n"
};

const char     *tfd_types[] = {
    "None Selected",
    "News",
    "Message of the Day",
    "Wizard's Mesasge of the Day",
    "\n"
};

const char     *connected_types[] = {
    "Playing",
    "Get name",
    "Confirm name",
    "Read Password",
    "Get new password",
    "Confirm new password",
    "Get sex",
    "Read messages of today",
    "Read Menu",
    "Get extra description",
    "Get class",
    "Link Dead",
    "New Password",
    "Password Confirm",
    "Wizlocked",
    "Get Race",
    "Racpar",
    "Auth",
    "City Choice",
    "Stat Order",
    "Delete",
    "Delete",
    "Stat Order",
    "Wizard MOTD",
    "Editing",
    "Nuking Theirself",
    "Mage/Sorcerer Question",
    "Object Editing",
    "Mob Editing",
    "Re-roll",
    "Press Enter",
    "Already Playing",
    "Getting Ansii",
    "Creation Menu",
    "Picking Alignment",
    "Help File Editing",
    "_e-mail reg",
    "Text File Editing",
    "\n"
};

/*
 * [class], [level] (all)
 */
const int       thaco[MAX_CLASS][ABS_MAX_LVL] = {
    /*
     * mage
     */
    {100, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 17,
     17, 17, 17, 17, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 14, 14, 14,
     14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * cleric
     */
    {100, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15,
     15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9,
     9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 1, 1, 1, 1, 1, 1, 1,
     1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * fighter
     */
    {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,
     2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * thief
     */
    {100, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 17, 16, 16,
     16, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11,
     11, 11, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * druid
     */
    {100, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15,
     15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9,
     9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 1, 1, 1, 1, 1, 1, 1,
     1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * monk
     */
    {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,
     2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * barbarian
     */
    {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,
     2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * sorcerer
     */
    {100, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 17,
     17, 17, 17, 17, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 14, 14, 14,
     14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * paladin
     */
    {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,
     2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * ranger
     */
    {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,
     2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * psionist
     */
    {100, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 17, 16, 16,
     16, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11,
     11, 11, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    /*
     * Necromancer
     */
    {100, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15,
     15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9,
     9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

};

/*
 * [ch] strength apply (all)
 */
const struct str_app_type str_app[31] = {
    {-5, -4, 0, 0},             /* 0 */
    {-5, -4, 1, 1},             /* 1 */
    {-3, -2, 1, 2},
    {-3, -1, 5, 3},             /* 3 */
    {-2, -1, 10, 4},
    {-2, -1, 20, 5},            /* 5 */
    {-1, 0, 25, 6},
    {-1, 0, 30, 7},
    {0, 0, 40, 8},
    {0, 0, 50, 9},
    {0, 0, 55, 10},             /* 10 */
    {0, 0, 70, 11},
    {0, 0, 80, 12},
    {0, 0, 90, 13},
    {0, 0, 100, 14},
    {0, 0, 110, 15},            /* 15 */
    {0, 1, 120, 16},
    {1, 1, 130, 18},
    {1, 2, 140, 20},            /* 18 */
    {3, 7, 485, 40},
    {3, 8, 535, 40},            /* 20 */
    {4, 9, 635, 40},
    {4, 10, 785, 40},
    {5, 11, 935, 40},
    {6, 12, 1235, 40},
    {7, 14, 1535, 40},          /* 25 */
    {1, 3, 155, 22},            /* 18/01-50 */
    {2, 3, 170, 24},            /* 18/51-75 */
    {2, 4, 185, 26},            /* 18/76-90 */
    {2, 5, 255, 28},            /* 18/91-99 */
    {3, 6, 355, 30}             /* 18/100 (30) */
};

/*
 * [dex] skillapply (thieves only)
 */
const struct dex_skill_type dex_app_skill[26] = {
    {-99, -99, -90, -99, -60},  /* 0 */
    {-90, -90, -60, -90, -50},  /* 1 */
    {-80, -80, -40, -80, -45},
    {-70, -70, -30, -70, -40},
    {-60, -60, -30, -60, -35},
    {-50, -50, -20, -50, -30},  /* 5 */
    {-40, -40, -20, -40, -25},
    {-30, -30, -15, -30, -20},
    {-20, -20, -15, -20, -15},
    {-15, -10, -10, -20, -10},
    {-10, -5, -10, -15, -5},    /* 10 */
    {-5, 0, -5, -10, 0},
    {0, 0, 0, -5, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},            /* 15 */
    {0, 5, 0, 0, 0},
    {5, 10, 0, 5, 5},
    {10, 15, 5, 10, 10},
    {15, 20, 10, 15, 15},
    {15, 20, 10, 15, 15},       /* 20 */
    {20, 25, 10, 15, 20},
    {20, 25, 15, 20, 20},
    {25, 25, 15, 20, 20},
    {25, 30, 15, 25, 25},
    {25, 30, 15, 25, 25}        /* 25 */
};

/*
 * [level] backstab multiplyer (thieves only)
 */
const byte      backstab_mult[ABS_MAX_LVL] = {
    1,                          /* 0 */
    2,                          /* 1 */
    2,
    2,
    2,
    2,                          /* 5 */
    2,
    2,
    3,                          /* 8 */
    3,
    3,                          /* 10 */
    3,
    3,
    3,
    3,
    3,                          /* 15 */
    4,                          /* 16 */
    4,
    4,
    4,
    4,                          /* 20 */
    4,
    4,
    4,
    5,                          /* 25 */
    5,
    5,
    5,
    5,
    5,                          /* 30 */
    5,
    5,
    6,
    6,
    6,                          /* 35 */
    6,
    6,
    6,
    7,
    7,                          /* 40 */
    7,
    7,
    7,
    8,
    8,                          /* 45 */
    8,
    8,
    8,
    9,
    10,                         /* 50? */
    11,
    11,
    12,
    12,
    13,
    14,
    15,
    16,
    17,
    18,                         /* 60 */
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28                          /* 70 */
};
/*
 * [level]backstab multiplier for non primary thieves (Gordon 11JAN04)
 */
const byte      lesser_backstab_mult[ABS_MAX_LVL] = {
    1,                          /* 0 */
    2,                          /* 1 */
    2,
    2,
    2,
    2,                          /* 5 */
    2,
    2,
    2,                          /* 8 */
    3,
    3,                          /* 10 */
    3,
    3,
    3,
    3,
    3,                          /* 15 */
    3,                          /* 16 */
    4,
    4,
    4,
    4,                          /* 20 */
    4,
    4,
    4,
    4,                          /* 25 */
    4,
    4,
    4,
    4,
    4,                          /* 30 */
    5,
    5,
    5,
    5,
    5,                          /* 35 */
    5,
    5,
    5,
    5,
    6,                          /* 40 */
    6,
    6,
    6,
    6,
    6,                          /* 45 */
    6,
    6,
    7,
    7,
    7,                          /* 50? */
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,                         /* 60 */
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26                          /* 70 */
};

/*
 * [dex] apply (all)
 */
struct dex_app_type dex_app[41] = {
    {-7, -7, 60},               /* 0 */
    {-6, -6, 50},               /* 1 */
    {-4, -4, 50},
    {-3, -3, 40},
    {-2, -2, 30},
    {-1, -1, 20},               /* 5 */
    {0, 0, 10},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},                  /* 10 */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, -10},                /* 15 */
    {1, 1, -20},
    {2, 2, -30},
    {2, 2, -40},
    {3, 3, -40},
    {3, 3, -40},                /* 20 */
    {4, 4, -50},
    {4, 4, -50},
    {4, 4, -50},
    {5, 5, -60},
    {5, 5, -60}                 /* 25 */

};

/*
 * [con] apply (all)
 */
struct con_app_type con_app[26] = {
    {-4, 20},                   /* 0 */
    {-3, 25},                   /* 1 */
    {-2, 30},
    {-2, 35},
    {-1, 40},
    {-1, 45},                   /* 5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},                    /* 16 */
    {3, 97},                    /* 17 */
    {4, 99},                    /* 18 */
    {4, 99},
    {5, 99},                    /* 20 */
    {6, 99},
    {6, 99},
    {7, 99},
    {8, 99},
    {9, 100}                    /* 25 */
};

/*
 * [int] apply (all)
 */
struct int_app_type int_app[26] = {
    {0},
    {1},                          /* 1 */
    {2},
    {3},
    {4},
    {5},                          /* 5 */
    {6},
    {8},
    {10},
    {12},
    {14},                         /* 10 */
    {16},
    {18},
    {20},
    {22},
    {25},                         /* 15 */
    {28},
    {32},
    {35},
    {40},
    {45},                         /* 20 */
    {50},
    {60},
    {70},
    {80},
    {99}                          /* 25 */
};

struct int_app_type int_sf_modifier[26] = {
    {60},
    {50},                         /* 1 */
    {42},
    {35},
    {29},
    {24},                         /* 5 */
    {19},
    {15},
    {11},
    {8},
    {5},                          /* 10 */
    {3},
    {2},
    {1},
    {0},
    {-1},                         /* 15 */
    {-2},
    {-4},
    {-8},
    {-16},
    {-25},                        /* 20 */
    {-50},
    {-50},
    {-50},
    {-50},
    {-50}                         /* 25 */
};

/*
 * [wis] apply (all)
 */
struct wis_app_type wis_app[26] = {
    {0},
    {0},
    {0},
    {1},
    {1},
    {1},
    {1},
    {1},
    {2},
    {2},
    {3},                          /* 10 */
    {3},                          /* 11 */
    {3},                          /* 12 */
    {4},                          /* 13 */
    {4},                          /* 14 */
    {5},                          /* 15 */
    {5},                          /* 16 */
    {6},                          /* 17 */
    {7},                          /* 18 */
    {7},
    {7},
    {7},
    {7},
    {7},
    {7},
    {8}
};

struct chr_app_type chr_apply[26] = {
    {0, -70},                   /* 0 */
    {0, -70},
    {1, -60},
    {1, -50},
    {1, -40},                   /* 4 */
    {2, -30},
    {2, -20},
    {3, -10},
    {4, 0},
    {5, 0},                     /* 9 */
    {6, 0},
    {7, 0},
    {8, 0},
    {9, +5},                    /* 13 */
    {10, +10},
    {12, +15},
    {14, +20},
    {17, +25},
    {20, +30},                  /* 18 */
    {20, +40},
    {25, +50},
    {25, +60},
    {25, +70},
    {25, +80},
    {25, +90},
    {25, +95},
};

const char     *spell_desc[] = {        /* Last reviewed by Sentinel */
    "!ERROR!",
    "$n is surrounded by a armor-like magical shield",
    "!Teleport!",
    "$n is blessed",
    "$n is temporarily blinded",
    "",
    "!Call Lightning",
    "",
    "",
    "!Clone!",
    "!Color Spray!",
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "$n has trouble hitting anything",
    "$n's eyes glow red",
    "$n's eyes glow yellow",
    "$n's eyes glow blue",
    "$n can see poison",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "",
    "!Lightning Bolt!",
    "!Locate object!",
    "!Magic Missile!",          /* haha, made you look! */
    "$n looks very ill.",
    "$n is protected from evil",
    "!Remove Curse!",
    "$n is surrounded by a white aura",
    "!Shocking Grasp!",
    "",
    "$n looks really strong",
    "!Summon!",
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "$n possesses acute senses",
    "",                         /* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "!Identify!",               /* 53 */
    "$n's eyes glow redly",
    "!cause light!",
    "!cause crit!",
    "!flamestrike!",
    "!dispel good!",
    "$n looks weak and pitiful",
    "!dispel magic!",
    "!knock!",
    "!know alignment!",
    "!animate dead!",
    "$n seems unable to move.",
    "!remove paralysis!",
    "!fear!",
    "!acid blast!",
    "$n looks rather fishy",
    "$n is flying",
    "spell 70, please report.", /* 70 */
    "spell 712, please report.",
    "spell 72, please report",
    "$n is surrounded by a weak shield of magic.",
    "spell 74, please report",
    "spell 75, please report.",
    "spell 76, please report.",
    "spell 77, please report.",
    "spell 78, please report.",
    "spell 79, please report.",
    "spell 80, please report.", /* 80 */
    "$n is surrounded by glowing red flames!",
    "",
    "spell83, please report.",
    "spell84, please report.",
    "spell85, please report.",
    "spell86, please report.",
    "spell87, please report.",
    "spell88, please report.",
    "spell89, please report.",
    "spell90, please report.",  /* 90 */
    "spell91, please report.",
    "$n's skin has taken on the consistency of hard granite",
    "spell93, please report.",
    "$n's eyes glow with a silvery hue",
    "spell95, please report",
    "$n is surrounded by a pink aura",  /* 96 */
    "spell 97, please report.",
    "spell 98 please report.",
    "spell 99 please report.",
    "spell 100 please report.",
    "spell 101 please report.",
    "spell 102 please report.",
    "spell 103 please report.",
    "$n is flying",
    "spell 105 please report.",
    "$n is covered by sticky webs.",
    "$n can find things",
    "$n can find anyone",       /* 108 */
    "spell 109 please report.",
    "",
    "spell 111 please report.",
    "spell 112 please report.",
    "spell 113 please report.",
    "spell 114 please report.",
    "spell 115 please report.",
    "spell 116 please report.",
    "spell 117 please report.",
    "$e seems aided by divine blessing",
    "spell 119 please report.",
    "",
    "$e is having some trouble thinking.",
    "spell 122 please report.",
    "spell 123 please report.",
    "spell 124 please report.",
    "$n seems unnaturally large.",
    "$n seems unnaturally large",
    "spell 127 please report.",
    "spell 128 please report.",
    "",                         /* animal summon one */
    "spell 130 please report.", /* animal summon two */
    "spell 131 please report.", /* animal summon three */
    "",                         /* fire servant */
    "",                         /* earth servant */
    "",                         /* water servant */
    "",                         /* wind servant */
    "spell 136 please report.", /* reincarnate */
    "spell 137 please report.",
    "$n has been magically enlarged!",
    "spell 139 please report.",
    "",
    "$n seems a bit shadowy",
    "$n moves rather fluidly",
    "creatures love $n",
    "predators ignore $n",
    "",                         /* slow poison */
    "$n is firmly entangled in vines.",
    "spell 147 please report.", /* Snare */
    "spell 148 please report.",
    "$n's skin seems rough, and brown, like bark!",
    "spell 150 please report.",
    "spell 151 please report.",
    "$n's armor is smoking and sizzling",
    "$n's eyes glimmer a bit.",
    "spell 154 please report.",
    "$e seems to be moving very rapidly",
    "$n seems to be slowed",
    "",
    "spell 158 please report.",
    "spell 159 please report.",
    "spell 160 please report.",
    "$n is very quiet.",
    "spell 162 please report.",
    "spell 163 please report.",
    "spell 164 please report.",
    "",
    "",
    "",
    "$n's eyes glow white.",
    "$n is protected from good.",       /* 169 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 190 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 200 */
    "$n is using $s wings to stay aloft.",      /* Wings flying */
    "$n's wings are resting.",
    "$n's wings are a painful mass of charred skin and feathers.",
    "$n is protected from good.",
    "$n is gigantic.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "A thick dark globe surrounds $n.", /* 215 */
    "$n is protected by a minor globe of protection.",
    "$n is protected by a major globe of protection.",
    "$n is resistant to drain.",        /* 218 */
    "$n is resistant to breath.",       /* 219 */
    "$n has a pulsing antimagic shell about $s body.",
    "",
    "",
    "",
    "$n is invisible.",
    "",                         /* 225 */
    "$n is surrounded by burning flames.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 240 */
    "$n is protected by a strong psionic shield.",      /* 241 */
    "$n is protected from evil.",
    "spell 243",
    "spell 244",
    "spell 245",
    "spell 246",
    "spell 247",
    "spell 248",
    "spell 249",
    "spell 250",
    "spell 251",
    "spell 252",
    "spell 253",
    "",
    "",                         /* 255 */
    "$n has protectection from fire surrounding $mself.",
    "$n has protectection from cold surrounding $mself.",
    "$n has protectection from energy surrounding $mself.",
    "$n has protectection from electricity surrounding $mself.",
    "",                         /* 260 */
    "",
    "$n is surrounded by a globe of protection from fire breath.",  /* 262 */
    "$n is surrounded by a globe of protection from frost breath.",
    "$n is surrounded by a globe of protection from electric breath.",
    "$n is surrounded by a globe of protection from acid breath.",
    "$n is surrounded by a globe of protection from gas breath.",
    "$n sees far ahead.",
    "",
    "",
    "$n knows where to step.",
    "",
    "",
    "$n levitates gently in the air.",
    "",
    "",
    "$n seems stronger.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 288 */
    "$n is surrounded by a armor-like magical shield.",
    "$n is strong.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",                         /* 300 */
    "",
    "",
    "",
    "",
    "",
    "",
    "Blackness surrounds $n.",
    "",
    "Bones cover $n.",
    "$n is surrounded by spirits.",     /* 310 */
    "",
    "",
    "",
    "$n is unnaturally strong.",
    "Shadows writhe around $n.",
    "",
    "",
    "$n skin seems cold.",
    "",
    "",                         /* 320 */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "$n eyes glow black.",
    "",
    "",                         /* 330 */
    "$n's soul belongs to the Dark Lord.",
    "",
    "Dark energy surrounds $n.",
    "",
    "",
    "",
    "",
    "An aura of chill flames surrounds $n.",
    "",
    "",                         /* 340 */
    "A whirling barrier of blades surrounds $n.",
    "\n"
};

#if 1
const struct QuestItem QuestList[4][IMMORTAL] = {
    {                           /* magic user */
        {2402, "An unusual way of looking at things."},
        {1, "It can be found in the donation room, or on your head\n\r"},
        {1410, "Its a heavy bag of white powder\n\r"},
        {6010, "You can make pies out of them, deer seem to like them too\n\r"},
        {3013, "Its a yummy breakfast food, they go great with eggs and "
               "cheese\n\r"},
        {20, "If you twiddle your thumbs enough, you'll find one.\n\r"},
        {24764, "Dead people might wear them on their hands\n\r"},
        {112, "If you found one of these, it would be mighty strange!\n\r"},
        {106, "Eye of Toad and Toe of Frog, bring me one or the other\n\r"},
        {109, "A child's favorite place holds the object of my desire, on "
              "the dark river\n\r"},
        /*
         * 10
         */
        {3628, "The latest in New padded footwear\n\r"},
        {113, "A child might play with one, when the skexies aren't "
              "around\n\r"},
        {19204, "A precious moon in a misty castle\n\r"},
        {20006, "Are you a fly? You might run into one. Beware..\n\r"},
        {1109, "Little people have tiny weapons.. bring me one\n\r"},
        {6203, "IReallyReallyWantACurvedBlade\n\r"},
        {21007, "I want to be taller and younger, find the nasty children\n\r"},
        {5228, "Don't you find screaming women so disarming?\n\r"},
        {7204, "Vaulted lightning\n\r"},
        {16043, "Precious elements can come in dull, dark mines\n\r"},
        /*
         * 20
         */
        {20007, "You'll catch him napping, no guardian of passing time."},
        {16903, "Nature's mistake, carried by a man on a new moon, "
                "fish on full."},
        {5226, "Sealed in the hands of a city's failed guardian."},
        {10900, "Anachronistic rectangular receptacle holds circular "
                "plane."}, /* Sentry, TL */
        {13840, "What kind of conditioner does one use for "
                "asps?"},/* Medusa, GQ */
        {7406, "If you don't bring a scroll of recall, you might die a "
               "fiery death"},      /* Room 7284, SM */
        {120, "Dock down anchor"}, /* Ixitxachitl, NT */
        {21008, "Very useful, behind a hearth."},  /* Dog, OR */
        {10002, "He didn't put them to sleep with these, the results were "
                "quite deadly"},  /* On Corpse, DCE */
        {3648, "Unsummoned, they pose large problems. What you want is on "
               "their queen."},  /* Chieftess, HGS */
        {15805, "A single sample of fine plumage, held by a guard and "
             "a ghost."},  /* Guard & Brack, SK */
        {21141, "In the land of the troglodytes there is a headpiece with "
             "unpleasant powers."},    /* Case, TR */
        {1532, "Three witches have the flighty component you need."},/*Pot,MT*/
        {5304, "A spectral force holds the key to advancement in a geometric "
               "dead end."},  /* Spectre, PY */
        {9496, "A great golden sword was once taken by the giants of the "
               "great with north.  return it. Bring friends."},
        {5105, "What you need is as dark as the heart of the elf "
               "who wields it."}, /* Weaponsmaster, DR */
        {21011, "The key to your current problem is in Orshingal, on a "
                "haughty strutter."},  /* Enfan, OR */
        {27004, "A small explosive pinapple shaped object. Ever see "
                "Monty-Python?, you might find it in the hands of sorcerous "
                "undead men"},       /* ???, Haplo's */
        {6616, "You might smile if drinking a can of this. Look in "
               "Prydain."},     /* ???, PRY */
        {21125, "With enough of this strong amber drink you'd forget about "
                "the nightly ghosts."},  /* Crate, ARO */
        /*
         * 40
         */
        {5309, "Powerful items of magic follow... first bring me a medallion "
               "of mana\n\r"},
        {1585, "Bubble bubble, toil and trouble, bring me a staff on the "
               "double\n\r"},
        {21003, "I need some good boots, you know how strange it is to find "
                "them\n\r"},
        {13704, "Watch for a dragon, find his ring\n\r"},
        {252, "Dead-makers cloaks, buried deep in stone\n\r"},
        {3670, "bottled mana\n\r"},
        {1104, "The master of fireworks, take his silver"},
        {5020, "You're not a real mage till you get platinum from purple\n\r"},
        {1599, "Grand Major\n\r"},
        {20002, "She's hiding in her room, but she has my hat!\n\r"}
    },
    {                       /* cleric */
        {0, ""},
        {1, "It can be found in the donation room, or on your head\n\r"},
        {1110, "White and young, with no corners or sides, a golden "
               "treasure can be found inside\n\r"},
        {3070, "The armorer might have a pair, but they're not his "
               "best on hand\n\r"},
        {3057, "judicandus dies\n\r"},
        {6001, "I want clothes i can play chess on\n\r"},
        {16033, "A goblin's favorite food, the eastern path\n\r"},
        {107, "Every righteous cleric should have one, but few do\n\r"},
        {4000, "I have a weakness for cheap rings\n\r"},
        {3025, "cleaver for hire, just outside of midgaard\n\r"},
        /*
         * 10
         */
        {3649, "My wife needs something New, to help keep her girlish "
               "figure\n\r"},
        {7202, "mindflayers have small ones, especially in the sewers\n\r"},
        {19203, "the weapon of a traitor, lost in a fog\n\r"},
        {15814, "striped black and white, take what this gelfling-friend "
                "offers\n\r"},
        {119, "Play with a gypsy child when he asks you to\n\r"},
        {5012, "You might use these to kill a vampire, they are in the "
               "desert\n\r"},
        {6809, "Really cool sunglasses, on a really cool guy, in a really "
               "cool place\n\r"},
        {17021, "The proof is in the volcano\n\r"},
        {3648, "Giant women have great fashion sense\n\r"},
        {27001, "Undead have the strangest law enforcement tools, near a "
                "split in the river\n\r"},
        /*
         * 20
         */
        {105, "A venomed bite will end your life - you need the jaws that "
              "do it."},    /* Rattler fangs, MidT */
        {3668, "Buy some wine from a fortuneteller's "
               "brother."}, /* Gypsy Bartender */
        {1703, "On a cat, but not in the petting zoo."},/* Bengal tiger, MZ */
        {13758, "Held by a doggie with who will bite you and bite you "
                "and bite you."}, /* Cerebus, Hades */
        {5240, "In the old city you'll find the accursed vibrant stone "
               "you require."}, /* Lamia, OT */
        {5013, "Where can you go for directions in the desert? Try the "
               "wet spot."},    /* Oasis Map, GED */
        {17011, "{An unholy symbol on an unholy creature under a fuming "
                "mountain."},   /* Amelia, WPM */
        {1708, "Some liar's got it in a cul-de-sac. The high priests of "
               "Odin know the way."},  /* Liar in room 10911, TL */
        {9203, "What would you use to swat a very large mosquito? Get it "
               "from the giants."},   /* Giant, HGS */
        {21109, "A bow made of dark wood, carried by a "
                "troglodyte."},  /* Trog, CT */
        /*
         * 30
         */
        {15817, "In an secret cold place, a dark flower is carried by a "
                "midnight one."},  /* Bechemel, SK */
        {9430, "Argent Ursa, Crevasse of the Arctic Enlargements"},
        {6112, "If you would be king, you'd need one. With a wooded "
               "worm."},   /* Dragon, DH-D */
        {1758, "Carried by a hag in the dark lake under the "
               "sewers."}, /* Sea Hag, SM */
        {27411, "This Roo's better than you and she has the stick to "
                "prove it."},      /* Queen Roo, LDU */
        {5317, "The dead don't lie still when properly "
               "prepared."},    /* Mummy, PY */
        {5033, "You can get it off a drider, but he won't give it to "
               "you."},   /* Drider, DR */
        {16615, "South of a bay, past a portal, into a tower, be "
                "prepared"},   /* Hoeur, HR */
        {121, "To the far southeast, in the lair of a pair of "
              "arachnids."},    /* Cave Spiders, TR */
        {13901, "On the shore, down the river from the "
                "troll-bridge"}, /* RHY */
        /*
         * 40
         */
        {5104, "Four heads are better than one\n\r"},
        {15806, "You don't stand a ghost of a chance against a glow of "
                "white and a cloak of fire"},
        {16022, "a Powerful, blunt, and fragile weapon\n\r"},
        {122, "The sole possession of a devil down-under\n\r"},
        {7220, "The highest thing at the top of a chain"},
        {13785, "From the fairest\n\r"},
        {1597, "Mana in a green ring\n\r"},
        {1563, "Famous, blue and very very rare"},
        {5001, "Search for a banded male\n\r"},
        {20003, "Ensnared for power, she holds the helmet of the wise"}
    },
    {                   /* warrior */
        {2402, "An unusual way of looking at things."},
        {11, "Something you might find in the donation room, or on "
             "your body\n\r"},
        {16034, "Goblins have been known to play with these, especially "
             "in dark caves\n\r"},
        {6000, "A decent weapon, just the right size for a tree\n\r"},
        {24760, "Dead men's feet look like this\n\r"},
        {1413, "You were SUPPOSED to bell the CAT!\n\r"},
        {18256, "In the city of Mordilnia, a shield of roygiv\n\r"},
        {8121, "A bag that opens with a ripping sound\n\r"},
        {108, "Floating for safety on the dark\n\r"},
        {123, "A mule stole my hat, now he fights in front of an "
              "audience\n\r"},
        /*
         * 10
         */
        {3621, "Thank goodness when I broke my arm, I still had my New "
               "shield\n\r"},
        {117, "If you get this, someone will be quite howling angry\n\r"},
        {7405, "Sewer Secret Light Sources\n\r"},
        {6205, "my eyes just arent as fast to focus as they used to be\n\r"},
        {4051, "These warriors seem scarred, but its all in their "
               "head"},  /* Scarred Warrior, MM */
        {5219, "Fresh deer.. yum!\n\r"},
        {16015, "An ugly bird in the mountains once told me: 'A despotic "
                "ruler rules with one of these'\n\r"},
        {1718, "Hey, that's not a painting at all! But boy is she ugly! "
               "In the new city."},        /* Mimic, NT */
        {5032, "Bushwhacked, Bushwhacked, West, West, Green. Start at "
               "the obvious\n\r"},
        {3685, "Mightier than a sword, wielded by a four man\n\r"},
        /*
         * 20
         */
        {5100, "Learn humility: I want a common sword\n\r"},
        {16902, "They'd all be normal in a moonless world. You need to "
                "steal a silver stole"},     /* Werefox, LY */
        {17022, "A lion with a woman's torso holds the book you "
                "need."},   /* Gynosphinx, WPM */
        {5206, "To hold the girth of a corpulent man, it must be ferrous. "
               "In the old city hall."}, /* Cabinet, OT */
        {1737, "In the hands of an elf with a green thumb."},/* Gardener, NT */
        {5306, "my mommy gave me a knife, but i lost it\n\r"},
        {21006, "Childlike, maybe, but they're not children. You need the "
                "locked up cloth."},      /* Case, OR */
        {9204, "The largest in the hands of the largest of the "
               "large"},    /* Giant Chief, HGS */
        {1721, "Get the toolbook of the trade from the royal cook in the "
               "new city."},      /* Chef, NT */
        {16901, "Only an elephant's might be as big as this bores' "
                "mouthpiece."},  /* Boarish, LY */
        /*
         * 30
         */
        {6511, "A bearded woman might be so engaged, but a guard's got "
               "this one."},
        {5101, "Dark elves can be noble too, but they won't let you take "
               "their arms."},    /* Drow Noble, DR */
        {1761, "In a suspended polygon, in a chest which is not."},/*Mimic, PY*/
        {15812, "You think that water can't be sharp? Look under "
                "birdland."},      /* Ice Pick, SK */
        {16046, "A miner's tool in the dwarven mines"}, /* Shovel, ?? */
        {21114, "These skeletal beasts will reel you in, you want the "
                "crowbar."},  /* Cave Fisher, MVE */
        {13762, "Once in Hades, the key to getting out lies with a long "
                "dead warrior."},   /* Skeletal Warrior, Hades */
        {20005, "This usurper will think you very tasty, defeat him for the "
                "thing you need."},     /* Yevaud, AR */
        {5019, "A nasty potion in the hands of an even nastier desert "
               "worm."},     /* Worm, GED */
        {10002, "This item can be found on 'Al'"}, /* On Corpse, DCE */
        /*
         * 40
         */
        {5221, "Weapons are the keys to the remaining quests. First, bring "
               "me a Stone golem's sword"},
        {9442, "The weapon of the largest giant in the rift"},
        {15808, "Weapon of champions"},
        {13775, "By the light of the moon\n\r"},
        {21004, "By name, you can assume its the largest weapon in the "
                "game\n\r"},
        {3092, "He's always pissed, and so are his guards. take his weapon "
               "and make it yours\n\r"},
        {5002, "The weapon of the oldest wyrm\n\r"},
        {5107, "One Two Three Four Five Six\n\r"},
        {1430, "It rises from the ashes, and guards a tower\n\r"},
        {5019, "You're not a REAL fighter til you've had one of these, "
               "enchanted\n\r"}
    },
    {               /* thief */
        {2402, "An unusual way of looking at things."},
        {4, "You might find one of these in the donation room, or in "
            "your hand\n\r"},
        {3071, "They're the best on hand for 5 coins\n\r"},
        {30, "At the wrong end of a nasty spell, or a heavy hitter\n\r"},
        {3902, "Michelob or Guiness Stout. which is better?\n\r"},
        {24767, "I've heard that skeletons love bleach\n\r"},
        {6006, "Nearly useless in a hearth\n\r"},
        {4104, "Its what makes kobolds green\n\r"},
        {42, "Do she-devils steal, as they flap their bat wings?\n\r"},
        {19202, "Animal light, lost in a fog\n\r"},
        {3647, "These New boots were made for walking\n\r"},
        {4101, "Hands only a warrior could love\n\r"},
        {116, "Near a road to somewhere city\n\r"},
        {111, "Only a fool would look at the end of the river\n\r"},
        {15812, "I'd love a really cool backstabbing weapon..  Make sure "
                "it doesn't melt\n\r"},
        {17023, "Being charming can be offensive, especially in a plumed "
                "white cap\n\r"},
        {9205, "You could hide a giant in this stuff\n\r"},
        {10002, "feeling tired and fuzzy?  Exhibit some stealth, or you "
                "just might get eaten\n\r"},
        {3690, "I am an old man, but I will crush you at chess\n\r"},
        {5000, "Find a dark dwarf. Pick something silver\n\r"},
        /*
         * 20
         */
        {15802, "It's easy work to work a rejected bird for the means "
                "to his former home."},   /* Skexie Reject, SK */
        {1750, "In the twisted forest of the Graecians a man in a black "
               "cloak has it."},       /* Put on 13731, GRF */
        {5012, "Vampire's bane in a wicker basket near a desert "
               "pool."},   /* Basket, GED */
        {20008, "The toothless dragon eats the means to your "
                "advancement."},   /* Young Wormkin, AR */
        {6810, "You are everywhere you look in this frozen northern "
               "maze of ice."},    /* Room 6854, ART */
        {255, "Get the happy stick from a desert worm."},/* Young Worm, GED */
        {7190, "In a secret sewer place a squeaking rodent wears a "
               "trinket."}, /* Rat, SM */
        {7205, "The master flayer under the city has it on him, but "
               "not in use. Steal it!"},   /* Master mind, SM */
        {7230, "You could be stoned for skinning this subterranean "
               "reptilian monster."},       /* Basilisk, SM */
        {3690, "An old man at the park might have one, but these old men "
               "are in the new city."},       /* Old man, NT */
        /*
         * 30
         */
        {1729, "In the forest north of the new city a traveller lost "
               "his way. It's on him."},  /* Lost Adventurer, MT */
        {1708, "It's growing on a cliff face, on the way to the lost "
               "kingdom."},       /* In room 21170, MVE */
        {1759, "The moon's phase can change a man. Find the badger in a "
               "tavern."},     /* Werebadger, LY */
        {1718, "You'll find it in the only ice cave a stone's throw from "
               "a desert."},  /* In room 10010, DCE */
        {5243, "I hope it is clear which stone you will need"}, /* Lamia, OT */
        {5302, "In a hanging desert artifact, the softest golem has the "
               "key to your success."},        /* Clay Golem, PY */
        {21008, "If your dog were this ugly, you'd lock him in a "
                "fireplace too!"},     /* Dog, OR */
        {9206, "It can be electrifying scaling a dragon; a big guy must "
               "have done it."},       /* Chieftain, HGS */
        {6524, "The dwarven mazekeeper has the only pair, if you can "
               "find him."},      /* Mazekeeper, Dwarf Mines */
        {1533, "Three witches in the mage's tower have the orb you "
               "need"},     /* Pot, MT */
        /*
         * 40
         */
        {9425, "A huge gemstone, guarded by ice toads, beware their poison"},
        {5113, "The weapon of a maiden, shaped like a goddess"},
        {21014, "The dagger of a yellow-belly"},
        {5037, "A thief of great reknown, at least he doesn't use a spoon"},
        {1101, "Elven blade of ancient lore, matches insects blow for blow"},
        {27000, "It strikes like a rattlesnake, but not as deadly"},
        {27409, "The weapon of a primitive man, just right for killing "
                "his mortal foe"},
        {1594, "White wielded by white, glowing white\n\r"},
        {20001, "He judges your soul, wields a weapon that shares your name"},
        {13703, "Watch for a dragon, he wears that which you seek"}
    }
};
#endif

const char     *att_kick_kill_ch[] = {
    "Your kick caves $N's chest in, which kills $M.",
    "Your kick destroys $N's arm and caves in one side of $S rib cage.",
    "Your kick smashes through $N's leg and into $S pelvis, killing $M.",
    "Your kick shatters $N's skull.",
    "Your kick at $N's snout shatters $S jaw, killing $M.",
    "You kick $N in the rump with such force that $E keels over dead.",
    "You kick $N in the belly, mangling several ribs and killing $M instantly.",
    "$N's scales cave in as your mighty kick kills $N.",
    "Your kick rips bark asunder and leaves fly everywhere, killing the $N.",
    "Bits of $N are sent flying as you kick him to pieces.",
    "You punt $N across the room, $E lands in a heap of broken flesh.",
    "You kick $N in the groin, $E dies screaming an octave higher.",
    ".",                        /* GHOST */
    "Feathers fly about as you blast $N to pieces with your kick.",
    "Your kick splits $N to pieces, rotting flesh flies everywhere.",
    "Your kick topples $N over, killing it.",
    "Your foot shatters cartilage, sending bits of $N everywhere.",
    "You launch a mighty kick at $N's gills, killing it.",
    "Your kick at $N sends $M to the grave.",
    "."
};
const char     *att_kick_kill_victim[] = {
    "$n crushes you beneath $s foot, killing you.",
    "$n destroys your arm and half your ribs.  You die.",
    "$n neatly splits your groin in two, you collapse and die instantly.",
    "$n splits your head in two, killing you instantly.",
    "$n forces your jaw into the lower part of your brain.",
    "$n kicks you from behind, snapping your spine and killing you.",
    "$n kicks your stomach and you into the great land beyond!!",
    "Your scales are no defense against $n's mighty kick.",
    "$n rips you apart with a massive kick, you die in a flutter of leaves.",
    "You are torn to little pieces as $n splits you with $s kick.",
    "$n's kick sends you flying, you die before you land.",
    "Puny little $n manages to land a killing blow to your groin, OUCH!",
    ".",                        /* GHOST */
    "Your feathers fly about as $n pulverizes you with a massive kick.",
    "$n's kick rips your rotten body into shreds, and your various pieces die.",
    "$n kicks you so hard, you fall over and die.",
    "$n shatters your exoskeleton, you die.",
    "$n kicks you in the gills!  You cannot breathe.... you die!.",
    "$n sends you to the grave with a mighty kick.",
    "."
};
const char     *att_kick_kill_room[] = {
    "$n strikes $N in chest, shattering the ribs beneath it.",
    "$n kicks $N in the side, destroying $S arm and ribs.",
    "$n nails $N in the groin, the pain killing $M.",
    "$n shatters $N's head, reducing $M to a twitching heap!",
    "$n blasts $N in the snout, destroying bones and causing death.",
    "$n kills $N with a massive kick to the rear.",
    "$n sends $N to the grave with a massive blow to the stomach!",
    "$n ignores $N's scales and kills $M with a mighty kick.",
    "$n sends bark and leaves flying as $e splits $N in two.",
    "$n blasts $N to pieces with a ferocious kick.",
    "$n sends $N flying, $E lands with a LOUD THUD, making no other noise.",
    "$N falls to the ground and dies clutching $S crotch due to $n's kick.",
    ".",                        /* GHOST */
    "$N disappears into a cloud of feathers as $n kicks $M to death.",
    "$n blasts $N's rotten body into pieces with a powerful kick.",
    "$n kicks $N so hard, it falls over and dies.",
    "$n blasts $N's exoskeleton to little fragments.",
    "$n kicks $N in the gills, killing it.",
    "$n sends $N to the grave with a mighty kick.",
    "."
};
const char     *att_kick_miss_ch[] = {
    "$N steps back, and your kick misses $M.",
    "$N deftly blocks your kick with $S forearm.",
    "$N dodges, and you miss your kick at $S legs.",
    "$N ducks, and your foot flies a mile high.",
    "$N steps back and grins evilly as your foot flys by $S face.",
    "$N laughs at your feeble attempt to kick $M from behind.",
    "Your kick at $N's belly makes it laugh.",
    "$N chuckles as your kick bounces off $S tough scales.",
    "You kick $N in the side, denting your foot.",
    "Your sloppy kick is easily avoided by $N.",
    "You misjudge $N's height and kick well above $S head.",
    "You stub your toe against $N's shin as you try to kick $M.",
    "Your kick passes through $N!!",    /* Ghost */
    "$N nimbly flitters away from your kick.",
    "$N sidesteps your kick and sneers at you.",
    "Your kick bounces off $N's leathery hide.",
    "Your kick bounces off $N's tough exoskeleton.",
    "$N deflects your kick with a fin.",
    "$N avoids your paltry attempt at a kick.",
    "."
};
const char     *att_kick_miss_victim[] = {
    "$n misses you with $s clumsy kick at your chest.",
    "You block $n's feeble kick with your arm.",
    "You dodge $n's feeble leg sweep.",
    "You duck under $n's lame kick.",
    "You step back and grin as $n misses your face with a kick.",
    "$n attempts a feeble kick from behind, which you neatly avoid.",
    "You laugh at $n's feeble attempt to kick you in the stomach.",
    "$n kicks you, but your scales are much too tough for that wimp.",
    "You laugh as $n dents $s foot on your bark.",
    "You easily avoid a sloppy kick from $n.",
    "$n's kick parts your hair but does little else.",
    "$n's light kick to your shin barely gets your attention.",
    "$n passes through you with $s puny kick.",
    "You nimbly flitter away from $n's kick.",
    "You sneer as you sidestep $n's kick.",
    "$n's kick bounces off your tough hide.",
    "$n tries to kick you, but your too tough.",
    "$n tried to kick you, but you deflected it with a fin.",
    "You avoid $n's feeble attempt to kick you.",
    "."
};

const char     *att_kick_miss_room[] = {
    "$n misses $N with a clumsy kick.",
    "$N blocks $n's kick with $S arm.",
    "$N easily dodges $n's feeble leg sweep.",
    "$N easily ducks under $n's lame kick.",
    "$N steps back and grins evilly at $n's feeble kick to $S face misses.",
    "$n launches a kick at $N's behind, but fails miserably.",
    "$N laughs at $n's attempt to kick $M in the stomach.",
    "$n tries to kick $N, but $s foot bounces off of $N's scales.",
    "$n hurts his foot trying to kick $N.",
    "$N avoids a lame kick launched by $n.",
    "$n misses a kick at $N due to $S small size.",
    "$n misses a kick at $N's groin, stubbing $s toe in the process.",
    "$n's foot goes right through $N!!!!",
    "$N flitters away from $n's kick.",
    "$N sneers at $n while sidestepping $s kick.",
    "$N's tough hide deflects $n's kick.",
    "$n hurts $s foot on $N's tough exterior.",
    "$n tries to kick $N, but is thwarted by a fin.",
    "$N avoids $n's feeble kick.",
    "."
};

const char     *att_kick_hit_ch[] = {
    "Your kick crashes into $N's chest.",
    "Your kick hits $N in the side.",
    "You hit $N in the thigh with a hefty sweep.",
    "You hit $N in the face, sending $M reeling.",
    "You plant your foot firmly in $N's snout, smashing it to one side.",
    "You nail $N from behind, sending him reeling.",
    "You kick $N in the stomach, winding $M.",
    "You find a soft spot in $N's scales and launch a solid kick there.",
    "Your kick hits $N, sending small branches and leaves everywhere.",
    "Your kick contacts with $N, dislodging little pieces of $M.",
    "Your kick hits $N right in the stomach, $N is rendered breathless.",
    "You stomp on $N's foot. After all, thats about all you can do to a giant.",
    ".",                        /* GHOST */
    "Your kick  sends $N reeling through the air.",
    "You kick $N and feel rotten bones crunch from the blow.",
    "You smash $N with a hefty roundhouse kick.",
    "You kick $N, cracking it's exoskeleton.",
    "Your mighty kick rearranges $N's scales.",
    "You leap off the ground and crash into $N with a powerful kick.",
    "."
};

const char     *att_kick_hit_victim[] = {
    "$n's kick crashes into your chest.",
    "$n's kick hits you in your side.",
    "$n's sweep catches you in the side and you almost stumble.",
    "$n hits you in the face, gee, what pretty colors...",
    "$n kicks you in the snout, smashing it up against your face.",
    "$n blasts you in the rear, ouch!",
    "Your breath rushes from you as $n kicks you in the stomach.",
    "$n finds a soft spot on your scales and kicks you, ouch!",
    "$n kicks you hard, sending leaves flying everywhere!",
    "$n kicks you in the side, dislodging small parts of you.",
    "You suddenly see $n's foot in your chest.",
    "$n lands a kick hard on your foot making you jump around in pain.",
    ".",                        /* GHOST */
    "$n kicks you, and you go reeling through the air.",
    "$n kicks you and your bones crumble.",
    "$n hits you in the flank with a hefty roundhouse kick.",
    "$n ruins some of your scales with a well placed kick.",
    "$n leaps off of the grand and crashes into you with $s kick.",
    "."
};

const char     *att_kick_hit_room[] = {
    "$n hits $N with a mighty kick to $S chest.",
    "$n whacks $N in the side with a sound kick.",
    "$n almost sweeps $N off of $S feet with a well placed leg sweep.",
    "$N's eyes roll as $n plants a foot in $S face.",
    "$N's snout is smashed as $n relocates it with $s foot.",
    "$n hits $N with an impressive kick from behind.",
    "$N gasps as $n kick $N in the stomach.",
    "$n finds a soft spot in $N's scales and launches a solid kick there.",
    "$n kicks $N.  Leaves fly everywhere!!",
    "$n hits $N with a mighty kick, $N loses parts of $Mself.",
    "$n kicks $N in the stomach, $N is rendered breathless.",
    "$n kicks $N in the foot, $N hops around in pain.",
    ".",                        /* GHOST */
    "$n sends $N reeling through the air with a mighty kick.",
    "$n kicks $N causing parts of $N to cave in!",
    "$n kicks $N in the side with a hefty roundhouse kick.",
    "$n kicks $N, cracking exo-skelelton.",
    "$n kicks $N hard, sending scales flying!",
    "$n leaps up and nails $N with a mighty kick.",
    "."
};

const int       preproomexitsquad1[] = {
    39901,
    39941,
    39935,
    39949,
    -1
};

const int       preproomexitsquad2[] = {
    40096,
    39991,
    40009,
    39982,
    -1
};

const int       preproomexitsquad3[] = {
    40011,
    40047,
    40018,
    40026,
    -1
};

const int       preproomexitsquad4[] = {
    40048,
    40075,
    40061,
    40081,
    -1
};

/*
 * Lennya: Let's make these for all classes, preferably in alphabetical
 * order for skills command.  Perhaps make a similar listing called
 * pracset warriorprac[] for GM purposes? would be easy to get spells at
 * GM in spell order (new spells at bottom).
 */
const struct skillset mainmageskills[] = {
    {"anti magic shell", SPELL_ANTI_MAGIC_SHELL, 48, 45},
    {"brew", SKILL_BREW, 1, 45},
    {"major invulnerability", SPELL_GLOBE_MAJOR_INV, 27, 45},
    {"mana shield", SPELL_MANA_SHIELD, 14, 45},
    {"None", -1, -1, -1}
};

const struct skillset mageskills[] = {
    {"acid blast", SPELL_ACID_BLAST, 6, 45},
    {"animate dead", SPELL_ANIMATE_DEAD, 16, 45},
    {"armor", SPELL_ARMOR, 5, 45},
    {"blindness", SPELL_BLINDNESS, 12, 45},
    {"burning hands", SPELL_BURNING_HANDS, 6, 45},
    {"cacaodemon", SPELL_CACAODEMON, 30, 45},
    {"calm", SPELL_CALM, 4, 45},
    {"chain lightning", SPELL_CHAIN_LIGHTNING, 25, 45},
    {"charm monster", SPELL_CHARM_MONSTER, 10, 45},
    {"charm person", SPELL_CHARM_PERSON, 4, 45},
    {"chill touch", SPELL_CHILL_TOUCH, 4, 45},
    {"colour spray", SPELL_COLOUR_SPRAY, 14, 45},
    {"comprehend languages", SPELL_COMP_LANGUAGES, 1, 45},
    {"cone of cold", SPELL_CONE_OF_COLD, 17, 45},
    {"conjure elemental", SPELL_CONJURE_ELEMENTAL, 16, 45},
    {"continual light", SPELL_CONT_LIGHT, 10, 45},
    {"create light", SPELL_LIGHT, 1, 45},
    {"curse", SPELL_CURSE, 13, 45},
    {"darkness", SPELL_GLOBE_DARKNESS, 5, 45},
    {"detect invisibility", SPELL_DETECT_INVISIBLE, 2, 45},
    {"detect magic", SPELL_DETECT_MAGIC, 1, 45},
    {"disintegrate", SPELL_DISINTEGRATE, 48, 45},
    {"dispel magic", SPELL_DISPEL_MAGIC, 9, 45},
    {"enchant armor", SPELL_ENCHANT_ARMOR, 16, 45},
    {"enchant weapon", SPELL_ENCHANT_WEAPON, 14, 45},
    {"energy drain", SPELL_ENERGY_DRAIN, 22, 45},
    {"faerie fire", SPELL_FAERIE_FIRE, 5, 45},
    {"faerie fog", SPELL_FAERIE_FOG, 16, 45},
    {"fear", SPELL_FEAR, 8, 45},
    {"feeblemind", SPELL_FEEBLEMIND, 34, 45},
    {"find familiar", SPELL_FAMILIAR, 2, 45},
    {"fireball", SPELL_FIREBALL, 25, 45},
    {"fireshield", SPELL_FIRESHIELD, 40, 45},
    {"fly", SPELL_FLY, 11, 45},
    {"group fly", SPELL_FLY_GROUP, 24, 45},
    {"gust of wind", SPELL_GUST_OF_WIND, 10, 45},
    {"haste", SPELL_HASTE, 23, 45},
    {"ice storm", SPELL_ICE_STORM, 11, 45},
    {"identify", SPELL_IDENTIFY, 10, 45},
    {"incendiary cloud", SPELL_INCENDIARY_CLOUD, 45, 45},
    {"infravision", SPELL_INFRAVISION, 8, 45},
    {"invisibility", SPELL_INVISIBLE, 4, 45},
    {"knock", SPELL_KNOCK, 3, 45},
    {"know alignment", SPELL_KNOW_ALIGNMENT, 7, 45},
    {"know monster", SPELL_KNOW_MONSTER, 9, 45},
    {"lightning bolt", SPELL_LIGHTNING_BOLT, 10, 45},
    {"magic missile", SPELL_MAGIC_MISSILE, 1, 45},
    {"major track", SPELL_MAJOR_TRACK, 20, 45},
    {"meteor swarm", SPELL_METEOR_SWARM, 36, 45},
    {"minor creation", SPELL_MINOR_CREATE, 8, 45},
    {"minor invulnerability", SPELL_GLOBE_MINOR_INV, 20, 45},
    {"minor track", SPELL_MINOR_TRACK, 12, 45},
    {"monsum five", SPELL_MON_SUM_5, 15, 45},
    {"monsum four", SPELL_MON_SUM_4, 12, 45},
    {"monsum one", SPELL_MON_SUM_1, 4, 45},
    {"monsum seven", SPELL_MON_SUM_7, 22, 45},
    {"monsum six", SPELL_MON_SUM_6, 18, 45},
    {"monsum three", SPELL_MON_SUM_3, 9, 45},
    {"monsum two", SPELL_MON_SUM_2, 7, 45},
    {"mount", SPELL_MOUNT, 47, 45},
    {"paralyze", SPELL_PARALYSIS, 20, 45},
    {"polymorph self", SPELL_POLY_SELF, 9, 45},
    {"portal", SPELL_PORTAL, 43, 45},
    {"power word blind", SPELL_PWORD_BLIND, 16, 45},
    {"power word kill", SPELL_PWORD_KILL, 23, 45},
    {"prismatic spray", SPELL_PRISMATIC_SPRAY, 17, 45},
    {"refresh", SPELL_REFRESH, 6, 45},
    {"scare", SPELL_SCARE, 4, 45},
    {"second wind", SPELL_SECOND_WIND, 20, 45},
    {"sending", SPELL_SENDING, 1, 45},
    {"shield", SPELL_SHIELD, 1, 45},
    {"shocking grasp", SPELL_SHOCKING_GRASP, 2, 45},
    {"silence", SPELL_SILENCE, 21, 45},
    {"sleep", SPELL_SLEEP, 3, 45},
    {"slowness", SPELL_SLOW, 19, 45},
    {"stone skin", SPELL_STONE_SKIN, 26, 45},
    {"strength", SPELL_STRENGTH, 6, 45},
    {"succor", SPELL_SUCCOR, 24, 45},
    {"summon", SPELL_SUMMON, 27, 45},
    {"teleport", SPELL_TELEPORT, 17, 45},
    {"teleport without error", SPELL_TELEPORT_WO_ERROR, 30, 45},
    {"ventriloquate", SPELL_VENTRILOQUATE, 1, 45},
    {"water breath", SPELL_WATER_BREATH, 9, 45},
    {"weakness", SPELL_WEAKNESS, 6, 45},
    {"web", SPELL_WEB, 11, 45},
    {"wizardeye", SPELL_WIZARDEYE, 35, 45},
    {"None", -1, -1, -1}
};

const struct skillset mainsorcskills[] = {
    {"anti magic shell", SPELL_ANTI_MAGIC_SHELL, 48, 45},
    {"brew", SKILL_BREW, 1, 45},
    {"major invulnerability", SPELL_GLOBE_MAJOR_INV, 27, 45},
    {"mana shield", SPELL_MANA_SHIELD, 14, 45},
    {"None", -1, -1}
};
const struct skillset sorcskills[] = {
    {"acid blast", SPELL_ACID_BLAST, 6, 45},
    {"animate dead", SPELL_ANIMATE_DEAD, 16, 45},
    {"armor", SPELL_ARMOR, 5, 45},
    {"blindness", SPELL_BLINDNESS, 12, 45},
    {"burning hands", SPELL_BURNING_HANDS, 6, 45},
    {"cacaodemon", SPELL_CACAODEMON, 30, 45},
    {"calm", SPELL_CALM, 4, 45},
    {"chain lightning", SPELL_CHAIN_LIGHTNING, 25, 45},
    {"charm monster", SPELL_CHARM_MONSTER, 10, 45},
    {"charm person", SPELL_CHARM_PERSON, 4, 45},
    {"chill touch", SPELL_CHILL_TOUCH, 4, 45},
    {"colour spray", SPELL_COLOUR_SPRAY, 14, 45},
    {"comprehend languages", SPELL_COMP_LANGUAGES, 1, 45},
    {"cone of cold", SPELL_CONE_OF_COLD, 17, 45},
    {"conjure elemental", SPELL_CONJURE_ELEMENTAL, 16, 45},
    {"continual light", SPELL_CONT_LIGHT, 10, 45},
    {"create light", SPELL_LIGHT, 1, 45},
    {"curse", SPELL_CURSE, 13, 45},
    {"darkness", SPELL_GLOBE_DARKNESS, 5, 45},
    {"detect invisibility", SPELL_DETECT_INVISIBLE, 2, 45},
    {"detect magic", SPELL_DETECT_MAGIC, 1, 45},
    {"disintegrate", SPELL_DISINTEGRATE, 48, 45},
    {"dispel magic", SPELL_DISPEL_MAGIC, 9, 45},
    {"enchant armor", SPELL_ENCHANT_ARMOR, 16, 45},
    {"enchant weapon", SPELL_ENCHANT_WEAPON, 14, 45},
    {"energy drain", SPELL_ENERGY_DRAIN, 22, 45},
    {"faerie fire", SPELL_FAERIE_FIRE, 5, 45},
    {"faerie fog", SPELL_FAERIE_FOG, 16, 45},
    {"fear", SPELL_FEAR, 8, 45},
    {"feeblemind", SPELL_FEEBLEMIND, 34, 45},
    {"find familiar", SPELL_FAMILIAR, 2, 45},
    {"fireball", SPELL_FIREBALL, 25, 45},
    {"fireshield", SPELL_FIRESHIELD, 40, 45},
    {"fly", SPELL_FLY, 11, 45},
    {"group fly", SPELL_FLY_GROUP, 24, 45},
    {"gust of wind", SPELL_GUST_OF_WIND, 10, 45},
    {"haste", SPELL_HASTE, 23, 45},
    {"ice storm", SPELL_ICE_STORM, 11, 45},
    {"identify", SPELL_IDENTIFY, 10, 45},
    {"incendiary cloud", SPELL_INCENDIARY_CLOUD, 45, 45},
    {"infravision", SPELL_INFRAVISION, 8, 45},
    {"invisibility", SPELL_INVISIBLE, 4, 45},
    {"knock", SPELL_KNOCK, 3, 45},
    {"know alignment", SPELL_KNOW_ALIGNMENT, 7, 45},
    {"know monster", SPELL_KNOW_MONSTER, 9, 45},
    {"lightning bolt", SPELL_LIGHTNING_BOLT, 10, 45},
    {"magic missile", SPELL_MAGIC_MISSILE, 1, 45},
    {"major track", SPELL_MAJOR_TRACK, 20, 45},
    {"meteor swarm", SPELL_METEOR_SWARM, 36, 45},
    {"minor creation", SPELL_MINOR_CREATE, 8, 45},
    {"minor invulnerability", SPELL_GLOBE_MINOR_INV, 20, 45},
    {"minor track", SPELL_MINOR_TRACK, 12, 45},
    {"monsum five", SPELL_MON_SUM_5, 15, 45},
    {"monsum four", SPELL_MON_SUM_4, 12, 45},
    {"monsum one", SPELL_MON_SUM_1, 4, 45},
    {"monsum seven", SPELL_MON_SUM_7, 22, 45},
    {"monsum six", SPELL_MON_SUM_6, 18, 45},
    {"monsum three", SPELL_MON_SUM_3, 9, 45},
    {"monsum two", SPELL_MON_SUM_2, 7, 45},
    {"mount", SPELL_MOUNT, 47, 45},
    {"paralyze", SPELL_PARALYSIS, 20, 45},
    {"polymorph self", SPELL_POLY_SELF, 9, 45},
    {"portal", SPELL_PORTAL, 43, 45},
    {"power word blind", SPELL_PWORD_BLIND, 16, 45},
    {"power word kill", SPELL_PWORD_KILL, 23, 45},
    {"prismatic spray", SPELL_PRISMATIC_SPRAY, 17, 45},
    {"refresh", SPELL_REFRESH, 6, 45},
    {"scare", SPELL_SCARE, 4, 45},
    {"second wind", SPELL_SECOND_WIND, 20, 45},
    {"sending", SPELL_SENDING, 1, 45},
    {"shield", SPELL_SHIELD, 1, 45},
    {"shocking grasp", SPELL_SHOCKING_GRASP, 2, 45},
    {"silence", SPELL_SILENCE, 21, 45},
    {"sleep", SPELL_SLEEP, 3, 45},
    {"slowness", SPELL_SLOW, 19, 45},
    {"stone skin", SPELL_STONE_SKIN, 26, 45},
    {"strength", SPELL_STRENGTH, 6, 45},
    {"succor", SPELL_SUCCOR, 24, 45},
    {"summon", SPELL_SUMMON, 27, 45},
    {"teleport", SPELL_TELEPORT, 17, 45},
    {"teleport without error", SPELL_TELEPORT_WO_ERROR, 30, 45},
    {"ventriloquate", SPELL_VENTRILOQUATE, 1, 45},
    {"water breath", SPELL_WATER_BREATH, 9, 45},
    {"weakness", SPELL_WEAKNESS, 6, 45},
    {"web", SPELL_WEB, 11, 45},
    {"wizardeye", SPELL_WIZARDEYE, 35, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainpaladinskills[] = {
    {"aura of power", SPELL_AURA_POWER, 15, 45},
    {"call steed", SKILL_STEED, 11, 45},
#if 0
    {"circle of protection", SPELL_CIRCLE_PROTECTION ,1},
#endif
    {"enlightenment", SPELL_ENLIGHTENMENT, 25, 45},
    {"holy armor", SPELL_HOLY_ARMOR, 5, 45},
    {"holy strength", SPELL_HOLY_STRENGTH, 3, 45},
    {"None", -1, -1, -1}
};
const struct skillset paladinskills[] = {
    {"bash", SKILL_BASH, 1, 45},
    {"blessing", SKILL_BLESSING, 1, 45},
    {"calm", SPELL_CALM, 18, 45},
    {"charge", SKILL_CHARGE, 4, 45},
    {"cure light", SPELL_CURE_LIGHT, 9, 45},
    {"cure serious", SPELL_CURE_SERIOUS, 45, 45},
    {"disarm", SKILL_DISARM, 1, 45},
    {"dodge", SKILL_DODGE, 1, 45},
    {"heroic rescue", SKILL_HEROIC_RESCUE, 1, 45},
    {"know alignment", SPELL_KNOW_ALIGNMENT, 5, 45},
    {"lay on hands", SKILL_LAY_ON_HANDS, 1, 45},
    {"pacifism", SPELL_PACIFISM, 7, 45},
    {"protection from evil", SPELL_PROTECT_FROM_EVIL, 20, 45},
    {"protection from evil group", SPELL_PROT_FROM_EVIL_GROUP, 35, 45},
    {"remove paralysis", SPELL_REMOVE_PARALYSIS, 15, 45},
    {"remove poison", SPELL_REMOVE_POISON, 40, 45},
    {"second wind", SPELL_SECOND_WIND, 35, 45},
    {"slow poison", SPELL_SLOW_POISON, 10, 45},
    {"spot", SKILL_SPOT, 1, 45},
    {"switch opponents", SKILL_SWITCH_OPP, 1, 45},
    {"turn", SPELL_TURN, 10, 45},
    {"warcry", SKILL_HOLY_WARCRY, 5, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainrangerskills[] = {
    {"commune", SPELL_COMMUNE, 20, 45},
    {"giant growth", SPELL_GIANT_GROWTH, 20, 45},
    {"None", -1, -1, -1}
};
const struct skillset rangerskills[] = {
    {"animal friendship", SPELL_ANIMAL_FRIENDSHIP, 3, 45},
    {"animal growth", SPELL_ANIMAL_GROWTH, 35, 45},
    {"animal summon one", SPELL_ANIMAL_SUM_1, 20, 45},
    {"bash", SKILL_BASH, 1, 45},
    {"climb", SKILL_CLIMB, 1, 45},
    {"detect poison", SPELL_DETECT_POISON, 3, 45},
    {"disarm", SKILL_DISARM, 1, 45},
    {"dodge", SKILL_DODGE, 1, 45},
    {"doorbash", SKILL_DOORBASH, 1, 45},
    {"dual wield", SKILL_DUAL_WIELD, 1, 45},
    {"faerie fire", SPELL_FAERIE_FIRE, 7, 45},
    {"find food", SKILL_FIND_FOOD, 1, 45},
    {"find traps", SPELL_FIND_TRAPS, 13, 45},
    {"find water", SKILL_FIND_WATER, 1, 45},
    {"first aid", SKILL_FIRST_AID, 1, 45},
    {"goodberry", SPELL_GOODBERRY, 10, 45},
    {"hide", SKILL_HIDE, 2, 45},
    {"hunt", SKILL_HUNT, 1, 45},
    {"invis to animals", SPELL_INVIS_TO_ANIMALS, 8, 45},
    {"protection from evil", SPELL_PROTECT_FROM_EVIL, 25, 45},
    {"protection from evil group", SPELL_PROT_FROM_EVIL_GROUP, 45, 45},
    {"ration", SKILL_RATION, 1, 45},
    {"remove poison", SPELL_REMOVE_POISON, 20, 45},
    {"rescue", SKILL_RESCUE, 3, 45},
    {"second wind", SPELL_SECOND_WIND, 30, 45},
    {"sending", SPELL_SENDING, 1, 45},
    {"slow poison", SPELL_SLOW_POISON, 10, 45},
    {"snare", SPELL_SNARE, 5, 45},
    {"sneak", SKILL_SNEAK, 4, 45},
    {"spot", SKILL_SPOT, 3, 45},
    {"spy", SKILL_SPY, 1, 45},
    {"switch opponents", SKILL_SWITCH_OPP, 1, 45},
    {"tan", SKILL_TAN, 1, 45},
    {"travelling", SPELL_TRAVELLING, 15, 45},
    {"water breath", SPELL_WATER_BREATH, 17, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainpsiskills[] = {
    {"kinolock", SKILL_KINOLOCK, 9, 45},
    {"sense object", SKILL_SENSE_OBJECT, 25, 45},
    {"None", -1, -1, -1}
};
const struct skillset psiskills[] = {
    {"adrenalize", SKILL_ADRENALIZE, 20, 45},
    {"aura sight", SKILL_AURA_SIGHT, 5, 45},
    {"canibalize", SKILL_CANIBALIZE, 2, 45},
    {"clairvoyance", SKILL_CLAIRVOYANCE, 8, 45},
    {"cell adjustment", SKILL_CELL_ADJUSTMENT, 14, 45},
    {"chameleon", SKILL_CHAMELEON, 8, 45},
    {"doorway", SKILL_DOORWAY, 3, 45},
    {"flame shroud", SKILL_FLAME_SHROUD, 15, 45},
    {"great sight", SKILL_GREAT_SIGHT, 11, 45},
    {"hypnosis", SKILL_HYPNOSIS, 2, 45},
    {"levitation", SKILL_LEVITATION, 10, 45},
    {"psionic strength", SKILL_PSI_STRENGTH, 6, 45},
    {"meditate", SKILL_MEDITATE, 1, 45},
    {"mind burn", SKILL_MIND_BURN, 5, 45},
    {"mind over body", SKILL_MIND_OVER_BODY, 14, 45},
    {"mind wipe", SKILL_MIND_WIPE, 21, 45},
    {"mindblank", SKILL_MINDBLANK, 17, 45},
    {"probability travel", SKILL_PROBABILITY_TRAVEL, 21, 45},
    {"psi invisibility", SKILL_INVIS, 1, 45},
    {"psi portal", SKILL_PORTAL, 32, 45},
    {"psi shield", SKILL_PSI_SHIELD, 1, 45},
    {"psi summon", SKILL_SUMMON, 15, 45},
    {"psionic blast", SKILL_PSIONIC_BLAST, 1, 45},
    {"psionic danger sense", SKILL_DANGER_SENSE, 23, 45},
    {"psionic disintegrate", SKILL_DISINTEGRATE, 38, 45},
    {"psionic teleport", SKILL_PSI_TELEPORT, 12, 30},
    {"psychic crush", SKILL_PSYCHIC_CRUSH, 37, 45},
    {"psychic impersonation", SKILL_PSYCHIC_IMPERSONATION, 3, 45},
    {"scry", SKILL_SCRY, 7, 45},
    {"telekinesis", SKILL_TELEKINESIS, 35, 45},
    {"tower of iron will", SKILL_TOWER_IRON_WILL, 34, 45},
    {"ultra blast", SKILL_ULTRA_BLAST, 20, 45},
    {"None", -1, -1, -1}
};
const struct skillset maindruidskills[] = {
    {"creeping death", SPELL_CREEPING_DEATH, 45, 45},
    {"iron skins", SPELL_IRON_SKINS, 20, 45},
    {"protection acid breath", SPELL_PROT_BREATH_ACID, 40, 45},
    {"protection electric breath", SPELL_PROT_BREATH_ELEC, 39, 45},
    {"protection fire breath", SPELL_PROT_BREATH_FIRE, 40, 45},
    {"protection frost breath", SPELL_PROT_BREATH_FROST, 38, 45},
    {"plant gate", SPELL_PLANT_GATE, 43, 45},
    {"None", -1, -1, -1}
};
const struct skillset druidskills[] = {
    {"animal friendship", SPELL_ANIMAL_FRIENDSHIP, 5, 45},
    {"animal growth", SPELL_ANIMAL_GROWTH, 35, 45},
    {"animal summon one", SPELL_ANIMAL_SUM_1, 15, 45},
    {"animate rock", SPELL_ANIMATE_ROCK, 31, 45},
    {"barkskin", SPELL_BARKSKIN, 3, 45},
    {"call lightning", SPELL_CALL_LIGHTNING, 18, 45},
    {"cause critical", SPELL_CAUSE_CRITICAL, 13, 45},
    {"cause light", SPELL_CAUSE_LIGHT, 2, 45},
    {"cause serious", SPELL_CAUSE_SERIOUS, 8, 45},
    {"chain lightning", SPELL_CHAIN_LIGHTNING, 25, 45},
    {"change form", SPELL_CHANGE_FORM, 12, 45},
    {"changestaff", SPELL_CHANGESTAFF, 30, 45},
    {"charm monster", SPELL_CHARM_MONSTER, 12, 45},
    {"charm vegetable", SPELL_CHARM_VEGGIE, 17, 45},
    {"commune", SPELL_COMMUNE, 20, 45},
    {"conjure elemental", SPELL_CONJURE_ELEMENTAL, 11, 45},
    {"continual light", SPELL_CONT_LIGHT, 16, 45},
    {"control weather", SPELL_CONTROL_WEATHER, 15, 45},
    {"create light", SPELL_LIGHT, 3, 45},
    {"cure critic", SPELL_CURE_CRITIC, 13, 45},
    {"cure light", SPELL_CURE_LIGHT, 2, 45},
    {"cure serious", SPELL_CURE_SERIOUS, 8, 45},
    {"darkness", SPELL_GLOBE_DARKNESS, 3, 45},
    {"detect evil", SPELL_DETECT_EVIL, 6, 45},
    {"detect good", SPELL_DETECT_GOOD, 6, 45},
    {"detect invisibility", SPELL_DETECT_INVISIBLE, 7, 45},
    {"detect magic ", SPELL_DETECT_MAGIC, 5, 45},
    {"detect poison", SPELL_DETECT_POISON, 1, 45},
    {"dispel magic", SPELL_DISPEL_MAGIC, 9, 45},
    {"dust devil", SPELL_DUST_DEVIL, 1, 45},
    {"earth servant", SPELL_EARTH_SERVANT, 36, 45},
    {"entangle", SPELL_ENTANGLE, 16, 45},
    {"faerie fire", SPELL_FAERIE_FIRE, 1, 45},
    {"faerie fog", SPELL_FAERIE_FOG, 10, 45},
    {"find traps", SPELL_FIND_TRAPS, 15, 45},
    {"fire servant", SPELL_FIRE_SERVANT, 35, 45},
    {"fireshield", SPELL_FIRESHIELD, 48, 45},
    {"firestorm", SPELL_FIRESTORM, 22, 45},
    {"flame blade", SPELL_FLAME_BLADE, 7, 45},
    {"fly", SPELL_FLY, 14, 45},
    {"goodberry", SPELL_GOODBERRY, 4, 45},
    {"group fly", SPELL_FLY_GROUP, 22, 45},
    {"heat stuff", SPELL_HEAT_STUFF, 23, 45},
    {"infravision", SPELL_INFRAVISION, 5, 45},
    {"insect growth", SPELL_INSECT_GROWTH, 33, 45},
    {"invis to animals", SPELL_INVIS_TO_ANIMALS, 11, 45},
    {"know alignment", SPELL_KNOW_ALIGNMENT, 2, 45},
    {"major track", SPELL_MAJOR_TRACK, 17, 45},
    {"messenger", SPELL_MESSENGER, 1, 45},
    {"minor track", SPELL_MINOR_TRACK, 7, 45},
    {"mount", SPELL_MOUNT, 15, 45},
    {"poison", SPELL_POISON, 8, 45},
    {"protection from cold", SPELL_PROT_COLD, 20, 45},
    {"protection from electricity", SPELL_PROT_ELEC, 18, 45},
    {"protection from fire", SPELL_PROT_FIRE, 21, 45},
    {"refresh", SPELL_REFRESH, 4, 45},
    {"reincarnate", SPELL_REINCARNATE, 39, 45},
    {"remove paralysis", SPELL_REMOVE_PARALYSIS, 9, 45},
    {"remove poison", SPELL_REMOVE_POISON, 8, 45},
    {"second wind", SPELL_SECOND_WIND, 14, 45},
    {"shillelagh", SPELL_SHILLELAGH, 3, 45},
    {"silence", SPELL_SILENCE, 25, 45},
    {"slow poison", SPELL_SLOW_POISON, 6, 45},
    {"snare", SPELL_SNARE, 8, 45},
    {"speak with plants", SPELL_SPEAK_WITH_PLANT, 7, 45},
    {"sunray", SPELL_SUNRAY, 27, 45},
    {"transport via plant", SPELL_TRANSPORT_VIA_PLANT, 10, 45},
    {"travelling", SPELL_TRAVELLING, 10, 45},
    {"tree", SPELL_TREE, 15, 45},
    {"tree travel", SPELL_TREE_TRAVEL, 8, 45},
    {"true sight", SPELL_TRUE_SIGHT, 24, 45},
    {"turn", SPELL_TURN, 12, 45},
    {"vegetable growth", SPELL_VEGGIE_GROWTH, 20, 45},
    {"warp weapon", SPELL_WARP_WEAPON, 19, 45},
    {"water breath", SPELL_WATER_BREATH, 6, 45},
    {"water servant", SPELL_WATER_SERVANT, 37, 45},
    {"wind servant", SPELL_WIND_SERVANT, 38, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainclericskills[] = {
    {"energy restore", SPELL_ENERGY_RESTORE, 48, 45},
    {"group heal", SPELL_GROUP_HEAL, 29, 45},
    {"protection from drain", SPELL_PROT_ENERGY_DRAIN, 48, 45},
    {"resurrection", SPELL_RESURRECTION, 36, 45},
    {"scribe", SKILL_SCRIBE, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset clericskills[] = {
    {"aid", SPELL_AID, 4, 45},
    {"animate dead", SPELL_ANIMATE_DEAD, 8, 45},
    {"armor", SPELL_ARMOR, 2, 45},
    {"astral walk", SPELL_ASTRAL_WALK, 30, 45},
    {"blade barrier", SPELL_BLADE_BARRIER, 45, 45},
    {"bless", SPELL_BLESS, 1, 45},
    {"blindness", SPELL_BLINDNESS, 14, 45},
    {"cacaodemon", SPELL_CACAODEMON, 29, 45},
    {"call lightning", SPELL_CALL_LIGHTNING, 45, 45},
    {"calm", SPELL_CALM, 2, 45},
    {"cause critical", SPELL_CAUSE_CRITICAL, 10, 45},
    {"cause light", SPELL_CAUSE_LIGHT, 1, 45},
    {"cause serious", SPELL_CAUSE_SERIOUS, 7, 45},
    {"command", SPELL_COMMAND, 1, 45},
    {"comprehend languages", SPELL_COMP_LANGUAGES, 3, 45},
    {"conjure elemental", SPELL_CONJURE_ELEMENTAL, 13, 45},
    {"continual light", SPELL_CONT_LIGHT, 26, 45},
    {"control weather", SPELL_CONTROL_WEATHER, 26, 45},
    {"create food", SPELL_CREATE_FOOD, 5, 45},
    {"create light", SPELL_LIGHT, 2, 45},
    {"create water", SPELL_CREATE_WATER, 2, 45},
    {"cure blind", SPELL_CURE_BLIND, 6, 45},
    {"cure critic", SPELL_CURE_CRITIC, 10, 45},
    {"cure light", SPELL_CURE_LIGHT, 1, 45},
    {"cure serious", SPELL_CURE_SERIOUS, 7, 45},
    {"curse", SPELL_CURSE, 12, 45},
    {"detect evil", SPELL_DETECT_EVIL, 1, 45},
    {"detect good", SPELL_DETECT_GOOD, 1, 45},
    {"detect invisibility", SPELL_DETECT_INVISIBLE, 5, 45},
    {"detect magic", SPELL_DETECT_MAGIC, 3, 45},
    {"dispel evil", SPELL_DISPEL_EVIL, 20, 45},
    {"dispel good", SPELL_DISPEL_GOOD, 20, 45},
    {"dispel magic", SPELL_DISPEL_MAGIC, 10, 45},
    {"dust devil", SPELL_DUST_DEVIL, 3, 45},
    {"earthquake", SPELL_EARTHQUAKE, 15, 45},
    {"faerie fire", SPELL_FAERIE_FIRE, 4, 45},
    {"faerie fog", SPELL_FAERIE_FOG, 11, 45},
    {"find traps", SPELL_FIND_TRAPS, 16, 45},
    {"flamestrike", SPELL_FLAMESTRIKE, 15, 45},
    {"fly", SPELL_FLY, 22, 45},
    {"golem", SPELL_GOLEM, 15, 45},
    {"harm", SPELL_HARM, 25, 45},
    {"heal", SPELL_HEAL, 25, 45},
    {"heroes feast", SPELL_H_FEAST, 24, 45},
    {"identify", SPELL_IDENTIFY, 16, 45},
    {"know alignment", SPELL_KNOW_ALIGNMENT, 4, 45},
    {"locate object", SPELL_LOCATE_OBJECT, 11, 45},
    {"messenger", SPELL_MESSENGER, 1, 45},
    {"poison", SPELL_POISON, 13, 45},
    {"protection acid breath", SPELL_PROT_BREATH_ACID, 40, 45},
    {"protection electric breath", SPELL_PROT_BREATH_ELEC, 39, 45},
    {"protection fire breath", SPELL_PROT_BREATH_FIRE, 40, 45},
    {"protection from cold", SPELL_PROT_COLD, 20, 45},
    {"protection from energy", SPELL_PROT_ENERGY, 19, 45},
    {"protection from electricity", SPELL_PROT_ELEC, 18, 45},
    {"protection from evil", SPELL_PROTECT_FROM_EVIL, 7, 45},
    {"protection from evil group", SPELL_PROT_FROM_EVIL_GROUP, 20, 45},
    {"protection from fire", SPELL_PROT_FIRE, 21, 45},
    {"protection from good", SPELL_PROTECT_FROM_GOOD, 7, 45},
    {"protection from good group", SPELL_PROT_FROM_GOOD_GROUP, 20, 45},
    {"protection frost breath", SPELL_PROT_BREATH_FROST, 38, 45},
    {"protection gas breath", SPELL_PROT_BREATH_GAS, 37, 45},
    {"refresh", SPELL_REFRESH, 3, 45},
    {"remove curse", SPELL_REMOVE_CURSE, 8, 45},
    {"remove paralysis", SPELL_REMOVE_PARALYSIS, 6, 45},
    {"remove poison", SPELL_REMOVE_POISON, 17, 45},
    {"sanctuary", SPELL_SANCTUARY, 26, 45},
    {"second wind", SPELL_SECOND_WIND, 9, 45},
    {"sense life", SPELL_SENSE_LIFE, 4, 45},
    {"silence", SPELL_SILENCE, 23, 45},
    {"succor", SPELL_SUCCOR, 23, 45},
    {"summon", SPELL_SUMMON, 19, 45},
    {"true sight", SPELL_TRUE_SIGHT, 19, 45},
    {"turn", SPELL_TURN, 1, 45},
    {"word of recall", SPELL_WORD_OF_RECALL, 15, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainwarriorskills[] = {
    {"mend", SKILL_MEND, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset warriorskills[] = {
    {"bash", SKILL_BASH, 1, 45},
    {"bellow", SKILL_BELLOW, 1, 45},
#if 0
    {"doorbash", SKILL_DOORBASH, 1}, / *this one learned at ninjamaster */
#endif
    {"kick", SKILL_KICK, 1, 45},
    {"rescue", SKILL_RESCUE, 1, 45},
    {"skin", SKILL_TAN, 1, 45},
    {"None", -1, -1, -1}
};

const struct skillset mainthiefskills[] = {
    {"disarm", SKILL_DISARM, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset thiefskills[] = {
    {"backstab", SKILL_BACKSTAB, 1, 45},
    {"remove trap", SKILL_REMOVE_TRAP, 1, 45},
    {"find trap", SKILL_FIND_TRAP, 1, 45},
    {"hide", SKILL_HIDE, 1, 45},
    {"pick", SKILL_PICK_LOCK, 1, 45},
    {"retreat", SKILL_RETREAT, 1, 45},
    {"sneak", SKILL_SNEAK, 1, 45},
    {"spy", SKILL_SPY, 1, 45},
    {"steal", SKILL_STEAL, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset loreskills[] = {
    /*
     * lores
     */
    {"animal lore", SKILL_CONS_ANIMAL, 1, 94},
    {"demonology", SKILL_CONS_DEMON, 1, 94},
    {"giant lore", SKILL_CONS_GIANT, 1, 94},
    {"necromancy", SKILL_CONS_UNDEAD, 1, 94},
    {"other lore", SKILL_CONS_OTHER, 1, 94},
    {"people lore", SKILL_CONS_PEOPLE, 1, 94},
    {"reptile lore", SKILL_CONS_REPTILE, 1, 94},
    {"vegetable lore", SKILL_CONS_VEGGIE, 1, 94},
    {"read magic", SKILL_READ_MAGIC, 1, 94},
    {"sign language", SKILL_SIGN, 1, 94},
    /*
     * languages
     */
    {"common", LANG_COMMON, 1, 94},
    {"dwarvish", LANG_DWARVISH, 1, 94},
    {"elvish", LANG_ELVISH, 1, 94},
    {"giantish", LANG_GIANTISH, 1, 94},
    {"gnomish", LANG_GNOMISH, 1, 94},
    {"halfling", LANG_HALFLING, 1, 94},
    {"ogre", LANG_OGRE, 1, 94},
    {"orcish", LANG_ORCISH, 1, 94},
    {"None", -1, -1, -1}
};
const struct skillset archerskills[] = {
    {"spot", SKILL_SPOT, 1, 94},
    {"None", -1, -1, -1}
};
/*
 * divided ninjaskills into 3 parts, seems easier
 */
const struct skillset thfninjaskills[] = {
    {"climb", SKILL_CLIMB, 1, 94},
    {"disguise", SKILL_HIDE, 1, 94},
    {"None", -1, -1, -1}
};
const struct skillset warninjaskills[] = {
    {"disarm", SKILL_DISARM, 1, 94},
    {"doorbash", SKILL_DOORBASH, 1, 94},
    {"None", -1, -1, -1}
};
const struct skillset allninjaskills[] = {
    {"riding", SKILL_RIDE, 1, 94},
    {"disengage", SKILL_DISENGAGE, 1, 94},
    {"None", -1, -1, -1}
};
const struct skillset warmonkskills[] = {
    {"dodge", SKILL_DODGE, 1, 94},
    {"retreat", SKILL_RETREAT, 1, 45},
    {"switch opponents", SKILL_SWITCH_OPP, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainmonkskills[] = {
    {"flowerfist", SKILL_FLOWERFIST, 36, 45},
    {"flurry of blows", SKILL_FLURRY, 18, 45},
    {"None", -1, -1, -1}
};
const struct skillset monkskills[] = {
    {"disarm", SKILL_DISARM, 1, 45},
    {"dodge", SKILL_DODGE, 1, 94},
    {"feign death", SKILL_FEIGN_DEATH, 1, 45},
    {"hide", SKILL_HIDE, 1, 45},
    {"kick", SKILL_KICK, 1, 45},
    {"legsweep", SKILL_LEG_SWEEP, 1, 45},
    {"pick lock", SKILL_PICK_LOCK, 1, 45},
    {"quivering palm", SKILL_QUIV_PALM, 30, 45},
    {"retreat", SKILL_RETREAT, 1, 45},
    {"safe fall", SKILL_SAFE_FALL, 1, 94},
    {"sneak", SKILL_SNEAK, 1, 45},
    {"spring leap", SKILL_SPRING_LEAP, 1, 45},
    {"None", -1, -1, -1}
};
const struct skillset mainbarbskills[] = {
    {"None", -1, -1, -1}
};
const struct skillset barbskills[] = {
    {"avoid backattack", SKILL_AVOID_BACK_ATTACK, 1, 45},
    {"bash", SKILL_BASH, 1, 45},
    {"bellow", SKILL_BELLOW, 1, 45},
    {"berserk", SKILL_BERSERK, 1, 45},
    {"camouflage", SKILL_HIDE, 1, 45},      /* 5 */
    {"climb", SKILL_CLIMB, 1, 45},
    {"disarm", SKILL_DISARM, 1, 45},
    {"disguise", SKILL_DISGUISE, 1, 45},
    {"dodge", SKILL_DODGE, 1, 45},
    {"doorbash", SKILL_DOORBASH, 1, 45},    /* 10 */
    {"find food", SKILL_FIND_FOOD, 1, 45},
    {"find water", SKILL_FIND_WATER, 1, 45},
    {"first aid", SKILL_FIRST_AID, 1, 45},
    {"hunt", SKILL_HUNT, 1, 45},
    {"retreat", SKILL_RETREAT, 1, 45},      /* 15 */
    {"skin", SKILL_TAN, 1, 45},
    {"spy", SKILL_SPY, 1, 45},
    {"switch opponents", SKILL_SWITCH_OPP, 1, 45},  /* 18 */
    {"None", -1, -1, -1}
};

const struct skillset mainnecroskills[] = {
    {"bind affinity", SPELL_BIND_AFFINITY, 48, 45},
    {"flesh golem", SPELL_FLESH_GOLEM, 37, 45},
    {"vampiric embrace", SPELL_VAMPIRIC_EMBRACE, 41, 45},
    {"None", -1, -1, -1}
};
const struct skillset necroskills[] = {
    {"animate dead", SPELL_ANIMATE_DEAD, 5, 45},
    {"binding", SPELL_BINDING, 16, 45},
    {"cavorting bones", SPELL_CAVORTING_BONES, 22, 45},
    {"chillshield", SPELL_CHILLSHIELD, 45, 45},
    {"clinging darkness", SPELL_CLINGING_DARKNESS, 5, 45},
    {"cold light", SPELL_COLD_LIGHT, 1, 45},
    {"dark empathy", SPELL_DARK_EMPATHY, 24, 45},
    {"dark pact", SPELL_DARK_PACT, 36, 45},
    {"darkness", SPELL_GLOBE_DARKNESS, 5, 45},
    {"darktravel", SPELL_DARKTRAVEL, 39, 45},
    {"decay", SPELL_DECAY, 18, 45},
    {"detect invisible", SPELL_DETECT_INVISIBLE, 8, 45},
    {"detect magic", SPELL_DETECT_MAGIC, 3, 45},
    {"disease", SPELL_DISEASE, 1, 45},
    {"dispel magic", SPELL_DISPEL_MAGIC, 14, 45},
    {"dominate undead", SPELL_DOMINATE_UNDEAD, 6, 45},
    {"enchant armor", SPELL_ENCHANT_ARMOR, 17, 45},
    {"enchant weapon", SPELL_ENCHANT_WEAPON, 21, 45},
    {"endure cold", SPELL_ENDURE_COLD, 12, 45},
    {"energy drain", SPELL_ENERGY_DRAIN, 15, 45},
    {"eye of the dead", SPELL_EYE_OF_THE_DEAD, 26, 45},
    {"invis to undead", SPELL_INVIS_TO_UNDEAD, 2, 45},
    {"fear", SPELL_FEAR, 4, 45},
    {"feign death", SKILL_FEIGN_DEATH, 1, 45},
    {"finger of death", SPELL_FINGER_OF_DEATH, 25, 45},
    {"fly", SPELL_FLY, 20, 45},
    {"gather shadows", SPELL_GATHER_SHADOWS, 8, 45},
    {"life draw", SPELL_LIFE_DRAW, 15, 45},
    {"life leech", SPELL_LIFE_LEECH, 31, 45},
    {"life tap", SPELL_LIFE_TAP, 2, 45},
    {"major invulnerbility", SPELL_GLOBE_MAJOR_INV, 30, 45},
    {"mend bones", SPELL_MEND_BONES, 10, 45},
    {"minor invulnerbility", SPELL_GLOBE_MINOR_INV, 26, 45},
    {"mist of death", SPELL_MIST_OF_DEATH, 28, 45},
    {"nullify", SPELL_NULLIFY, 23, 45},
    {"numb the dead", SPELL_NUMB_DEAD, 15, 45},
    {"poison", SPELL_POISON, 8, 45},
    {"protection from good", SPELL_PROTECT_FROM_GOOD, 1, 45},
    {"refresh", SPELL_REFRESH, 5, 45},
    {"scourge of the warlock", SPELL_SCOURGE_WARLOCK, 29, 45},
    {"shadow step", SPELL_SHADOW_STEP, 20, 45},
    {"siphon strength", SPELL_SIPHON_STRENGTH, 8, 45},
    {"spectral shield", SPELL_SPECTRAL_SHIELD, 4, 45},
    {"soul steal", SPELL_SOUL_STEAL, 27, 45},
    {"stone skin", SPELL_STONE_SKIN, 32, 45},
    {"suit of bone", SPELL_SUIT_OF_BONE, 3, 45},
    {"teleport without error", SPELL_TELEPORT_WO_ERROR, 35, 45},
    {"trace corpse", SPELL_TRACE_CORPSE, 11, 45},
    {"unsummon", SPELL_UNSUMMON, 7, 45},
    {"weakness", SPELL_WEAKNESS, 9, 45},
    {"None", -1, -1, -1}
};

/*
 * '0=General size,1=tiny, 2=small, 3=medium, 4=large, 5=huge,
 * 6=gargantuan '
 */
const struct race_type race_list[] = {
    {RACE_HALFBREED, "Half-Breed", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_HUMAN, "Human", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_MOON_ELF, "Moon-Elf", 120, 0, 251, 501, 751, 1126, 1501, 2},
    {RACE_DWARF, "Dwarven", 45, 0, 61, 121, 181, 271, 361, 2},
    {RACE_HALFLING, "Halfling", 25, 0, 31, 61, 91, 136, 181, 2},
    {RACE_ROCK_GNOME, "Rock Gnome", 45, 0, 91, 181, 271, 406, 541, 2},
    {RACE_REPTILE, "Reptilian", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_SPECIAL, "Mysterion", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_LYCANTH, "Lycanthropian", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_DRAGON, "Draconian", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_UNDEAD, "Undead", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_ORC, "Orc", 17, 0, 15, 29, 43, 64, 85, 4},
    {RACE_INSECT, "Insectoid", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_ARACHNID, "Arachnoid", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_DINOSAUR, "Saurian", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_FISH, "Icthyiod", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_BIRD, "Avian", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_GIANT, "Giant", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_PREDATOR, "Carnivororous", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_PARASITE, "Parasitic", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_SLIME, "Slime", 17, 0, 21, 41, 61, 91, 121, 1},
    {RACE_DEMON, "Demonic", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_SNAKE, "Snake", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_HERBIV, "Herbivorous", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_TREE, "Tree", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_VEGGIE, "Vegan", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_ELEMENT, "Elemental", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_PLANAR, "Planar", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_DEVIL, "Diabolic", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_GHOST, "Ghostly", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_GOBLIN, "Goblinoid", 17, 0, 15, 29, 43, 64, 85, 2},
    {RACE_TROLL, "Troll", 25, 0, 31, 61, 91, 136, 181, 4},
    {RACE_VEGMAN, "Vegman", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_MFLAYER, "Mindflayer", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_PRIMATE, "Primate", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_ENFAN, "Enfan", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_DROW, "Dark-Elf", 120, 0, 251, 501, 751, 1126, 1501, 4},
    {RACE_GOLEM, "Golem", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_SKEXIE, "Skexie", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_TROGMAN, "Troglodyte", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_PATRYN, "Patryn", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_LABRAT, "Labrynthian", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_SARTAN, "Sartan", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_TYTAN, "Tytan", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_SMURF, "Smurf", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_ROO, "Kangaroo", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_HORSE, "Horse", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_DRAAGDIM, "Ratperson", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_ASTRAL, "Astralion", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_GOD, "God", 1, 0, 555, 666, 777, 888, 999, 0},
    {RACE_GIANT_HILL, "Hill Giant", 50, 0, 151, 226, 301, 356, 401, 5},
    {RACE_GIANT_FROST, "Frost Giant", 50, 0, 151, 226, 301, 376, 451, 5},
    {RACE_GIANT_FIRE, "Fire Giant", 50, 0, 151, 201, 251, 301, 351, 5},
    {RACE_GIANT_CLOUD, "Cloud Giant", 50, 0, 151, 226, 301, 376, 451, 5},
    {RACE_GIANT_STORM, "Storm Giant", 50, 0, 151, 226, 301, 376, 451, 5},
    {RACE_GIANT_STONE, "Stone Giant", 50, 0, 151, 226, 301, 376, 451, 5},
    {RACE_DRAGON_RED, "Red Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_BLACK, "Black Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_GREEN, "Green Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_WHITE, "White Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_BLUE, "Blue Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_SILVER, "Silver Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_GOLD, "Gold Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_BRONZE, "Bronze Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_COPPER, "Copper Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_DRAGON_BRASS, "Brass Dragon", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_UNDEAD_VAMPIRE, "Undead Vampire", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_UNDEAD_LICH, "Undead Lich", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_UNDEAD_WIGHT, "Undead Wight", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_UNDEAD_GHAST, "Undead Ghast", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_UNDEAD_SPECTRE, "Undead Spectre", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_UNDEAD_ZOMBIE, "Undead Zombie", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_UNDEAD_SKELETON, "Undead Skeleton", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_UNDEAD_GHOUL, "Undead Ghoul", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_HALF_ELF, "Half-Elf", 25, 0, 51, 100, 151, 226, 301, 3},
    {RACE_HALF_OGRE, "Half-Ogre", 17, 0, 15, 29, 43, 64, 85, 4},
    {RACE_HALF_ORC, "Half-Orc", 17, 0, 15, 29, 43, 64, 85, 4},
    {RACE_HALF_GIANT, "Half-Giant", 25, 0, 31, 61, 91, 136, 181, 5},
    {RACE_LIZARDMAN, "Lizardman", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_DARK_DWARF, "Dark-Dwarf", 45, 0, 61, 121, 181, 271, 361, 2},
    {RACE_DEEP_GNOME, "Deep-Gnome", 45, 0, 91, 181, 271, 406, 541, 2},
    {RACE_GNOLL, "Gnoll", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_GOLD_ELF, "Gold-Elf", 120, 0, 251, 501, 751, 1126, 1501, 2},
    {RACE_WILD_ELF, "Wild-Elf", 120, 0, 151, 301, 451, 676, 901, 2},
    {RACE_SEA_ELF, "Sea-Elf", 120, 0, 151, 301, 451, 676, 901, 2},
    {RACE_FOREST_GNOME, "Forest-Gnome", 45, 0, 91, 181, 271, 406, 541, 2},
    {RACE_AVARIEL, "Avariel", 120, 0, 151, 301, 451, 676, 901, 3},
    /*
     * start of new Races by greg Hovey (GH)
     */
    {RACE_ETTIN, "Ettin", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_COCKATRICE, "Cockatrice", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_COCODILE, "Cocodile", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_BASILISK, "Basilisk", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_GARGOYLE, "Gargoyle", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_DRIDER, "Drider", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_DISPLACER_BEAST, "Displacer Beast", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_GRIFFON, "Griffon", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_HELL_HOUND, "Hell Hound", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_HYDRA, "Hydra", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_MARGOYLE, "Margoyle", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_MASTODON, "Mastodon", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_MEDUSA, "Medusa", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_MINOTAUR, "Minotaur", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_MOBAT, "Mobat", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_OTYUGH, "Otyugh", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_NEO_OTYUGH, "Neo-Otyugh", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_REMORHAZ, "Remorhaz", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_PURPLE_WORM, "Purple Worm", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_PHASE_SPIDER, "Phase Spider", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_SPHINX, "Sphinx", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_WARG, "Warg", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_WYVERN, "Wyvern", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_UMBER_HULK, "Umber Hulk", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_SLUG, "Slug", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_BULETTE, "Bulette", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_CARRION_CRAWLER, "Carrion Crawler", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_DRACOLISK, "Dracolisk", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_BANSHEE, "Banshee", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_BEHOLDER, "Beholder", 17, 0, 21, 41, 61, 91, 121, 5},
    {RACE_DEATH_TYRANT, "Death Tyrant", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_DRACOLICH, "Dracolich", 17, 0, 21, 41, 61, 91, 121, 6},
    {RACE_RAKSHASA, "Rakshasa", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_BANE_MINION, "Bane Minion", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_ANKHEGS, "Ankhegs", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_BUGBEARS, "Bugbears", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_EFREET, "Efreet", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_HARPIES, "Harpies", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_MANTICORES, "Manticores", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_SCRAGS, "Scrags", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_OWLBEARS, "Owlbears", 17, 0, 21, 41, 61, 91, 121, 2},
    {RACE_SQUID, "Squids", 17, 0, 21, 41, 61, 91, 121, 0},
    {RACE_TIGER, "Tiger", 17, 0, 21, 41, 61, 91, 121, 4},
    {RACE_WIGHTS, "Wights", 17, 0, 21, 41, 61, 91, 121, 3},
    {RACE_YETI, "Yeti", 17, 0, 21, 41, 61, 91, 121, 4}
};

const char     *RaceDesc[132] = {
    "Half-breeds are a mix of 2 different races. **\n",
    "Humans are really the most average of all the races.. They can advance\n"
        "in just about every class and have average stats.\n",

    "   Sharp features such as pointed ears set this race apart from most.\n"
        "They are known to be flightly and not dependable. Elves are "
        "also known to be\n"
        "some of the best magicians around, as well as being on of the most\n"
        "dexterous races.\n",

    "   Wide as a barrel and about as tall. This short stalky race builds\n"
        "the best stone houses/tunnels or anything made of stone around. "
        "Dwarves are\n"
        "known to have very good constitution, but as also known to be very "
        "hard to\n"
        "get along with. If a dwarf does not have a beard a foot long he "
        "must be a\n"
        "baby.\n",

    "  Mini-humans is what they look like, but nothing like humans are\n"
        "they. Halflings like to enjoy good food and lay about. They are "
        "good at most\n"
        "thief skills when born, although they are generally not a evil "
        "race.\n",

    "   Some would say that these guys look like a dwarf with a very large\n"
        "nose, I would not say that to a dwarf though. Gnomes are known "
        "to be very\n"
        "intelligent, but are not wise at all.\n",

    "Reptiles are cold blooded and have scales.\n",

    "Unknown\n",

    "Lycanthropes are humans who can transform themselves to resemble normal\n"
        "animals or monsters.\n",

    "Dragons arn't meant to be toyed with.  Dragons are huge scaled reptile "
        "like\n"
        "creatures with various affects and types.\n",

    "Undead creatures are generally monsters brought back from the dead "
        "by an evil\n"
        "mage of some sort.\n",

    "Orcs are one of the most common, and disliked, of the demihuman races.\n"
        "They are agressive and warlike.\n",

    "Insects are the heartiest and the most numerous of creatures. Normal "
        "insects \n"
        "are found almost everywhere. The giant variety, with added brawn "
        "and power, \n"
        "make tough opponents.\n",

    "Spiders and Arachnids are aggressive predators, dwelling both "
        "above and below \n"
        "ground. Most listed here are poisonous and bite prey first, "
        "because unconscious\n"
        "victims are easier to carry to a lair.\n",

    "Dinosaurs are found on alternate planes of existence, or even on "
        "lost continents.\n"
        "They are very rare.\n",

    "Any of numerous cold-blooded aquatic vertebrates, characteristically "
        "having fins,\n"
        "gills, and a streamlined body. \n",

    "Avians, whether magical or mundane in nature, are among the most "
        "interesting creatures\n"
        "ever to evolve. Their unique physiology sets them apart from "
        "all other life, and their\n"
        "grace and beauty have earned them a place of respect and adoration "
        "in the tales of\n"
        "many races.\n",

    "These species vary greatly n power and intelligences.There are "
        "different \n"
        "types of giants out there.\n",

    "Not much are know about these Predator creatures except for there "
        "desire to\n"
        "hunt out and feeding on the living.\n",

    "An organism that grows, feeds, and is sheltered on or in a "
        "different organism \n"
        "while contributing nothing to the survival of its host.\n",

    "Slime is a strain of monstrous plant life, that grows while "
        "clinging to \n"
        "ceilings. Slime favors moist, subterranean regions. It feeds "
        "on whatever \n"
        "animal, vegetable, or metallic substances happen to cross its "
        "path. \n",

    "Demons are evil vial creatures that are pretty dangerous to play with.\n",

    "Snakes vary in size and slay their prey with deadly venom.  "
        "Neutralize poison\n"
        "counters snake bites\n",

    "They like to eat grass and plants.\n",

    "A perennial woody plant having a main trunk and usually a "
        "distinct crown.\n",

    "A perennial plant gone bad.\n",

    "Elementals are strong, but relatively stupid being conjured up "
        "from their \n"
        "normal habitat on the elemental planes.\n",

    "Planar species are normally found inhabiting on the elemental planes.\n",

    "Devils are extremely evil creatures and shouldn't be taken lightly. "
        "Goes by a\n"
        "varity of names.\n",

    "Ghosts are the spirits of humans who were either so greatly evil "
        "in life \n"
        "or whose deaths were so unusually emotional they have been "
        "cursed with the \n"
        "gift of undead status. Thus, they roam about at night or in "
        "places of darkness.\n"
        "These spirits hate goodness and life, hungering to draw the "
        "living essences from\n"
        "the living.\n",

    "Goblins are small, nasty humanoids... They would be no concern "
        "except for\n"
        "there great numbers.\n",

    "Trolls are ravenous horrors found in almost all climates and "
        "locales.  They \n"
        "attack with their diry, clawed hands and must be killed "
        "quickly because they \n"
        "regenerate hit points.\n",

    "Not much is know about the Vegman race.\n",

    "Not much is know about the MindFlayr race.**\n",

    "Primates that are characterized by long arms and legs, large "
        "dog-like muzzles, \n"
        "and sharp canine teeth.\n",

    "Not much is know about the Enfan race.\n",

    "   The most evil race of elves, these live under the ground and rarely\n"
        "tunnel out to the surface. The sun and any light hurts their "
        "eyes extremely\n"
        "so. Generally drow have black colored skin and white hair.\n",

    "Golems are magically created automatons of great power.  Golems can be \n"
        "construced of flesh, clay, stone ,or iron - all are dangerous.\n",

    "Skexies are weird birdlike creatures.\n",

    "Troglodytes are a warlike race of carnivorous reptilian humanoids "
        "that dwell\n"
        "in natural subterranean caverns and in the cracks and crevices "
        "of mountains.\n"
        "They hate man above all other creatures and often launch bloody "
        "raids on human \n"
        "communities in search of food and steel. \n",

    "Not much is know about the Patryn race.\n",

    "Giant rats are ferocious rodents the size of large dogs.\n",

    "Not much is know about the Sartan race.\n",

    "Not much is know about the Tytan race.\n",

    "Are small little blue creatures that like to sing and "
        "dance.. (Don't ask)\n",

    "Not much is know about the Roo race.\n",

    "A large hoofed mammal having a short-haired coat, a long mane, "
        "and a long tail,\n"
        "domesticated since ancient times and used for riding.\n",

    "Not much is known about this race.\n",

    "Not much is know about these astral creatures. **\n",

    "An immortal creature of the realm, generally unkillable and human "
        "like in appearance.\n",

    "Hill giants are one of the smaller of the giant races, they are "
        "brutish hulk\n"
        "possessing low intelliences and tremendous strength.\n",

    "These giants have a reputation for rudeness and stupididtiy.  While "
        "the reputation\n"
        "may be deserved, frost giants are crafty skilled fighters.\n",

    "Fire giants are brutal and ruthless warriors who resemble huge "
        "dwarves and \n"
        "have flaming red or orange hair and coal black skin.\n",

    "These members of the giant races consider themselves to be above "
        "all others of \n"
        "the species, except storm giants, whom they view as equals.\n",

    "The most noble and intelligent of the giant races.  These giants "
        "are dangerous \n"
        "fighters when angry, and can often use magic.\n",

    "Stone giants are lean, but muscular. Their hard, hairless flesh is "
        "smooth and gray,\n"
        "making it easy for them to blend in with their mountainous "
        "surroundings. Their gaunt\n"
        "facial features and deep, sunken black eyes make them seem "
        "perpetually grim.\n",

    "Red dragons can exhale great spouts of flame or attack with their "
        "claws and fangs.\n",

    "Black dragons are abusive, quick to anger, and resent intrusions "
        "of any kind. They \n"
        "like dismal surroundings, heavy vegetation, and prefer darkness "
        "to daylight. Although\n"
        "not as intelligent as other dragons, black dragons are "
        "instinctively cunning \n"
        "and malevolent.\n",

    "Green dragons are bad tempered, mean, cruel, and rude. They hate "
        "goodness and \n"
        "good-aligned creatures. They love intrigue and seek to enslave "
        "other woodland \n"
        "creatures, killing those who cannot be controlled or intimidated.\n",

    "Unique among dragons in their preferences for cold climates, these "
        "evil beast \n"
        "can attack with their freezing cold breath in addition to razor "
        "sharp claws and \n"
        "fangs.  Smaller in size and not as intelligent as their cousins,  "
        "these dragons are \n"
        "still quite dangerous.\n",

    "Blue dragons are extremely territorial and voracious. They love to "
        "spend long hours \n"
        "preparing ambushes for herd animals and unwary travelers, "
        "and they spend equally long \n"
        "hours dwelling on their success and admiring their trophies. \n",

    "Silver dragons are kind and helpful. They will cheerfully assist "
        "good creatures if \n"
        "their need is genuine. They often take the forms of kindly old "
        "men or fair damsels \n"
        "when associating with people. \n",

    "Gold dragons are wise, judicious, and benevolent. They often embark "
        "on self-appointed\n"
        "quests to promote goodness, and are not easily distracted from "
        "them. They hate \n"
        "injustice and foul play. A gold dragon frequently assumes human "
        "or animal guise and \n"
        "usually will be encountered disguised.\n",

    "Bronze dragons are inquisitive and fond of humans and demi-humans. "
        "They enjoy \n"
        "polymorphing into small, friendly animals so they can "
        "unobtrusively observe humans and \n"
        "demi-humans, especially adventurers. Bronze dragons thrive on "
        "simple challenges such \n"
        "as riddles and harmless contests. They are fascinated by warfare "
        "and will eagerly join \n"
        "an army if the cause is just and the pay is good.\n",

    "Copper dragons are incorrigible pranksters, joke tellers, and riddlers. "
        "They are prideful\n"
        "and are not good losers, although they are reasonably good winners. "
        "They are particularly\n"
        "selfish, and greedy for their alignment, and have an almost "
        "neutral outlook where wealth \n"
        "is concerned.\n",

    "Brass dragons are great talkers, but not particularly good "
        "conversationalists. They are \n"
        "egotistical and often boorish. They oftern have useful "
        "information, but will divulge it\n"
        "only after drifting off the subject many times and after hints "
        "that a gift would be\n"
        "appreciated.\n",

    "Theses undead feed on the blood of the living.  Often they are "
        "indistinguishable\n"
        "from humans, and they maintain abilies they possessed in life "
        "(Inlucding sometimes\n"
        "spellcasting).  Vampires can only be hit by magical weapons.\n",

    "This is perhaps the single most powerful type of undead creature.  A "
        "lich is the\n"
        "remains of a powerful magic user who has kept his body animated "
        "after death \n"
        "through the use of foul magics.  Liches can use magic as they did "
        "while still \n"
        "living, and have other powers similar to greater undead "
        "creatures.\n",

    "These undead creatues have burning eyes set in mummified features.  "
        "They feed\n"
        "on adventures' life essences and can only be hit by silver or "
        "magical weapons.\n",

    "These creatures are so like ghouls as to be completely "
        "indistinguishable from them,\n"
        "and they are usually found only with a pack of ghouls. When a "
        "pack of ghouls and \n"
        "ghasts attacks it will quickly become evident that ghasts "
        "are present, for they\n"
        "exude a carrion stench in a 10' radius which causes retching "
        "and nausea.\n",

    "These undead spirits haunt the most desolate and deserted places.  They\n"
        "attack all living creatues with mindless rage, drain life levels, "
        "and can \n"
        "only be hit by magical weapons.\n",

    "Zombies are mindless, animated undead controlled by evil wizards or "
        "clerics.\n"
        "While more dangerous than skeletons, they move very slowly and can "
        "be damaged\n"
        "by holy water.\n",

    "These undead are one of the weaker types.  Skeletons are animated by "
        "evil wizards\n"
        "or clerics and are often used as guardains or warriors.\n",

    "Ghouls are undead creatues who feed on the flesh of those who are "
        "still living.\n"
        "Their attacks can paralyze all races of characetsrs except elves.\n",

    "   A Half breed of human and elf, this race calls no place home and\n"
        "tries to make anywhere s/he stays their home. A Half elf can take "
        "on the\n"
        "qualities of either of it human or elvish parents.\n",

    "   Half Ogres are huge harry human looking race. They are not very\n"
        "smart nor very quick moving about, however they are strong "
        "and healthy.\n",

    "   Half human and half orc, this guy looks like a very very ugly human.\n"
        "Not very sure where to live these guys can be very dangerous or very\n"
        "helpful.\n",

    "Race of half giant and half human, they look more human than anything\n"
        "and can often be confused as a large human. Half-Giants are known "
        "for their\n"
        "high strength and constitution.\n",

    "Lizard men are savage reptilian humanoids.  They generally attack "
        "in groups.\n",

    "   Wide as a barrel and about as tall. This short stalky race builds\n"
        "the best stone houses/tunnels or anything made of stone around. "
        "Dwarves are\n"
        "known to have very good constitution, but as also known to be "
        "very hard to\n"
        "get along with. If a dwarf does not have a beard a foot long he "
        "must be a\n"
        "baby.\n",

    "Small cousins of the dwarves, gnomes are friendly but reticent, "
        "quick to help\n"
        "their friends but rarely seen by other races unless they want "
        "to be. They tend\n"
        "to dwell underground in hilly, wooded regions where they can "
        "pursue their \n"
        "interests in peace.\n",

    "Gnolls are large, evil, hyena-like humanoids that roam in loosely "
        "organized \n"
        "packs.  They attack by overwhelming unwary victims with their "
        "numbers.\n",

    "   Sharp features such as pointed ears set this race apart from most.\n"
        "They are known to be flightly and not dependable. Elves are also "
        "known to be\n"
        "some of the best magicians around, as well as being on of the most\n"
        "dexterous races.\n",

    "   Sharp features such as pointed ears set this race apart from most.\n"
        "They are known to be flightly and not dependable. Elves are also "
        "known to be\n"
        "some of the best magicians around, as well as being on of the most\n"
        "dexterous races.\n",

    "   Sharp features such as pointed ears set this race apart from most.\n"
        "They are known to be flightly and not dependable. Elves are also "
        "known to be\n"
        "some of the best magicians around, as well as being on of the most\n"
        "dexterous races.\n",

    "   Some would say that these guys look like a dwarf with a very large\n"
        "nose, I would not say that to a dwarf though. Gnomes are known to "
        "be very\n"
        "intelligent, but are not wise at all.\n",

    "  Avariel are a sub-species of elf, blessed with a pair of feathered\n"
        "wings. These wings are attached between their shoulder blades and "
        "range in\n"
        "color from white to black, with most being some shade of tufted "
        "grey.\n"
        "These wings are somewhat vulnerable to fire, and as a result "
        "Avariel tend\n"
        "to be somewhat paranoid about fire wielding mages and other natural\n"
        "sources of flame.\n",

    "These foes look like giant two headed orcs.  They have great strength "
        "and \n"
        "can wield two spiked clubs that inflict terrible damage in combat.\n",

    "A repulsive creature that appears as part cock, part lizard.  They have "
        "the\n"
        "power to turn flesh to stone. \n",

    "Large reptilian carnivores with big Teeth!!\n",

    "Reptilian monsters whose very gaze can turn to stone any fleshy "
        "creature.\n",

    "Ferocious predators of a magical nature.  Gargoyles are typically found \n"
        "amid ruins or dwellins in underground caverns.\n",

    "Part dark-efl, part spider monster.  This horror is the subterranean \n"
        "counterpart of the centaur.\n",

    "Displacer beasts resemble a sixlegged puma with two ebony tentacles "
        "growing \n"
        "from behind its shoulders.  These beasts have the magical "
        "ability to displace\n"
        "their image about three feet from their actual body, making "
        "them especially \n"
        "tricky opponents.\n",

    "Half-lion, half-eagle avian carnivores.  Their favorite prey is "
        "horses and \n"
        "their distant kin.\n",

    "Theses other-planar creatures resemble wolves, but they can breathe "
        "fire and \n"
        "detect invisible enemies.\n",

    "Immense reptillian monsters with multiple heads.  All of its heads "
        "must be \n"
        "severed before a hydra can be slain.  Hydras come in many sizes, "
        "with an \n"
        "increasing number of heads as they grow stronger.\n",

    "Stony monsters which immune to normal weapons and can attack many "
        "times with \n"
        "their sharp claws and spikes.\n",

    "Large cold climate relatives of the elephants found in warmer "
        "regions.\n",

    "These are hideous womencreatures with coilng masses of snakes for hair. \n"
        "They can turn a person to stone with thir gaze.\n",

    "These creature are part-man and part-bull warriors.  They are "
        "highly intelligent\n"
        "and dangerous opponents.\n",

    "Theses are huge omnivorous bats who like nothing better than "
        "warm blooded \n"
        "humanoids for dinner.\n",

    "These scavengers have long tentacles that they use to scoop "
        "trash into their\n"
        "cavernous mouths.\n",

    "This is a more powerful form of Otyugh.  These disgusting scavengers "
        "have several\n"
        "vicious attacks and a heavily armored body.\n",

    "these are sometimes referred to as polar woarms. They inhabit "
        "cold regions and are \n"
        "aggressive predators who have been known to attack even "
        "frost giants.\n",

    "These enormous carnivores burrow through solid ground in search "
        "of small(Man-sized)\n"
        "morsels.\n",

    "These are giant, poisonous spiders with the ability to phase "
        "in and out of this\n"
        "dimension.  They are 'phased in' until they attack and "
        "are 'phased out' afterward.\n",

    "An extremely rare creature that is part-lion, and has the "
        "upper torso of a woman. \n"
        "Rather than fight, sphinxes will often converse with "
        "the adventurers.\n",

    "Large, vicous wolves.\n",

    "These creatures are distant relativies of dragons.  They "
        "attack by biting and using\n"
        "the poisonous sting in their tail.\n",

    "These powerful subterranean creatures can use their claws "
        "to burrow through solid \n"
        "stone in search of prey.\n",

    "These omnivorous garden pest are kind of slimy and attack by "
        "biting and can spit a \n"
        "higly corrosive acid.\n",

    "Also called landsharks, these are the results of a mad "
        "mage's experiment.  They are\n"
        "supid, irascible and always hungry.\n",

    "These are giant, segmented creatures whose eight tentacles can "
        "attack once apiece \n"
        "each around.  Carrion crawlers paralyze their victims and "
        "devour them.\n",

    "These creatures are the hybrid offspring of rogue black dragons "
        "and basilisk.  \n"
        "Dracolisk combine a dragon's breath attack with the petrifying "
        "gaze of a basilisk.\n",

    "These evil spirits' keening wail strikes fear into the hearts "
        "of men.  They attack \n"
        "with a chilling touch.\n",

    "Also called eye tyrants or Spheres of Many Eyes, they are "
        "solitary horrors of great\n"
        "power.  Each of the creatures' eyes has a unique magical "
        "pwer, and they are armored \n"
        "with tough chitinous skin.\n",

    "These are a rare and dangerous form of undead beholder.  They "
        "appear as sluggish, \n"
        "wounded beholders, but still possess some of their orignal "
        "magical pwoers.\n",

    "These are powerful undead dragons whose attacks include a breath "
        "weapon and a \n"
        "paralyzing touch.\n",

    "These evils spirits ply victims with illusion and false civility, "
        "but in reality are\n"
        "cunning fighter/magic-users.\n",

    "A creation of Bane.\n",

    "Ankhegs are burrowing monsters usually found in forests and "
        "farming areas.  They resemble\n"
        "legged worms armed with wicked mandibles and sharply hooked "
        "limbs.\n",

    "Bugbears are giant, hairy cousins of goblins.  They stand about "
        "7' tall and are powerful\n"
        "warriors.\n",

    "Efreet are genies from the elemental plane of fire.  These creatures "
        "are immune to all \n"
        "forms of fire but can hit by other magical attacks.\n",

    "Harpies are wicked avian humanoids thay prey upon nearly all "
        "creatures resembling wolves,\n"
        "but they can breathe fire and can detect invisible enemies.\n",

    "Manticores are a strage mix: They have a lions torso and legs, "
        "bats wings, a spiked tipped\n"
        "tail and a human head.  They fire voileys of spikes from their "
        "tails and have a great \n"
        "appetite for human flesh.\n",

    "Scrags, also called river trolls, live in waterways and are related "
        "to trolls.They\n"
        "perhaps the most feared of all troll-kind.\n",

    "Owlbears are thought to be a hybrid created by some long-forgotten "
        "wizard.  They have \n"
        "the body of a large bear, and the head and slashing beak of a "
        "giant owl.   Owlbears \n"
        "are vicious, evil tempered, and ravenous.\n",

    "Squids are dangerous varieties of the smaller sea creatures.  They "
        "are known to attack \n"
        "travelers on the Trackless Sea.\n",

    "Tigers are carnivourous predators that are sometimes trained for "
        "combat.\n",

    "Theses evil undead spirits feed on adventures life essences and, "
        "like wights, can\n"
        "only be hit by silver or magical weapons.\n",

    "Yetis are large creatures that inhabit ice regions.  Their white "
        "fur makes them nearly \n"
        "invisible.  Oftentimes, you cannont spot them until they are "
        "within 10 to 30 feet of you.\n"
};

const char     *AttackType[] = {
    "Smite",
    "Stab",                     /* 1 */
    "Whip",
    "Slash",
    "Smash",
    "Cleave",
    "Crush",
    "Pound",
    "Claw",
    "Bite",
    "Sting",                    /* 10 */
    "Pierce",
    "Blast",
    "Impale",
    "Range Weapon",
#if 0
    "Unknown", /* 15 */
    "Unknown", /* 16 */
#endif
    "Jab",
    "Punch",
    "Strike",
    "Undefined",                /* test for 0 */
    "\n"
};

const struct affect_list oedit_list[] = {
    {"Blindness", AFF_BLIND, 1},
    {"Curse", AFF_CURSE, 1},
    {"Detect Invisible", AFF_DETECT_INVISIBLE, 1},
    {"Detect Magic", AFF_DETECT_MAGIC, 1},
    {"Detect Evil", AFF_DETECT_EVIL, 1},
    {"Detect Good", AFF2_DETECT_GOOD, 2},
    {"Sense Life", AFF_SENSE_LIFE, 1},
    {"Infravision", AFF_INFRAVISION, 1},
    {"True Sight", AFF_TRUE_SIGHT, 1},
    {"Invisibility", AFF_INVISIBLE, 1},
    {"Sneak", AFF_SNEAK, 1},
    {"Hide", AFF_HIDE, 1},
    {"Prot. from Evil", AFF_PROTECT_FROM_EVIL, 1},
    {"Prot. from Good", AFF2_PROTECT_FROM_GOOD, 2},
    {"Growth", AFF_GROWTH, 1},
    {"Sanctuary", AFF_SANCTUARY, 1},
    {"Fireshield", AFF_FIRESHIELD, 1},
    {"Flying", AFF_FLYING, 1},
    {"Travelling", AFF_TRAVELLING, 1},
    {"Tree Travel", AFF_TREE_TRAVEL, 1},
    {"Water Breath", AFF_WATERBREATH, 1},
    {"Poison", AFF_POISON, 1},
    {"Sleep", AFF_SLEEP, 1},
    {"Silence", AFF_SILENCE, 1},
    {"Paralysis", AFF_PARALYSIS, 1},
    {"Scrying", AFF_SCRYING, 1},
    {"Telepathy", AFF_TELEPATHY, 1},
    {"Berserk", AFF2_BERSERK, 2},
    {"Invis to Animals", AFF2_ANIMAL_INVIS, 2}
};

const struct skillset styleskillset[] = {
    {"standard", STYLE_STANDARD, 1, 94},
    {"berserked", STYLE_BERSERKED, 10, 94},
    {"aggressive", STYLE_AGGRESSIVE, 10, 94},
    {"defensive", STYLE_DEFENSIVE, 10, 94},
    {"evasive", STYLE_EVASIVE, 10, 94},
    {"None", -1, -1, -1}
};

const struct skillset weaponskills[] = {
    {"short sword", WEAPON_SHORT_SWORD, 1, 45},
    {"long sword", WEAPON_LONG_SWORD, 1, 45},
    {"broadsword", WEAPON_BROADSWORD, 1, 45},
    {"bastard sword", WEAPON_BASTARD_SWORD, 1, 45},
    {"two-handed sword", WEAPON_TWO_HANDED_SWORD, 1, 45},
    {"sickle", WEAPON_SICKLE, 1, 45},
    {"scythe", WEAPON_SCYTHE, 1, 45},
    {"scimitar", WEAPON_SCIMITAR, 1, 45},
    {"rapier", WEAPON_RAPIER, 1, 45},
    {"sabre", WEAPON_SABRE, 1, 45},
    {"katana", WEAPON_KATANA, 1, 45},
    {"wakizashi", WEAPON_WAKIZASHI, 1, 45},
    {"dagger", WEAPON_DAGGER, 1, 45},
    {"knife", WEAPON_KNIFE, 1, 45},
    {"stiletto", WEAPON_STILETTO, 1, 45},
    {"spear", WEAPON_SPEAR, 1, 45},
    {"pike", WEAPON_PIKE, 1, 45},
    {"ranseur", WEAPON_RANSEUR, 1, 45},
    {"naginata", WEAPON_NAGINATA, 1, 45},
    {"halberd", WEAPON_HALBERD, 1, 45},
    {"lucern hammer", WEAPON_LUCERN_HAMMER, 1, 45},
    {"trident", WEAPON_TRIDENT, 1, 45},
    {"fork", WEAPON_FORK, 1, 45},
    {"axe", WEAPON_AXE, 1, 45},
    {"hand axe", WEAPON_HAND_AXE, 1, 45},
    {"two-handed axe", WEAPON_TWO_HANDED_AXE, 1, 45},
    {"pick", WEAPON_PICK, 1, 45},
    {"hammer", WEAPON_HAMMER, 1, 45},
    {"club", WEAPON_CLUB, 1, 45},
    {"great club", WEAPON_GREAT_CLUB, 1, 45},
    {"mace", WEAPON_MACE, 1, 45},
    {"maul", WEAPON_MAUL, 1, 45},
    {"morningstar", WEAPON_MORNINGSTAR, 1, 45},
    {"flail", WEAPON_FLAIL, 1, 45},
    {"nunchaku", WEAPON_NUNCHAKU, 1, 45},
    {"chain", WEAPON_CHAIN, 1, 45},
    {"bostick", WEAPON_BOSTICK, 1, 45},
    {"staff", WEAPON_STAFF, 1, 45},
    {"sai", WEAPON_SAI, 1, 45},
    {"sap", WEAPON_SAP, 1, 45},
    {"bolas", WEAPON_BOLAS, 1, 45},
    {"cestus", WEAPON_CESTUS, 1, 45},
    {"scourge", WEAPON_SCOURGE, 1, 45},
    {"whip", WEAPON_WHIP, 1, 45},
    {"boomerang", WEAPON_BOOMERANG, 1, 45},
    {"shuriken", WEAPON_SHURIKEN, 1, 45},
    {"throwing knife", WEAPON_THROWING_KNIFE, 1, 45},
    {"dart", WEAPON_DART, 1, 45},
    {"javelin", WEAPON_JAVELIN, 1, 45},
    {"harpoon", WEAPON_HARPOON, 1, 45},
    {"sling", WEAPON_SLING, 1, 45},
    {"staff sling", WEAPON_STAFF_SLING, 1, 45},
    {"short bow", WEAPON_SHORT_BOW, 1, 45},
    {"long bow", WEAPON_LONG_BOW, 1, 45},
    {"hand crossbow", WEAPON_HAND_CROSSBOW, 1, 45},
    {"light crossbow", WEAPON_LIGHT_CROSSBOW, 1, 45},
    {"heavy crossbow", WEAPON_HEAVY_CROSSBOW, 1, 45},
    {"blowgun", WEAPON_BLOWGUN, 1, 45},
    {"tetsubo", WEAPON_TETSUBO, 1, 45},
    {"generic", WEAPON_GENERIC, 1, 45},
    {"none", -1, -1, -1}
};

#if 0
/* This string is WAY too long and is unused */
const char      map2[] =
    "************^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^++++++++++++++++++++++      "
    "*************^^^^^^^^^^^II^^^^^^^+++++~~++^^^+++++++++++++++::++++++++++~~~   "
    "**********^^^^^^^^^^^+II::II+++++++++~~+^^^+++I+++++++++++++::+++++++++++~~~~ "
    "******^^^^^^^^^^++++++++::++++^^^^^^~~^^^^^^^^^^^^++++++++II::II++++++++~~~~~~"
    "*****^^^^^^^^+++++++++++::++++^^^^^^~^^^^^^^^^^^^^^::::::::::::::::::::::~~~~ "
    "*****^^^^^^+++++++^^++++::++++++^^^^^^^^^^^^^^^^^^ ::     II::II       ^^~~~~~"
    "^::++++++++++++++^^+++++::+++++^^^^^^^^^^^^^^^^^^^^::I~~~ ^^::        ^^~~~~~ "
    "+:::++++ + + +++++++++++::++++^^^^:^^^^^^^^^^^^^      ~~^^^ ::^^^^^^   ^^^~~~~"
    "+I::I++ + + ++++++++++++::+++++++::++++++++++^^~~~~~~~~^^...::....^^^^^^^^~~~/"
    "++::++++ +:+++++++++++++:: ++++++::++++++++++^^~~~~~~~^^.........  ^^^~~~~~ / "
    "++::+++++::++++::::++III::III++++::++++++++++^^~~~~~~^^...^^........^^~~~  /  "
    "++::+++++::++++++::++II :: II++++::+++++++~+++^^^~~~^^...............^^   /   "
    "++::::::::::::::::::::::::::::::::::::+++~+~+:^^~~^^...............^^^   /    "
    "++++++::+++++++++++++II :: II++++++++++++~~++^^^^~~^^...............^^  /     "
    "++++++::+++++++++++++III::III++++++~~+++~++~~~~~~~^^^^............^^^^ /  N   "
    "~~~~~~~~~~~~~~~~~~~~~~~~::~~~~~~~~~~~~~~~~~~~~~~ ^^^^^^^^^^+++++++^^  /   |   "
    "++~~++::+++~~~~~~~~~~+++::++++++~~~~~~~~~~~~^^^^^^^^^^^^^^^^++++++^  /  W-+-E "
    "++~~++::++++++++++++++++::^^^^ ++++++++++^^^^^^^^^+++I+I+++++++++^^ /     |   "
    "++~~~~::II+++++++^^^^^^ ::^^^+++++++++++++^^^^^^^++++I+I++++++++^^ /      S   "
    "++++~~::II+++++^^^^^++::::::+++++++++++^^^^^^^^^^^^^+++++++++++^^ /           "
    "++++~~::+++++  ^^^^^^^+++++++++++++^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ /   Astral[ ]"
    "++++~~~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^                  /Dimensional[ ]"
    "++++++++++++++^^^^^^^^^                                       /     Unknown[ ]";

#endif

const int       zoneloc[171] = { 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1636,
    883, 960, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    922, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 807, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 180, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792
};

const struct clan clan_list[MAX_CLAN] = {
    {0, "None", "None", "No description", 3005},
    {1, "the Exiled", "exiled", "Exiled from a clan", 3005},
    {2, "the $c000BStorm$c000xCloaks", "{$c000BS$c000xC$c000w}",
        "No clan description set", 22704},
    {3, "the Brothers of Eternal Light", "$c000WBoEL$c000w",
        "No clan description set", 22717},
    {4, "the Order", "{$c000WO$c000w}", "No clan description set", 22737},
    {5, "NewClan1", "N1", "No Clan description", 3005},
    {6, "NewClan2", "N2", "No Clan description", 3005},
    {7, "NewClan3", "N3", "No Clan description", 3005},
    {8, "NewClan4", "N4", "No clan description set", 3005},
    {9, "NewClan5", "N5", "No clan description set", 3005},
};

const char     *languagelist[] = {
    "???",
    "Common",
    "Elvish",
    "Halfling",
    "Dwarvish",
    "Orcish",
    "Giantish",
    "Ogre",
    "Gnomish",
    "All",
    "GodLike"
};

char           *classname[] =
    { "Mu", "Cl", "Wa", "Th", "Dr", "Mo", "Ba", "So", "Pa", "Ra", "Ps",
"Ne" };
/*
 * None, exiles, Stormcloaks, Brothers, Order, Brood, Cult of the dragon,
 * Returners
 */
#if 0
const char *MortalLevel[7][3] = {
    {"$c0008Apprentice","$c0008Apprentice" ,"$c0008Apprentice"},
    {"$c0004Pilgrim" ,"$c0004Pilgrim" ,"$c0004Pilgrim"}, {"$c0006Explorer"
     ,"$c0006Explorer" ,"$c0006Explorer",},
    {"$c0014Adventurer","$c0014Adventurer" ,"$c0014Adventurer"},
    {"$c0015Mystical" ,"$c0015Mystical" ,"$c0015Mystical"}, {"$c000BHero"
     ,"$c000BHero" ,"$c000BHero"}, {"$c000RHero" ,"$c000RHeroine"
     ,"$c000RHero"} };


    case 51:
        sprintf(levels,"Newbie Immortal");
        break;
    case 52:
        sprintf(levels,"Saint");
        break;
    case 53:
        sprintf(levels,"Creator");
        break;
    case 54:
        sprintf(levels,"Demi God");
        break;
    case 55:
        sprintf(levels,"God");
        break;
    case 56:
        sprintf(levels,"Greater God");
        break;
    case 57:
        sprintf(levels,"Overseer");
        break;
    case 58:
        sprintf(levels,"SD Lord");
        break;
    case 59:
        sprintf(levels,"Lord of Realm");
        break;
    case 60:
        sprintf(levels,"God of Realm");
        break;
        Lesser
 /* using new names for levels */
const char *ImmortalLevel[10][3] = {

    {"Supreme Being","Supreme Being","Supreme Being"}, /* 60 */
    {"Lesser Deity","Lesser Deity","Lesser Deity"}, /* 51 */
    {"Deity","Deity","Deity"}, /* 52 */
    {"Greater Deity","Greater Deity","Greater Deity"}, /* 53 */
    {"Lesser God", "Lesser Goddess", "Lesser God"}, /* 54 */
    {"God","Goddess","God"}, /* 55 */
    {"Greater God", "Greater Goddess","Greater God"}, /* 56 */
    {"God of Judgement""Goddess of Judgement", "Judgement Caller"}, /* 57 */
    {"Lord", "Lady", "Thing"}, /* 58 */
    {"Supreme Lady", "Supreme Lord", "Supreme Thing"} /* 59 */
};

#endif
/*
 * xcoord, y coord, Disembark room, embark room
 */
const struct map_coord map_coords[] = {
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {-1, -1, -1, -1}
};

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
