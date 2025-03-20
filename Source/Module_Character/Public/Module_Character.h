#pragma once

#include "GameFramework/Character.h"
#include "Module_Character.generated.h"

//-------------------------------------------------------------------------------------------------------------
class USpringArmComponent;
class UCameraComponent;
//-------------------------------------------------------------------------------------------------------------
UCLASS(config = Game) class AAModule_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AAModule_Character();

	virtual void BeginPlay();
	virtual void NotifyControllerChanged();
	//virtual void SetupPlayerInputComponent(UInputComponent *player_input_component);

	void Look(const FVector2D look_axis_vector);
	void Zoom(const float step_offset);
	void Camera_Exit();  // Restore Boom state || Menu || Q Button |

	bool Is_State_Camera;  // Remove to switch if have more states

	UFUNCTION(BlueprintCallable) void Camera_Switch(const FVector location, const FRotator rotation);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) USpringArmComponent *Camera_Boom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) UCameraComponent *Camera_Follow;
};
//-------------------------------------------------------------------------------------------------------------
