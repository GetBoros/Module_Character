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
	Health.SetBaseValue(100.0f);
	Mana.SetBaseValue(50.0f);
	Damage.SetBaseValue(10.0f);
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
UAGE_Experience_Gain::UAGE_Experience_Gain()
{
	constexpr float give_exp_value = 10.0f;
	FGameplayModifierInfo modifier;
	DurationPolicy = EGameplayEffectDurationType::Instant;

	modifier.Attribute = FGameplayAttribute(FindFieldChecked<FProperty>(UAModule_Character_Attribute::StaticClass(), TEXT("Experience") ) );  // try find and access attribute
	modifier.ModifierOp = EGameplayModOp::Additive;  // add to existing value
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
	AActor *player = 0;
	AActor *actor_target;
	FHitResult hit_result {};
	FVector start {};
	FVector end {};
	FCollisionQueryParams params {};

	if (!CommitAbility(handle, actor_info, activation_info) )
		return;

	player = actor_info->AvatarActor.Get();
	if (player != 0)
		UE_LOG(LogTemp, Warning, TEXT("Attack Activated!") );

	if (!HasAuthority(&activation_info) )
		return;

	if (!player != 0)
		return;

	UE_LOG(LogTemp, Warning, TEXT("%s used LockpickAbility"), *player->GetName() );

	start = player->GetActorLocation();
	end = start + player->GetActorForwardVector() * 200.0f;  // trace at 200 centimetr
	params.AddIgnoredActor(player);

	if (GetWorld()->LineTraceSingleByChannel(hit_result, start, end, ECC_Visibility, params) != true)
		return;
	actor_target = hit_result.GetActor();
	if (!actor_target != 0)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Try to open: %s"), *actor_target->GetName() );
	actor_target->Destroy();  // Destroy box or any item if trace catch
	EndAbility(handle, actor_info, activation_info, true, false);
	Experience_Give(player);
}
//-------------------------------------------------------------------------------------------------------------
void UAGA_Lockpick::Experience_Give(AActor *actor)
{
	UAbilitySystemComponent *asc = 0;
	FGameplayEffectSpecHandle effect_spec {};

	if (!actor != 0)
		return;

	asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(actor);
	if (!asc != 0)
		return;

	effect_spec = asc->MakeOutgoingSpec(UAGE_Experience_Gain::StaticClass(), 1, asc->MakeEffectContext() );
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
	//effect_context = Ability_System_Component->MakeEffectContext();  // we make context effect
	//effect_spec = Ability_System_Component->MakeOutgoingSpec(UAGE_Initialize_Attributes::StaticClass(), 1, effect_context);  // make spec effect 
	//if (effect_spec.IsValid() )
	//	Ability_System_Component->ApplyGameplayEffectSpecToSelf(*effect_spec.Data);  // и применяем на компоненте

	//Character_Attribute->Experience.SetBaseValue(UAModule_IO::Module_IO_Create()->GAS_Attributes_Load() );

	// !!! Make array and transform || DeepSeek Example
	Character_Attribute->Experience.SetCurrentValue(UAModule_IO::Module_IO_Create()->GAS_Attributes_Load() );
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
	if (UAbilitySystemComponent *asc = GetAbilitySystemComponent() )  // if GAS added
	{
		FGameplayTag tag_interact = FGameplayTag::RequestGameplayTag(FName("Ability.Interact") );  // try to find tag
		FGameplayAbilitySpec *spec = asc->FindAbilitySpecFromClass(UAGA_Lockpick::StaticClass() );
		
		if (spec && spec->IsActive() == false)
			asc->TryActivateAbility(spec->Handle);

		float experience = Character_Attribute->Experience.GetBaseValue();
		UAModule_IO *module_io = UAModule_IO::Module_IO_Create();
		module_io->GAS_Attributes_Save(experience);
		module_io->Pawn_Transform_Save(GetTransform() );
	}
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
