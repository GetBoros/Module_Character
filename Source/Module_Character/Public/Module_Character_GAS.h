//-------------------------------------------------------------------------------------------------------------
#pragma once
//-------------------------------------------------------------------------------------------------------------
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Module_Character_GAS.generated.h"
//-------------------------------------------------------------------------------------------------------------
class AActor;
struct FGameplayAbilitySpecHandle;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;
struct FGameplayEventData;
//-------------------------------------------------------------------------------------------------------------
UCLASS() class MODULE_CHARACTER_API UModule_Character_GAS : public UAttributeSet
{
	GENERATED_BODY()
};
//-------------------------------------------------------------------------------------------------------------
UCLASS() class UAModule_Character_Attribute : public UAttributeSet
{
    GENERATED_BODY()

public:
    UAModule_Character_Attribute();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    UFUNCTION() void OnRep_Health(const FGameplayAttributeData &old_value);
    UFUNCTION() void OnRep_Mana(const FGameplayAttributeData &old_value);
    UFUNCTION() void OnRep_Damage(const FGameplayAttributeData &old_value);
    UFUNCTION() void OnRep_Experience(const FGameplayAttributeData &old_value);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health) FGameplayAttributeData health;
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Mana) FGameplayAttributeData mana;
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Damage) FGameplayAttributeData damage;
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Experience) FGameplayAttributeData experience;
};
//-------------------------------------------------------------------------------------------------------------




// GAMEPLAY EFFECTS
UCLASS() class UAGE_Loaded_Attributes : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UAGE_Loaded_Attributes();

    void Update();  // Or call in constructor
};
//-------------------------------------------------------------------------------------------------------------
UCLASS() class UAGE_Experience_Gain : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UAGE_Experience_Gain();

    void Update();
};
//-------------------------------------------------------------------------------------------------------------




// GAMEPLAY ABILITIES
UCLASS() class UAGA_Lockpick: public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAGA_Lockpick();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo *actor_info,
	const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData *event_data_triger);

	void Experience_Give(AActor *actor);
};
//-------------------------------------------------------------------------------------------------------------
UCLASS() class UAGA_Interact : public UGameplayAbility
{
	GENERATED_BODY()

public:
    UAGA_Interact();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo *actor_info,
	const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData *event_data_triger);

	void Effect_Apply(AActor *actor);
};
//-------------------------------------------------------------------------------------------------------------
