// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFlowTags.h"

// ----- Ability -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Sprint, "Ability.Sprint", "Sprint");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Mantle, "Ability.Mantle", "Mantle");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Push, "Ability.Push", "Push");

// ----- Cooldown -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Ability_Push, "Cooldown.Ability.Push", "Push Cooldown");

// ----- Event -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Hit_Falling, "Event.Hit.Falling", "Character Hit when Falling");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Movement_Stopped, "Event.Movement.Stopped", "Character Stops");

// ----- Effect -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Stamina_Regen, "Effect.Stamina.Regen", "Stamina Regen");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effect_Stamina_Drain, "Effect.Stamina.Drain", "Stamina drain while sprint");

// ----- Movement / State -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Sprinting, "State.Movement.Sprinting", "Sprinting state");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_State_Mantling, "Status.State.Mantling", "Mantling state");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Debuff_Stun, "State.Debuff.Stun", "Stun Debuff state");

// ----- Status -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Stamina_Empty, "Status.Stamina.Empty", "No stamina");

// ----- Game Phase -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Wait, "Game.Phase.Wait", "Waiting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Match, "Game.Phase.Match", "Matching");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Loading, "Game.Phase.Loading", "LevelLoading");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Play, "Game.Phase.Play", "GamePlaying");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Game_Phase_Finish, "Game.Phase.Finish", "GameFinish");

// ----- Player State -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Ready, "Player.Ready", "Player Ready");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Character_First, "Player.Character.First", "Bunny Selected");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Player_Character_Second, "Player.Character.Second", "Mouse Selected");

// ----- GameplayCue -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayCue_Movement_Sprinting, "GameplayCue.Movement.Sprinting", "Speedlines while sprinting");

// ----- Item Effects -----
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_SpeedBoost_Active, "Item.SpeedBoost.Active", "Speed boost effect is active.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_JumpBoost_Active, "Item.JumpBoost.Active", "Jump boost effect is active.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_ImmunePotion_Active, "Item.ImmunePotion.Active", "Immune potion effect is active.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_SpeedSlow_Active, "Item.SpeedSlow.Active", "Speed Slow effect is active.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_Stun_Active, "Item.Stun.Active", "Stun effect is active.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_Ink_Active, "Item.Ink.Active", "Ink effect is active.");