// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace FCharacterCurveName
{
	static const FName NAME_BasePose_N = "BasePose_N";
	static const FName NAME_BasePose_C = "BasePose_CLF";
	static const FName NAME_Enable_FootIK_L = "Enable_FootIK_L";
	static const FName NAME_Enable_FootIK_R = "Enable_FootIK_R";
	static const FName NAME_Enable_HandIK_L = "Enable_HandIK_L";
	static const FName NAME_Enable_HandIK_R = "Enable_HandIK_R";
	static const FName NAME_Enable_Transition = "Enable_Transition";
	static const FName NAME_FootLock_L = "FootLock_L";
	static const FName NAME_FootLock_R = "FootLock_R";
	static const FName NAME_Gait = "W_Gait";
	static const FName NAME_Grounded_Slot = "Grounded Slot";
	static const FName NAME_Mask_AimOffset = "Mask_AimOffset";
	static const FName NAME_Mask_LandPrediction = "Mask_LandPrediction";
	static const FName NAME_RotationAmount = "RotationAmount";
}

namespace FCharacterBoneName
{
	static const FName Root = "root";
	static const FName Pelvis = "pelvis";
	static const FName Spine_01 = "spine_01";
	static const FName Spine_02 = "spine_02";
	static const FName Spine_03 = "spine_03";
	static const FName Spine_04 = "spine_04";
	static const FName Spine_05 = "spine_05";
	static const FName Neck_01 = "neck_01";
	static const FName Neck_02 = "neck_02";
	static const FName Head = "head";
	static const FName Clavicle_L = "clavicle_l";
	static const FName Upperarm_L = "upperarm_l";
	static const FName Hand_L = "hand_l";
	static const FName Clavicle_R = "clavicle_r";
	static const FName Upperarm_R = "upperarm_r";
	static const FName Hand_R = "hand_r";
	static const FName Thigh_L = "thigh_l";
	static const FName Calf_L = "calf_l";
	static const FName Foot_L = "foot_l";
	static const FName Thigh_R = "thigh_r";
	static const FName Calf_R = "calf_r";
	static const FName Foot_R = "foot_r";
	static const FName IkFoot_L = "ik_foot_l";
	static const FName IkFoot_R = "ik_foot_r";
	static const FName VB_Foot_Target_L = "VB foot_target_l";
	static const FName VB_Foot_Target_R = "VB foot_target_r";
}

namespace FCharacterLayerName
{
	static const FName NAME_Layering_Arm_L = "Layering_Arm_L";
	static const FName NAME_Layering_Arm_L_Add = "Layering_Arm_L_Add";
	static const FName NAME_Layering_Arm_L_LS = "Layering_Arm_L_LS";
	static const FName NAME_Layering_Arm_R = "Layering_Arm_R";
	static const FName NAME_Layering_Arm_R_Add = "Layering_Arm_R_Add";
	static const FName NAME_Layering_Arm_R_LS = "Layering_Arm_R_LS";
	static const FName NAME_Layering_Hand_L = "Layering_Hand_L";
	static const FName NAME_Layering_Hand_R = "Layering_Hand_R";
	static const FName NAME_Layering_Head_Add = "Layering_Head_Add";
	static const FName NAME_Layering_Spine_Add = "Layering_Spine_Add";
}
