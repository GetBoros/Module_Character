#pragma once

#include "CoreMinimal.h"  // !!! Leave only and create .h Module_Player.h this is pch

// GAS
#include "GameplayAbilitiesModule.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

// Character
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Modules/ModuleManager.h"


// NET
#include "Net/UnrealNetwork.h"

// CUSTOM MODULE
#include "Module_IO.h"

#pragma region PLANS
/*
	X - Use Module_IO here don`t need in main game?
		X - Set cords to player
			X - attributes IO:
				X - Load and set attributes
				X - Save attributes to file
*/
#pragma endregion
