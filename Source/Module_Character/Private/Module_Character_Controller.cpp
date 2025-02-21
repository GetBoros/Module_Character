#include "Module_Character_Controller.h"

#include "Module_Character.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

//-------------------------------------------------------------------------------------------------------------
AAModule_Character_Controller::AAModule_Character_Controller()
 : Module_Character(0), Mapping_Context(0), Action_Jump(0), Action_Move(0), Action_Look(0), Action_Zoom(0), Action_Exit(0)
{

}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::BeginPlay()
{
	Super::BeginPlay();

	Module_Character = Cast<AAModule_Character>(GetPawn() );

	if (UEnhancedInputLocalPlayerSubsystem *sub_system = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer() ) )
		sub_system->AddMappingContext(Mapping_Context, 0);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent *input_component = Cast<UEnhancedInputComponent>(InputComponent);
	
	if (!input_component != 0)
		return;

	input_component->BindAction(Action_Jump, ETriggerEvent::Started, this, &AAModule_Character_Controller::Jump);
	input_component->BindAction(Action_Jump, ETriggerEvent::Completed, this, &AAModule_Character_Controller::Jump_Stop);
	input_component->BindAction(Action_Move, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Move);
	input_component->BindAction(Action_Look, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Look);
	input_component->BindAction(Action_Zoom, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Zoom);
	input_component->BindAction(Action_Exit, ETriggerEvent::Started, this, &AAModule_Character_Controller::Exit);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Move(const FInputActionValue &value)
{
	const FVector2D movement_vector = value.Get<FVector2D>();
	const FRotator rotation = GetControlRotation();
	const FRotator rotation_yaw(0, rotation.Yaw, 0);
	const FVector direction_forward = FRotationMatrix(rotation_yaw).GetUnitAxis(EAxis::X);
	const FVector direction_right = FRotationMatrix(rotation_yaw).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(direction_forward, movement_vector.Y);
	GetPawn()->AddMovementInput(direction_right, movement_vector.X);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Look(const FInputActionValue &value)
{
	const FVector2D look_axis_vector = value.Get<FVector2D>();

	GetPawn()->AddControllerYawInput(look_axis_vector.X);
	GetPawn()->AddControllerPitchInput(look_axis_vector.Y);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Zoom(const FInputActionValue &value)
{
	const float look_axis_vector = value.Get<FVector2D>().X * -1;

	Module_Character->Zoom(look_axis_vector);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Exit(const FInputActionValue &value)
{
	Module_Character->Exit();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Jump(const FInputActionValue &value)
{
	Module_Character->Jump();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Jump_Stop(const FInputActionValue &value)
{
	Module_Character->StopJumping();
}
//-------------------------------------------------------------------------------------------------------------
