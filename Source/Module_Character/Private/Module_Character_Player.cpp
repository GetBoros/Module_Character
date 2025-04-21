#include "Module_Character_Player.h"
#include "Modules/ModuleManager.h"

#include "Module_IO.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

// UAModule_Character_Attribute
UAModule_Character_Attribute::UAModule_Character_Attribute()
{

}
//-------------------------------------------------------------------------------------------------------------
void UAModule_Character_Attribute::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION_NOTIFY(UAModule_Character_Attribute, Experience, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAModule_Character_Attribute, Experience, COND_None, REPNOTIFY_Always);
}
//-------------------------------------------------------------------------------------------------------------



// UAGE_Experience_Gain
UAGE_Loaded_Attributes::UAGE_Loaded_Attributes()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Update();
}
//-------------------------------------------------------------------------------------------------------------
void UAGE_Loaded_Attributes::Update()
{
	FProperty* test;
	constexpr float give_exp_value = 3.0f;
	FGameplayModifierInfo modifier;
	TArray<float> player_attributes;

	UAModule_IO::Module_IO_Create()->GAS_Attributes_Load(player_attributes);

	Modifiers.Empty();

	test = FindFieldChecked<FProperty>(UAModule_Character_Attribute::StaticClass(), TEXT("Experience"));
	if (test == 0)
		return;
	modifier.Attribute = FGameplayAttribute(test);  // try find and access attribute
	modifier.ModifierOp = EGameplayModOp::Additive;
	modifier.ModifierMagnitude = FScalableFloat(player_attributes[3]);  // value to add to existed base value

	Modifiers.Add(modifier);  // applying
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
	modifier.Attribute = FGameplayAttribute(test);  // try find and access attribute
	modifier.ModifierOp = EGameplayModOp::Additive;
	modifier.ModifierMagnitude = FScalableFloat(give_exp_value);  // value to add to existed base value

	Modifiers.Add(modifier);  // applying
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




// AAModule_Character_Player
AAModule_Character_Player::AAModule_Character_Player()
 : Is_State_Camera(false), Ability_System_Component(0), Camera_Boom(0), Camera_Follow(0)
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	Character_Attribute = CreateDefaultSubobject<UAModule_Character_Attribute>("AttributeSet");
	Ability_System_Component = CreateDefaultSubobject<UAbilitySystemComponent>("Ability_System_Component");

	Camera_Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera_Boom") );
	Camera_Boom->SetupAttachment(RootComponent);
	Camera_Boom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	Camera_Boom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	Camera_Follow = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_Follow") );
	Camera_Follow->SetupAttachment(Camera_Boom, USpringArmComponent::SocketName);  // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	Camera_Follow->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::BeginPlay()
{
	FGameplayEffectContextHandle effect_context {};
	FGameplayEffectSpecHandle effect_spec {};
	const FVector player_location_initial { 550.0, 1990.0, 50.0 };
	const FTransform player_transform = UAModule_IO::Module_IO_Create()->Pawn_Transform_Load();  // load from Module IO last saved player transform

	Super::BeginPlay();

	// 1.0. If loaded non zero transform set prev player transform || not game begining
	if (player_transform.GetLocation() == FVector::ZeroVector)
		SetActorLocation(player_location_initial);  // starting point
	else
		SetActorTransform(player_transform);  // loaded prev player transform and set it

	// 2.0. GAS | If have asc give ability lockpicking
	if (HasAuthority() && Ability_System_Component)
		Ability_System_Component->GiveAbility(FGameplayAbilitySpec(UAGA_Lockpick::StaticClass(), 1, 0) );

	// 2.1. GAS | Load from Module_IO and use Effect to apply
	Character_Attribute_Load();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
}
//-------------------------------------------------------------------------------------------------------------
UAbilitySystemComponent *AAModule_Character_Player::GetAbilitySystemComponent() const
{
	return Ability_System_Component;
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Look(const FVector2D look_axis_vector)
{
	AddControllerYawInput(look_axis_vector.X);

	if (!Is_State_Camera)
		AddControllerPitchInput(look_axis_vector.Y);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Zoom(const float step_offset)
{
	constexpr float step_min = 10.0f;
	constexpr float step_max = 100.0f;
	const float field_of_view = step_offset * step_min + Camera_Follow->FieldOfView;

	Camera_Follow->FieldOfView = FMath::Clamp(field_of_view, step_min, step_max);  // !!! Can decrease mouse sensivity maybe, i don`t know if i need it now
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Camera_Exit()
{
	Is_State_Camera = false;
	Camera_Follow->FieldOfView = 90;  // If custom in bp bed
	Camera_Follow->SetRelativeRotation(FRotator().ZeroRotator);
	Camera_Boom->TargetArmLength = 400;
	Camera_Boom->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Interact()
{
	UAbilitySystemComponent *ability_sys_component;
	FGameplayAbilitySpec *lock_pick;

	ability_sys_component = GetAbilitySystemComponent();
	if (ability_sys_component == 0)  // if GAS added
		return;

	lock_pick = ability_sys_component->FindAbilitySpecFromClass(UAGA_Lockpick::StaticClass() );
	if (lock_pick == 0 && lock_pick->IsActive() != false)
		return;

	ability_sys_component->TryActivateAbility(lock_pick->Handle);
	
	Character_Attribute_Save_Ext();  // !!! TEMP 
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Character_Attribute_Save()
{
	UAModule_IO *module_io = 0;
	TArray<float> player_attributes;
	
	module_io = UAModule_IO::Module_IO_Create();
	player_attributes.SetNumZeroed(4);

	player_attributes[0] = Character_Attribute->Health.GetCurrentValue();
	player_attributes[1] = Character_Attribute->Mana.GetCurrentValue();
	player_attributes[2] = Character_Attribute->Damage.GetCurrentValue();
	player_attributes[3] = Character_Attribute->Experience.GetCurrentValue();

	module_io->GAS_Attributes_Save(player_attributes);
	module_io->Pawn_Transform_Save(GetTransform() );
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Character_Attribute_Save_Ext()
{
	constexpr int32 array_size = 4;  // 4 attributes change to enum
	int32 index;

	UObject *obj;
	const UClass *class_info;
	UAModule_IO *module_io;
	FProperty *property;
	FGameplayAttributeData *character_attributes;
	TArray<float> player_attributes;

	index = 0;
	obj = Character_Attribute;
	class_info = obj->GetClass();
	module_io = UAModule_IO::Module_IO_Create();
	player_attributes.SetNumZeroed(array_size);

	for (property = class_info->PropertyLink; property != 0; property = property->PropertyLinkNext)
	{
		if (property->IsA<FStructProperty>() == 0)  // if param not struct
			return;
		
		if (CastField<FStructProperty>(property)->Struct == FGameplayAttributeData::StaticStruct() == 0)  // if struct not FGameplayAttributeData
			return;
		
		character_attributes = property->ContainerPtrToValuePtr<FGameplayAttributeData>(obj);  // 
		if (character_attributes == 0)
			return;

		player_attributes[index++] = character_attributes->GetCurrentValue();
	}

	module_io->GAS_Attributes_Save(player_attributes);
	module_io->Pawn_Transform_Save(GetTransform() );
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Character_Attribute_Load()
{
	UAGE_Loaded_Attributes *loaded_attributes_effect;
	FGameplayEffectContextHandle effect_context;
	FActiveGameplayEffectHandle effect_active_handler;

	if (!GetAbilitySystemComponent() )
		return;

	loaded_attributes_effect = NewObject<UAGE_Loaded_Attributes>(this);
	if (loaded_attributes_effect == 0)
		return;
	loaded_attributes_effect->Update();

	effect_context = GetAbilitySystemComponent()->MakeEffectContext();
	effect_context.AddSourceObject(this);

	effect_active_handler = GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(loaded_attributes_effect, 1, effect_context);

	if (!effect_active_handler.IsValid() )
		UE_LOG(LogTemp, Error, TEXT("Failed to apply loaded_attributes_effect!") );
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Camera_Switch(const FVector location, const FRotator rotation)
{
	Is_State_Camera = true;
	Camera_Boom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Camera_Boom->SetWorldLocation(location);
	Camera_Follow->SetWorldRotation(rotation);
	Camera_Boom->TargetArmLength = 0;
}
//-------------------------------------------------------------------------------------------------------------
