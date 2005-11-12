
#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

#include "protos.h"
#include "externs.h"

#define REGISTER_PROC(name, type) { #name, name, type }

struct spec_proc specProcs[] = {
    REGISTER_PROC( ABShout, PROC_MOBILE ),
    REGISTER_PROC( AbyssGateKeeper, PROC_MOBILE ),
    REGISTER_PROC( AcidBreather, PROC_MOBILE ),
    REGISTER_PROC( acid_monster, PROC_MOBILE ),
    REGISTER_PROC( AGGRESSIVE, PROC_MOBILE ),
    REGISTER_PROC( archer, PROC_MOBILE ),
    REGISTER_PROC( archer_instructor, PROC_MOBILE ),
    REGISTER_PROC( astral_portal, PROC_MOBILE ),
    REGISTER_PROC( athos, PROC_MOBILE ),
    REGISTER_PROC( attack_rats, PROC_MOBILE ),
    REGISTER_PROC( AvatarPosereisn, PROC_MOBILE ),
    REGISTER_PROC( baby_bear, PROC_MOBILE ),
    REGISTER_PROC( banshee, PROC_MOBILE ),
    REGISTER_PROC( Barbarian, PROC_MOBILE ),
    REGISTER_PROC( barbarian_guildmaster, PROC_MOBILE ),
    REGISTER_PROC( Beholder, PROC_MOBILE ),
    REGISTER_PROC( blinder, PROC_MOBILE ),
    REGISTER_PROC( blink, PROC_MOBILE ),
    REGISTER_PROC( board_ship, PROC_MOBILE ),
    REGISTER_PROC( boneshardbreather, PROC_MOBILE ),
    REGISTER_PROC( braxis_swamp_dragon, PROC_MOBILE ),
    REGISTER_PROC( BreathWeapon, PROC_MOBILE ),
    REGISTER_PROC( butcher, PROC_MOBILE ),
    REGISTER_PROC( CaravanGuildGuard, PROC_MOBILE ),
    REGISTER_PROC( CarrionCrawler, PROC_MOBILE ),
    REGISTER_PROC( citizen, PROC_MOBILE ),
    REGISTER_PROC( cleric, PROC_MOBILE ),
    REGISTER_PROC( ClericGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( cleric_specialist_guildmaster, PROC_MOBILE ),
    REGISTER_PROC( coldcaster, PROC_MOBILE ),
    REGISTER_PROC( confusionmob, PROC_MOBILE ),
    REGISTER_PROC( CorsairPush, PROC_MOBILE ),
    REGISTER_PROC( creeping_death, PROC_MOBILE ),
    REGISTER_PROC( cronus_pool, PROC_MOBILE ),
    REGISTER_PROC( DarkBreather, PROC_MOBILE ),
    REGISTER_PROC( death_knight, PROC_MOBILE ),
    REGISTER_PROC( DehydBreather, PROC_MOBILE ),
    REGISTER_PROC( Demon, PROC_MOBILE ),
    REGISTER_PROC( Deshima, PROC_MOBILE ),
    REGISTER_PROC( Devil, PROC_MOBILE ),
    REGISTER_PROC( DispellerIncMob, PROC_MOBILE ),
    REGISTER_PROC( DragonHunterLeader, PROC_MOBILE ),
    REGISTER_PROC( Drow, PROC_MOBILE ),
    REGISTER_PROC( DruidChallenger, PROC_MOBILE ),
    REGISTER_PROC( DruidGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( druid_protector, PROC_MOBILE ),
    REGISTER_PROC( DwarvenMiners, PROC_MOBILE ),
    REGISTER_PROC( elamin, PROC_MOBILE ),
    REGISTER_PROC( Etheral_post, PROC_MOBILE ),
    REGISTER_PROC( fido, PROC_MOBILE ),
    REGISTER_PROC( fighter, PROC_MOBILE ),
    REGISTER_PROC( fighter_mage, PROC_MOBILE ),
    REGISTER_PROC( FightingGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( FireBreather, PROC_MOBILE ),
    REGISTER_PROC( FrostBreather, PROC_MOBILE ),
    REGISTER_PROC( GasBreather, PROC_MOBILE ),
    REGISTER_PROC( geyser, PROC_MOBILE ),
    REGISTER_PROC( ghastsmell, PROC_MOBILE ),
    REGISTER_PROC( ghost, PROC_MOBILE ),
    REGISTER_PROC( ghostsoldier, PROC_MOBILE ),
    REGISTER_PROC( ghoul, PROC_MOBILE ),
    REGISTER_PROC( ghoultouch, PROC_MOBILE ),
    REGISTER_PROC( gnome_collector, PROC_MOBILE ),
    REGISTER_PROC( goblin_chuirgeon, PROC_MOBILE ),
    REGISTER_PROC( goblin_sentry, PROC_MOBILE ),
    REGISTER_PROC( golgar, PROC_MOBILE ),
    REGISTER_PROC( green_slime, PROC_MOBILE ),
    REGISTER_PROC( guardianextraction, PROC_MOBILE ),
    REGISTER_PROC( guild_guard, PROC_MOBILE ),
    REGISTER_PROC( hangman, PROC_MOBILE ),
    REGISTER_PROC( hunter, PROC_MOBILE ),
    REGISTER_PROC( HuntingMercenary, PROC_MOBILE ),
    REGISTER_PROC( idiot, PROC_MOBILE ),
    REGISTER_PROC( janaurius, PROC_MOBILE ),
    REGISTER_PROC( janitor, PROC_MOBILE ),
    REGISTER_PROC( Jessep, PROC_MOBILE ),
    REGISTER_PROC( jugglernaut, PROC_MOBILE ),
    REGISTER_PROC( Keftab, PROC_MOBILE ),
    REGISTER_PROC( lich_church, PROC_MOBILE ),
    REGISTER_PROC( LightBreather, PROC_MOBILE ),
    REGISTER_PROC( LightningBreather, PROC_MOBILE ),
    REGISTER_PROC( lizardman_shaman, PROC_MOBILE ),
    REGISTER_PROC( loremaster, PROC_MOBILE ),
    REGISTER_PROC( lust_sinner, PROC_MOBILE ),
    REGISTER_PROC( mad_cyrus, PROC_MOBILE ),
    REGISTER_PROC( mad_gertruda, PROC_MOBILE ),
    REGISTER_PROC( MageGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( mage_specialist_guildmaster, PROC_MOBILE ),
    REGISTER_PROC( magic_user, PROC_MOBILE ),
    REGISTER_PROC( master_smith, PROC_MOBILE ),
    REGISTER_PROC( mayor, PROC_MOBILE ),
    REGISTER_PROC( medusa, PROC_MOBILE ),
    REGISTER_PROC( mermaid, PROC_MOBILE ),
    REGISTER_PROC( MidgaardCityguard, PROC_MOBILE ),
    REGISTER_PROC( mime_jerry, PROC_MOBILE ),
    REGISTER_PROC( mistgolemtrap, PROC_MOBILE ),
    REGISTER_PROC( moldexplosion, PROC_MOBILE ),
    REGISTER_PROC( monk, PROC_MOBILE ),
    REGISTER_PROC( MonkChallenger, PROC_MOBILE ),
    REGISTER_PROC( monk_master, PROC_MOBILE ),
    REGISTER_PROC( MordGuard, PROC_MOBILE ),
    REGISTER_PROC( MordGuildGuard, PROC_MOBILE ),
    REGISTER_PROC( nadia, PROC_MOBILE ),
    REGISTER_PROC( NecromancerGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( NewThalosGuildGuard, PROC_MOBILE ),
    REGISTER_PROC( NewThalosMayor, PROC_MOBILE ),
    REGISTER_PROC( nightwalker, PROC_MOBILE ),
    REGISTER_PROC( ninja_master, PROC_MOBILE ),
    REGISTER_PROC( NudgeNudge, PROC_MOBILE ),
    REGISTER_PROC( PaladinGuildGuard, PROC_MOBILE ),
    REGISTER_PROC( PaladinGuildmaster, PROC_MOBILE ),
    REGISTER_PROC( portal_regulator, PROC_MOBILE ),
    REGISTER_PROC( PostMaster, PROC_MOBILE ),
    REGISTER_PROC( pridemirror, PROC_MOBILE ),
    REGISTER_PROC( PrisonGuard, PROC_MOBILE ),
    REGISTER_PROC( PrydainGuard, PROC_MOBILE ),
    REGISTER_PROC( PsiGuildmaster, PROC_MOBILE ),
    REGISTER_PROC( QPSalesman, PROC_MOBILE ),
    REGISTER_PROC( QuestMobProc, PROC_MOBILE ),
    REGISTER_PROC( QuestorGOD, PROC_MOBILE ),
    REGISTER_PROC( RangerGuildmaster, PROC_MOBILE ),
    REGISTER_PROC( raven_iron_golem, PROC_MOBILE ),
    REGISTER_PROC( real_fox, PROC_MOBILE ),
    REGISTER_PROC( real_rabbit, PROC_MOBILE ),
    REGISTER_PROC( receptionist, PROC_MOBILE ),
    REGISTER_PROC( regenerator, PROC_MOBILE ),
    REGISTER_PROC( remort_guild, PROC_MOBILE ),
    REGISTER_PROC( RepairGuy, PROC_MOBILE ),
    REGISTER_PROC( Ringwraith, PROC_MOBILE ),
    REGISTER_PROC( RustMonster, PROC_MOBILE ),
    REGISTER_PROC( sageactions, PROC_MOBILE ),
    REGISTER_PROC( sailor, PROC_MOBILE ),
    REGISTER_PROC( shadow, PROC_MOBILE ),
    REGISTER_PROC( shadowtouch, PROC_MOBILE ),
    REGISTER_PROC( shaman, PROC_MOBILE ),
    REGISTER_PROC( ShardBreather, PROC_MOBILE ),
    REGISTER_PROC( silktrader, PROC_MOBILE ),
    REGISTER_PROC( sinpool, PROC_MOBILE ),
    REGISTER_PROC( sin_spawner, PROC_MOBILE ),
    REGISTER_PROC( sisyphus, PROC_MOBILE ),
    REGISTER_PROC( skillfixer, PROC_MOBILE ),
    REGISTER_PROC( Slavalis, PROC_MOBILE ),
    REGISTER_PROC( SleepBreather, PROC_MOBILE ),
    REGISTER_PROC( snake, PROC_MOBILE ),
    REGISTER_PROC( snake_avt, PROC_MOBILE ),
    REGISTER_PROC( snake_guardians, PROC_MOBILE ),
    REGISTER_PROC( SorcGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( SoundBreather, PROC_MOBILE ),
    REGISTER_PROC( starving_man, PROC_MOBILE ),
    REGISTER_PROC( StatTeller, PROC_MOBILE ),
    REGISTER_PROC( StormGiant, PROC_MOBILE ),
    REGISTER_PROC( strahd_vampire, PROC_MOBILE ),
    REGISTER_PROC( strahd_zombie, PROC_MOBILE ),
    REGISTER_PROC( stu, PROC_MOBILE ),
    REGISTER_PROC( SultanGuard, PROC_MOBILE ),
    REGISTER_PROC( sund_earl, PROC_MOBILE ),
    REGISTER_PROC( temple_labrynth_liar, PROC_MOBILE ),
    REGISTER_PROC( temple_labrynth_sentry, PROC_MOBILE ),
    REGISTER_PROC( thief, PROC_MOBILE ),
    REGISTER_PROC( ThiefGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( ThrowerMob, PROC_MOBILE ),
    REGISTER_PROC( timnus, PROC_MOBILE ),
    REGISTER_PROC( tmk_guard, PROC_MOBILE ),
    REGISTER_PROC( tormentor, PROC_MOBILE ),
    REGISTER_PROC( trapper, PROC_MOBILE ),
    REGISTER_PROC( TreeThrowerMob, PROC_MOBILE ),
    REGISTER_PROC( trinketlooter, PROC_MOBILE ),
    REGISTER_PROC( trogcook, PROC_MOBILE ),
    REGISTER_PROC( troguard, PROC_MOBILE ),
    REGISTER_PROC( Tyrannosaurus_swallower, PROC_MOBILE ),
    REGISTER_PROC( Tysha, PROC_MOBILE ),
    REGISTER_PROC( Vaelhar, PROC_MOBILE ),
    REGISTER_PROC( Valik, PROC_MOBILE ),
    REGISTER_PROC( vampire, PROC_MOBILE ),
    REGISTER_PROC( VaporBreather, PROC_MOBILE ),
    REGISTER_PROC( village_princess, PROC_MOBILE ),
    REGISTER_PROC( virgin_sac, PROC_MOBILE ),
    REGISTER_PROC( WarriorGuildMaster, PROC_MOBILE ),
    REGISTER_PROC( web_slinger, PROC_MOBILE ),
    REGISTER_PROC( winger, PROC_MOBILE ),
    REGISTER_PROC( wraith, PROC_MOBILE ),
    REGISTER_PROC( zork, PROC_MOBILE ),
    REGISTER_PROC( mazekeeper, PROC_MOBILE ),
    REGISTER_PROC( mazekeeper_riddle_master, PROC_MOBILE),
    REGISTER_PROC( mazekeeper_riddle_one, PROC_MOBILE ),
    REGISTER_PROC( mazekeeper_riddle_two, PROC_MOBILE ),
    REGISTER_PROC( mazekeeper_riddle_three, PROC_MOBILE ),
    REGISTER_PROC( mazekeeper_riddle_four, PROC_MOBILE ),
    REGISTER_PROC( dragon, PROC_MOBILE ),

    REGISTER_PROC( altarofsin, PROC_OBJECT ),
    REGISTER_PROC( applepie, PROC_OBJECT ),
    REGISTER_PROC( BerserkerItem, PROC_OBJECT ),
    REGISTER_PROC( board, PROC_OBJECT ),
    REGISTER_PROC( chestproc, PROC_OBJECT ),
    REGISTER_PROC( grayswandir, PROC_OBJECT ),
    REGISTER_PROC( mirrorofopposition, PROC_OBJECT ),
    REGISTER_PROC( nodrop, PROC_OBJECT ),
    REGISTER_PROC( portal, PROC_OBJECT ),
    REGISTER_PROC( qp_potion, PROC_OBJECT ),
    REGISTER_PROC( scraps, PROC_OBJECT ),
    REGISTER_PROC( ships_helm, PROC_OBJECT ),
    REGISTER_PROC( SlotMachine, PROC_OBJECT ),
    REGISTER_PROC( soap, PROC_OBJECT ),
    REGISTER_PROC( thunder_black_pill, PROC_OBJECT ),
    REGISTER_PROC( thunder_blue_pill, PROC_OBJECT ),
    REGISTER_PROC( thunder_sceptre_one, PROC_OBJECT ),
    REGISTER_PROC( thunder_sceptre_two, PROC_OBJECT ),
    REGISTER_PROC( trinketcount, PROC_OBJECT ),
    REGISTER_PROC( level_limiter, PROC_OBJECT ),
    REGISTER_PROC( mazekeeper_portal, PROC_OBJECT ),

    REGISTER_PROC( arena_arrow_dispel_trap, PROC_ROOM ),
    REGISTER_PROC( arena_dispel_trap, PROC_ROOM ),
    REGISTER_PROC( arena_fireball_trap, PROC_ROOM ),
    REGISTER_PROC( arena_prep_room, PROC_ROOM ),
    REGISTER_PROC( bahamut_home, PROC_ROOM ),
    REGISTER_PROC( bank, PROC_ROOM ),
    REGISTER_PROC( ChurchBell, PROC_ROOM ),
    REGISTER_PROC( climb_room, PROC_ROOM ),
    REGISTER_PROC( close_doors, PROC_ROOM ),
    REGISTER_PROC( cog_room, PROC_ROOM ),
    REGISTER_PROC( dispel_room, PROC_ROOM ),
    REGISTER_PROC( Donation, PROC_ROOM ),
    REGISTER_PROC( druid_challenge_prep_room, PROC_ROOM ),
    REGISTER_PROC( druid_challenge_room, PROC_ROOM ),
    REGISTER_PROC( dump, PROC_ROOM ),
    REGISTER_PROC( fiery_alley, PROC_ROOM ),
    REGISTER_PROC( Fountain, PROC_ROOM ),
    REGISTER_PROC( gnome_home, PROC_ROOM ),
    REGISTER_PROC( greed_disabler, PROC_ROOM ),
    REGISTER_PROC( guardianroom, PROC_ROOM ),
    REGISTER_PROC( knockproc, PROC_ROOM ),
    REGISTER_PROC( lag_room, PROC_ROOM ),
    REGISTER_PROC( legendfountain, PROC_ROOM ),
    REGISTER_PROC( Magic_Fountain, PROC_ROOM ),
    REGISTER_PROC( Magic_Pool, PROC_ROOM ),
    REGISTER_PROC( monk_challenge_prep_room, PROC_ROOM ),
    REGISTER_PROC( monk_challenge_room, PROC_ROOM ),
    REGISTER_PROC( pet_shops, PROC_ROOM ),
    REGISTER_PROC( pick_acorns, PROC_ROOM ),
    REGISTER_PROC( pick_berries, PROC_ROOM ),
    REGISTER_PROC( pray_for_items, PROC_ROOM ),
    REGISTER_PROC( preperationproc, PROC_ROOM ),
    REGISTER_PROC( pride_disabler, PROC_ROOM ),
    REGISTER_PROC( pride_remover_five, PROC_ROOM ),
    REGISTER_PROC( pride_remover_four, PROC_ROOM ),
    REGISTER_PROC( pride_remover_one, PROC_ROOM ),
    REGISTER_PROC( pride_remover_seven, PROC_ROOM ),
    REGISTER_PROC( pride_remover_six, PROC_ROOM ),
    REGISTER_PROC( pride_remover_three, PROC_ROOM ),
    REGISTER_PROC( pride_remover_two, PROC_ROOM ),
    REGISTER_PROC( Read_Room, PROC_ROOM ),
    REGISTER_PROC( riddle_exit, PROC_ROOM ),
    REGISTER_PROC( rope_room, PROC_ROOM ),
    REGISTER_PROC( sinbarrel, PROC_ROOM ),
    REGISTER_PROC( Thunder_Fountain, PROC_ROOM ),
    REGISTER_PROC( trapjawsroom, PROC_ROOM ),
    REGISTER_PROC( traproom, PROC_ROOM ),
    REGISTER_PROC( ventroom, PROC_ROOM )
};
int specProcCount = NELEMS(specProcs);


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


int procIsRegistered( int_func func, proc_type type ) 
{
    int i;

    for( i = 0; i < specProcCount; i++ ) {
        if( specProcs[i].func == func && specProcs[i].type == type ) {
            return( TRUE );
        }
    }
    return( FALSE );
}

char *procGetNameByFunc( int_func func, proc_type type ) 
{
    int i;

    if( !func ) {
        return( NULL );
    }

    for( i = 0; i < specProcCount; i++ ) {
        if( specProcs[i].func == func && specProcs[i].type == type ) {
            return( specProcs[i].name );
        }
    }
    return( NULL );
}

int_func procGetFuncByName( char *name, proc_type type ) 
{
    int i;

    if( !name ) {
        return( NULL );
    }

    for( i = 0; i < specProcCount; i++ ) {
        if( !strcmp( name, specProcs[i].name ) && specProcs[i].type == type ) {
            return( specProcs[i].func );
        }
    }
    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
