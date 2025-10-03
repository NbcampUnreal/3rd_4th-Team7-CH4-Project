// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

// ----- Ability -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Sprint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Mantle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Push);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Climb);

// ----- Cooldown -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Ability_Push);

// ----- Event -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Hit_Falling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Movement_Stopped);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Climb_Up);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Climb_Move);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Climb_Stop);

// ----- Effect -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Regen);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_Drain);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_Stamina_ClimbDrain);

// ----- Movement / State -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Movement_Sprinting);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_State_Mantling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Debuff_Stun);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Movement_Climbing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Cooldown_SprintAfterMantle);

// ----- Status -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stamina_Empty);

// ----- Game Phase -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Wait);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Match);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Play);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Phase_Finish);

// ----- Game Phase: AfterGameOver -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Rematch_Pending);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Rematch_AcceptedBoth);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Rematch_Declined);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Rematch_OpponentLeft);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Game_Rematch_Timeout);

// ----- Player State -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Ready);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Character_First);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Character_Second);

// ----- GameplayCue -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayCue_Movement_Sprinting);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GamePlayCue_StompStun);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GamePlayCue_FootStep);

// ----- Item Effects -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_JumpBoost_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_ImmunePotion_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_SpeedSlow_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Stun_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Ink_Active);

// ----- Item Cue -----
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_ImmunePotion);


namespace ClimbTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Climb);       // Input.Climb
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Parkour);     // Input.Parkour

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Climb);     // Ability.Climb
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mantle);    // Ability.Mantle

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing);    // State.Climbing
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mantling);    // State.Mantling

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Climb);   // Event.Input.Climb
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Parkour); // Event.Input.Parkour
}