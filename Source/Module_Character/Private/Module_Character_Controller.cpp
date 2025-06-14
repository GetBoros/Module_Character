#include "Module_Character_Controller.h"
#include "Module_Character_Player.h"
#include "Module_Character.h"

//-------------------------------------------------------------------------------------------------------------
AAModule_Character_Controller::AAModule_Character_Controller()
 : Module_Character_Player(0), Is_Camera(false), Button_Pressed(EButton_Pressed::EBP_None), Mapping_Context(0),
   Action_Jump(0), Action_Move(0), Action_Look(0), Action_Zoom(0), Action_Exit(0), Action_Mini_Map(0), Action_Interact(0), Action_Inventory_Open(0)
{

}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::BeginPlay()
{
	Super::BeginPlay();

	Module_Character_Player = Cast<AAModule_Character_Player>(GetPawn() );

	if (UEnhancedInputLocalPlayerSubsystem *sub_system = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer() ) )
		sub_system->AddMappingContext(Mapping_Context, 0);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::SetupInputComponent()
{
	Super::SetupInputComponent();  // Must be first

	UEnhancedInputComponent *input_component = Cast<UEnhancedInputComponent>(InputComponent);
	
	if (input_component == 0)
		return;

	input_component->BindAction(Action_Jump, ETriggerEvent::Started, this, &AAModule_Character_Controller::Jump);
	input_component->BindAction(Action_Jump, ETriggerEvent::Completed, this, &AAModule_Character_Controller::Jump_Stop);
	input_component->BindAction(Action_Move, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Move);
	input_component->BindAction(Action_Look, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Look);
	input_component->BindAction(Action_Zoom, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Zoom);
	input_component->BindAction(Action_Mini_Map, ETriggerEvent::Triggered, this, &AAModule_Character_Controller::Mini_Map);
	input_component->BindAction(Action_Exit, ETriggerEvent::Started, this, &AAModule_Character_Controller::Exit);
	input_component->BindAction(Action_Interact, ETriggerEvent::Started, this, &AAModule_Character_Controller::Interact);
	input_component->BindAction(Action_Inventory_Open, ETriggerEvent::Started, this, &AAModule_Character_Controller::Inventory_Open);

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

	Module_Character_Player->Look(look_axis_vector);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Zoom(const FInputActionValue &value)
{
	const float look_axis_vector = value.Get<FVector2D>().X * -1;

	Module_Character_Player->Zoom(look_axis_vector);
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Jump(const FInputActionValue &value)
{
	Module_Character_Player->Jump();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Exit(const FInputActionValue &value)
{
	Button_Pressed = EButton_Pressed::EBP_Exit;

	if (!Module_Character_Player->Is_State_Camera != true)  // if looked from camera while press Exit button remove state
		Module_Character_Player->Camera_Exit();
	else
		On_Button_Pressed();  // Can be additional code in blueprints
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Mini_Map(const FInputActionValue &value)
{
	Button_Pressed = EButton_Pressed::EBP_Mini_Map;
	On_Button_Pressed();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Interact(const FInputActionValue &value)
{
	Module_Character_Player->Interact();
	Button_Pressed = EButton_Pressed::EBP_Interact;
	On_Button_Pressed();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Jump_Stop(const FInputActionValue &value)
{
	Module_Character_Player->StopJumping();
}
//-------------------------------------------------------------------------------------------------------------
void AAModule_Character_Controller::Inventory_Open(const FInputActionValue &value)
{
	Module_Character_Player->Inventory_Handle();
	Button_Pressed = EButton_Pressed::EBP_Inventory_Open;
	On_Button_Pressed();
}
//-------------------------------------------------------------------------------------------------------------
