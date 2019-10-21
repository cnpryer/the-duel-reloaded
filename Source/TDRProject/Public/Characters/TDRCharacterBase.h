// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDRCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UAnimSequence;

UCLASS()
class TDRPROJECT_API ATDRCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDRCharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UStaticMeshComponent* MeshComp;
	
	virtual void BeginPlay() override;

protected:
	
	FTimerHandle TimerHandle;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void DodgeForward();
	void DodgeBackward();
	void DodgeRight();
	void DodgeLeft();
	void InteractPressed();
	void StopCurrentAnimation();
  
  UPROPERTY(EditAnywhere, Category = "Debug")
	bool Debug;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseLookupAtRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseDodgeMultiplier;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* DodgeForwardAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* DodgeBackwardAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* DodgeRightAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* DodgeLeftAnim;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float TraceDistance;

	UFUNCTION(BlueprintNativeEvent)
	void TraceForward();
	void TraceForward_Implementation();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:	

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	AActor* FocusedActor;
};
