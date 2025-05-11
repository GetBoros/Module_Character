#include "Module_Character_Player.h"
#include "Module_Character_GAS.h"  // !!! TEMP
#include "Module_Character.h"

// AAModule_Character_Player
AAModule_Character_Player::AAModule_Character_Player()
 : Is_State_Camera(false), Attributes(0), Ability_System_Component(0), Camera_Boom(0), Camera_Follow(0)
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

	Attributes = CreateDefaultSubobject<UAModule_Character_Attribute>("Attributes");
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
	FGameplayEffectContextHandle effect_context;
	FGameplayEffectSpecHandle effect_spec;
	const FVector player_location_initial { 550.0, 1990.0, 50.0 };
	const FTransform player_transform = UAModule_IO_Handler::Module_IO_Create()->Pawn_Transform_Load();  // load from Module IO last saved player transform

	// 1.0. If loaded non zero transform set prev player transform || not game beginning
	if (player_transform.GetLocation() == FVector::ZeroVector)
		SetActorLocation(player_location_initial);  // starting point
	else
		SetActorTransform(player_transform);  // loaded prev player transform and set it

	// 2.0. GAS | If have asc give ability lockpicking
	if (HasAuthority() == true && Ability_System_Component != 0)
	{
		Ability_System_Component->GiveAbility(FGameplayAbilitySpec(UAGA_Lockpick::StaticClass(), 1, 0) );
		Ability_System_Component->GiveAbility(FGameplayAbilitySpec(UAGA_Interact::StaticClass(), 1, 0) );
	}

	// 2.1. GAS | Load from Module_IO and use Effect to apply
	Attribute_Load();

	Super::BeginPlay();
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
	Abilities_Handler(false);
	Attribute_Save();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Abilities_Handler(const bool is_lock_pick)
{
    FGameplayAbilitySpec *ability_spec;

	if (is_lock_pick)
        ability_spec = Ability_System_Component->FindAbilitySpecFromClass(UAGA_Lockpick::StaticClass() );
	else
        ability_spec = Ability_System_Component->FindAbilitySpecFromClass(UAGA_Interact::StaticClass() );

    if (ability_spec == 0 && ability_spec->IsActive() == true)
        return; // if ability active or don`t have ability

	Ability_System_Component->TryActivateAbility(ability_spec->Handle);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Attribute_Save()
{
	constexpr int32 array_size = 4;  // !!! 4 attributes change to enum
	int32 index;

	UObject *obj;
	const UClass *class_info;
	const FProperty *property;
	const FGameplayAttributeData *character_attributes;
	UAModule_IO_Handler *module_io;
	TArray<float> player_attributes;

	// 1.0. Init
	index = 0;
	obj = Attributes;
	class_info = obj->GetClass();
	module_io = UAModule_IO_Handler::Module_IO_Create();
	player_attributes.SetNumZeroed(array_size);

	// 1.1. From each property get value if its struct and double value
	for (property = class_info->PropertyLink; property != 0; property = property->PropertyLinkNext)
	{
		if (property->IsA<FStructProperty>() == 0)
			return;  // if param not struct
		
		if (CastField<FStructProperty>(property)->Struct == FGameplayAttributeData::StaticStruct() == 0)
			return;  // if struct not FGameplayAttributeData
		
		character_attributes = property->ContainerPtrToValuePtr<FGameplayAttributeData>(obj);
		if (character_attributes == 0)
			return;  // If don`t contains valid pointer

		player_attributes[index++] = character_attributes->GetCurrentValue();  // Save value
	}
	
	module_io->GAS_Attributes_Save(player_attributes);
	module_io->Pawn_Transform_Save(GetTransform() );
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Player::Attribute_Load()
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
		return;
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
//void AAModule_Character_Player::Inventory_Handle()
//{
//	// Show | Hide Inventory
//
//	// Load info for each slot
//
//	// Slots can try apply GE or GA if skill
//
//}
////-------------------------------------------------------------------------------------------------------------
