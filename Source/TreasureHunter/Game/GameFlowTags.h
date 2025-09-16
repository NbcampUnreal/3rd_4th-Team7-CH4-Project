// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

// ----- Ability -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Sprint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Mantle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Push);

// ----- CoolDown -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Ability_Push);

// ----- Effect -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Regen);

// ----- Movement State -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Movement_Sprinting);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_State_Mantling);

// ----- Status -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stamina_Empty);

// ----- Game Phase -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Wait);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Match);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Play);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Finish);

// ----- Player State -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Ready);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Character_First);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Character_Second);