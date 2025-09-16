// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFlowTags.h"

// ----- Ability -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Sprint, "Ability.Sprint", "Sprint");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Mantle, "Ability.Mantle", "Mantle");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Push, "Ability.Push", "Push");

// ----- CoolDown -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Ability_Push, "Cooldown.Ability.Push", "Push Cooldown");

// ----- Effect -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Stamina_Regen, "Effect.Stamina.Regen", "Stamina Regen");

// ----- Movement State -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Sprinting, "State.Movement.Sprinting", "Sprinting state");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_State_Mantling, "Status.State.Mantling", "Mantling state");

// ----- Status -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Stamina_Empty, "Status.Stamina.Empty", "No stamina");

// ----- Game Phase -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Wait, "Game.Phase.Wait", "Waiting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Match, "Game.Phase.Match", "Matching");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Loading, "Game.Phase.Loading", "LevelLoading");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Play, "Game.Phase.Play", "GamePlaying");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Finish, "Game.Phase.Finish", "GameFinish");

// ----- Player State -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Ready, "Player.Ready", "PlayerReady");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Character_First, "Player.Character.First", "BunnySelected");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Character_Second, "Player.Character.Second", "MouseSelected");