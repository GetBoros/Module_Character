#include "Module_Character_Player.h"
#include "Modules/ModuleManager.h"

#include "AbilitySystemComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

//-------------------------------------------------------------------------------------------------------------
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

	Ability_System_Component = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");

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
	Super::BeginPlay();
	const USkeletalMeshComponent *skeletal_mesh_component = GetMesh();
	TArray<FName> bone_names = skeletal_mesh_component->GetAllSocketNames();

	for (const FName &bone: bone_names)  // !!!
	{
		int yy = 0;
		yy++;
	}

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
void AAModule_Character_Player::Camera_Switch(const FVector location, const FRotator rotation)
{
	Is_State_Camera = true;
	Camera_Boom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Camera_Boom->SetWorldLocation(location);
	Camera_Follow->SetWorldRotation(rotation);
	Camera_Boom->TargetArmLength = 0;
}
//-------------------------------------------------------------------------------------------------------------
