#pragma once

#include "GameFramework/PlayerController.h"
#include "Module_Character_Controller.generated.h"

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
	void Jump_Stop(const FInputActionValue &value);

	AAModule_Character *Module_Character;  // !!! Maybe change to component if need

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputMappingContext *Mapping_Context;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Jump;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Move;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Look;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Zoom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true") ) UInputAction *Action_Exit;
};
//-------------------------------------------------------------------------------------------------------------



#pragma region TASKS
/*
X	- Create in module character BP Controller and BP Character and spec Folder
V	- AAModule_Character_Controller must have ptr to Owner
*/
#pragma endregion
