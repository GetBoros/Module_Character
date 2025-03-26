#pragma once

#include "GameFramework/PlayerController.h"
#include "Module_Character_Controller.generated.h"

//-------------------------------------------------------------------------------------------------------------
UENUM(BlueprintType) enum class EButton_Pressed : uint8
{
	EBP_None,
	EBP_Exit,
	EBP_Mini_Map
};
//-------------------------------------------------------------------------------------------------------------
class UInputMappingContext;
class UInputAction;
class AAModule_Character;
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
	void Look(const FInputActionValue &value);
	void Zoom(const FInputActionValue &value);  // !!! Can be better | change from Vector2D to something else
	void Exit(const FInputActionValue &value);  // Restore Boom state || Menu || Q Button |
	void Jump(const FInputActionValue &value);
	void Mini_Map(const FInputActionValue &value);
	void Jump_Stop(const FInputActionValue &value);

	bool Is_Camera;
	AAModule_Character *Module_Character;  // !!! Maybe change to component if need

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
};
//-------------------------------------------------------------------------------------------------------------



#pragma region TASKS
/*
	- Interact from camera:
		- Set marks, find items to update quest or else

	- While press M button open and close mini map
		X - Make input 
		X - 

	- Door Interaction:
		- Open, Close, Unlock, Lock, 
		- If quick E open close, if hold else actions
		- Talk
*/
#pragma endregion
