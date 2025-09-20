// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

// ----- Ability -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Sprint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Mantle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Push);

// ----- CoolDown -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Ability_Push);

// ----- Event -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Hit_Falling);

// ----- Effect -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Regen);

// ----- Movement State -----
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

// ----- Item Set By Caller -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_WalkDelta);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_SprintDelta);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_JumpDelta);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_StaminaDelta);

// ----- Item Activation Tag -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_JumpBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_ImmunePotion_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedSlow_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Stun_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Ink_Active);