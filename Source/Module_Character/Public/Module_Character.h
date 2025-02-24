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

	void Zoom(const float step_offset);
	void Exit();  // Restore Boom state || Menu || Q Button |

	UFUNCTION(BlueprintCallable) void Camera_Switch(AActor *camera);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) USpringArmComponent *Camera_Boom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true") ) UCameraComponent *Camera_Follow;
};
//-------------------------------------------------------------------------------------------------------------
