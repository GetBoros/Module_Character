#include "Module_Character_GAS.h"
#include "Module_Character.h"

// UAModule_Character_Attribute
UAModule_Character_Attribute::UAModule_Character_Attribute()
{
}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // DOREPLIFETIME_CONDITION_NOTIFY(UAModule_Character_Attribute, Experience, COND_OwnerOnly, REPNOTIFY_Always);
    //DOREPLIFETIME_CONDITION_NOTIFY(UAModule_Character_Attribute, Experience, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME(UAModule_Character_Attribute, health);
    DOREPLIFETIME(UAModule_Character_Attribute, mana);
    DOREPLIFETIME(UAModule_Character_Attribute, damage);
    DOREPLIFETIME(UAModule_Character_Attribute, experience);
}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::OnRep_Health(const FGameplayAttributeData &old_value)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAModule_Character_Attribute, health, old_value);
}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::OnRep_Mana(const FGameplayAttributeData &old_value)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAModule_Character_Attribute, mana, old_value);
}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::OnRep_Damage(const FGameplayAttributeData &old_value)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAModule_Character_Attribute, damage, old_value);
}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::OnRep_Experience(const FGameplayAttributeData &old_value)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAModule_Character_Attribute, experience, old_value);
}
//-------------------------------------------------------------------------------------------------------------



// UAGE_Experience_Gain
UAGE_Loaded_Attributes::UAGE_Loaded_Attributes()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
}
//-------------------------------------------------------------------------------------------------------------
void UAGE_Loaded_Attributes::Update()
{
    int32 attribute_index = 0;
    TArray<float> player_attributes;
    FProperty *prop_attribute;

	UAModule_IO_Handler::Module_IO_Create()->GAS_Attributes_Load(player_attributes);

    for (TFieldIterator<FProperty> prop_iterator(UAModule_Character_Attribute::StaticClass()); prop_iterator; ++prop_iterator)
    {
        prop_attribute = *prop_iterator;
        if (prop_attribute == 0 || attribute_index >= player_attributes.Num())
            break;

        Modifiers.Add(FGameplayModifierInfo
            {
                .Attribute = FGameplayAttribute(prop_attribute),
                .ModifierOp = EGameplayModOp::Additive,
                .ModifierMagnitude = FScalableFloat(player_attributes[attribute_index++])
            } );
    }
}
//-------------------------------------------------------------------------------------------------------------




// UAGE_Experience_Gain
UAGE_Experience_Gain::UAGE_Experience_Gain()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
    Update();
}
//-------------------------------------------------------------------------------------------------------------
void UAGE_Experience_Gain::Update()
{
    FProperty *test;
    constexpr float give_exp_value = 3.0f;
    FGameplayModifierInfo modifier;

    Modifiers.Empty();

    test = FindFieldChecked<FProperty>(UAModule_Character_Attribute::StaticClass(), TEXT("Experience") );
    if (test == 0)
        return;
    modifier.Attribute = FGameplayAttribute(test); // try find and access attribute
    modifier.ModifierOp = EGameplayModOp::Additive;
    modifier.ModifierMagnitude = FScalableFloat(give_exp_value); // value to add to existed base value

    Modifiers.Add(modifier); // applying
}
//-------------------------------------------------------------------------------------------------------------




// UAGA_Lockpick  
UAGA_Lockpick::UAGA_Lockpick()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;  // Not neccessary to call EndAbility
	SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Interact") ) ) );
}
//-------------------------------------------------------------------------------------------------------------
void UAGA_Lockpick::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo *actor_info,
	const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData *event_data_triger)
{
	AActor *player, *target;
	FVector player_location, offset_location;
	FHitResult hit_result;
	FCollisionQueryParams collision_query;

	if (!CommitAbility(handle, actor_info, activation_info) )
		return;

	if (HasAuthority(&activation_info) == false)
		return;
	
	player = actor_info->AvatarActor.Get();
	if (player == 0)
		return;

	player_location = player->GetActorLocation();
	offset_location = player_location + player->GetActorForwardVector() * 200.0f;  // trace at 200 centimetr
	collision_query.AddIgnoredActor(player);
	if (GetWorld()->LineTraceSingleByChannel(hit_result, player_location, offset_location, ECC_Visibility, collision_query) != true)
		return;
	
	target = hit_result.GetActor();
	if (!target != 0)
		return;

	target->Destroy();  // Destroy box or any item if trace catch
	Experience_Give(player);
	EndAbility(handle, actor_info, activation_info, true, false);
}
//-------------------------------------------------------------------------------------------------------------
void UAGA_Lockpick::Experience_Give(AActor *actor)
{
	UAbilitySystemComponent *asc;
	FGameplayEffectSpecHandle effect_spec;

	if (!actor != 0)
		return;

	asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(actor);
	if (!asc != 0)
		return;

	effect_spec = asc->MakeOutgoingSpec(UAGE_Experience_Gain::StaticClass(), 1.0f, asc->MakeEffectContext() );
	if (!effect_spec.IsValid() )
		return;

	asc->ApplyGameplayEffectSpecToSelf(*effect_spec.Data);
}
//-------------------------------------------------------------------------------------------------------------




// UAGA_Interact
UAGA_Interact::UAGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;  // Not neccessary to call EndAbility
	SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Interact") ) ) );
}
//-------------------------------------------------------------------------------------------------------------
void UAGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo *actor_info,
	const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData *event_data_triger)
{
	AActor *player, *target;
	FVector player_location, offset_location;
	FHitResult hit_result;
	FCollisionQueryParams collision_query;

	if (!CommitAbility(handle, actor_info, activation_info) )
		return;

	if (HasAuthority(&activation_info) == false)
		return;
	
	player = actor_info->AvatarActor.Get();
	if (player == 0)
		return;

	player_location = player->GetActorLocation();
	offset_location = player_location + player->GetActorForwardVector() * 200.0f;  // trace at 200 centimeter
	collision_query.AddIgnoredActor(player);
	if (GetWorld()->LineTraceSingleByChannel(hit_result, player_location, offset_location, ECC_Visibility, collision_query) != true)
		return;
	
	target = hit_result.GetActor();
	if (!target != 0)
		return;

	target->Destroy();  // Destroy box or any item if trace catch
	Effect_Apply(player);
	EndAbility(handle, actor_info, activation_info, true, false);
}
//-------------------------------------------------------------------------------------------------------------
void UAGA_Interact::Effect_Apply(AActor *actor)
{
	UAbilitySystemComponent *asc;
	FGameplayEffectSpecHandle effect_spec;

	if (!actor != 0)
		return;

	asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(actor);
	if (!asc != 0)
		return;

	effect_spec = asc->MakeOutgoingSpec(UAGE_Experience_Gain::StaticClass(), 1.0f, asc->MakeEffectContext() );
	if (!effect_spec.IsValid() )
		return;

	asc->ApplyGameplayEffectSpecToSelf(*effect_spec.Data);
}
//-------------------------------------------------------------------------------------------------------------
