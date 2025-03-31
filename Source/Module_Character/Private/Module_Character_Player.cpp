#include "Module_Character_Player.h"
#include "Modules/ModuleManager.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

// UMyCharacterAttributes
UMyCharacterAttributes::UMyCharacterAttributes()
{
	Health.SetBaseValue(100.0f);
	Mana.SetBaseValue(50.0f);
	Damage.SetBaseValue(10.0f);
}
//-------------------------------------------------------------------------------------------------------------
void UMyCharacterAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMyCharacterAttributes, Experience, COND_OwnerOnly, REPNOTIFY_Always);
}
//-------------------------------------------------------------------------------------------------------------




// ULockpickAbility 
ULockpickAbility::ULockpickAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Interact") ) ) );
}
//-------------------------------------------------------------------------------------------------------------
void ULockpickAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	AActor *player = 0;
	AActor *actor_target;
	FHitResult hit_result {};
	FVector start;
	FVector end;
	FCollisionQueryParams params;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) )
		return;

	player = ActorInfo->AvatarActor.Get();
	if (player != 0)
		UE_LOG(LogTemp, Warning, TEXT("Attack Activated!") );

	if (!HasAuthority(&ActivationInfo) )
		return;

	if (player != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s использует LockpickAbility"), *player->GetName() );

		start = player->GetActorLocation();
		end = start + player->GetActorForwardVector() * 200.0f;  // Луч на 200 см вперед
		params.AddIgnoredActor(player);

		if (GetWorld()->LineTraceSingleByChannel(hit_result, start, end, ECC_Visibility, params) )
		{
			actor_target = hit_result.GetActor();
			if (actor_target)
			{
				UE_LOG(LogTemp, Warning, TEXT("Открываем: %s"), *actor_target->GetName() );
				actor_target->Destroy();  // Здесь может быть вызов анимации открытия
				GiveExperience(player);

				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------
void ULockpickAbility::GiveExperience(AActor* PlayerActor)
{
	if (!PlayerActor)
		return;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerActor))
	{
		const UMyCharacterAttributes* ConstAttributes = static_cast<const UMyCharacterAttributes*>(ASC->GetAttributeSet(UMyCharacterAttributes::StaticClass()));
		if (!ConstAttributes) return;

		// Убираем const, чтобы можно было изменить значение
		UMyCharacterAttributes* Attributes = const_cast<UMyCharacterAttributes*>(ConstAttributes);

		// Проверяем, что объект есть и увеличиваем опыт
		if (Attributes)
		{
			float NewExp = Attributes->GetExperience() + 10.0f;
			Attributes->SetExperience(NewExp);

			UE_LOG(LogTemp, Warning, TEXT("%s получил опыт: %.2f"), *PlayerActor->GetName(), NewExp);
		}
	}
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

	AttributeSet = CreateDefaultSubobject<UMyCharacterAttributes>("AttributeSet");
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
	Super::BeginPlay();

	if (HasAuthority() && Ability_System_Component)
		Ability_System_Component->GiveAbility(FGameplayAbilitySpec(ULockpickAbility::StaticClass(), 1, 0) );

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
void AAModule_Character_Player::Interact()
{
	if (UAbilitySystemComponent *asc = GetAbilitySystemComponent() )  // if GAS added
	{
		FGameplayTag tag_interact = FGameplayTag::RequestGameplayTag(FName("Ability.Interact") );  // try to find tag
		FGameplayAbilitySpec *spec = asc->FindAbilitySpecFromClass(ULockpickAbility::StaticClass() );
		
		if (spec && spec->IsActive() == false)
			asc->TryActivateAbility(spec->Handle);
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
