#pragma once

#include "GameFramework/PlayerController.h"
#include "Module_Character_Controller.generated.h"

//-------------------------------------------------------------------------------------------------------------
UENUM(BlueprintType) enum class EButton_Pressed : uint8
{
	EBP_None,
	EBP_Exit,
	EBP_Mini_Map,
	EBP_Interact
};
//-------------------------------------------------------------------------------------------------------------
class UInputMappingContext;
class UInputAction;
class AAModule_Character_Player;
struct FInputActionValue;
//-------------------------------------------------------------------------------------------------------------
UCLASS() class MODULE_CHARACTER_API AAModule_Character_Controller : public APlayerController
{
	GENERATED_BODY()

public:
	AAModule_Character_Controller();

	virtual void BeginPlay();
	virtual void SetupInputComponent();

	void Move(const FInputActionValue &value);
	void Look(const FInputActionValue &value);  // !!! Maybe change, make another InputComponent for camera or ...
	void Zoom(const FInputActionValue &value);  // !!! Can be better | change from Vector2D to something else
	void Jump(const FInputActionValue &value);
	void Exit(const FInputActionValue &value);  // Restore Boom state || Menu || Q Button |
	void Mini_Map(const FInputActionValue &value);
	void Interact(const FInputActionValue &value);
	void Jump_Stop(const FInputActionValue &value);

	AAModule_Character_Player *Module_Character_Player;  // !!! Maybe change to component if need
	bool Is_Camera;

	UFUNCTION(BlueprintImplementableEvent) void On_Button_Exit();  // BP Event || REMOVE THIS
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable) void On_Button_Pressed();  // BP Event

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) EButton_Pressed Button_Pressed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputMappingContext *Mapping_Context;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Jump;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Move;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Look;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Zoom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Exit;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Mini_Map;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Interact;
};
//-------------------------------------------------------------------------------------------------------------



#pragma region TASKS
/*

#MAIN

#MAIN_END

	X - Interact from camera:
		- Set marks, find items to update quest or else
	
	X - How to say controler show interaction widget?

	X - Inventory

	X - UI to show interactions

	X - Door Interaction:
		X - Open, Close, Unlock, Lock, 
		X - If quick E open close, if hold else actions
		X - Talk
*/
#pragma endregion

#pragma region TASKS_DONE
/*
	V - While press M button open and close mini map
		V - Make input
		V - Set widget in BP
	
	V - Interaction Button
	V - Add GAS to Module_Character

*/
#pragma endregion

