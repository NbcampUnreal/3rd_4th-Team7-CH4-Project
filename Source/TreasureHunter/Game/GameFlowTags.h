// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

// ----- Ability -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Sprint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Mantle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Push);

// ----- Cooldown -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Ability_Push);

// ----- Event -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Hit_Falling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Stopped);

// ----- Effect -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Regen);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Drain);

// ----- Movement / State -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Movement_Sprinting);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_State_Mantling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Stun);

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

// ----- GameplayCue -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayCue_Movement_Sprinting);

// ----- Item Effects -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_JumpBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_ImmunePotion_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedSlow_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Stun_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Ink_Active);