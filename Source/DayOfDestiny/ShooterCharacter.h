// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class DAYOFDESTINY_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// Basic functionality for forwards/backwards movement
	void MoveForward(float Value);

	// Basic functionality for left/right movement
	void MoveRight(float Value);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Camera boom positioning the camera behind the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// This is the scaling parameter for turning left/right
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float TurnRate;

	// This is the scaling parameter for looking up/down
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float LookupRate;

	// Function used to look right/left at a specific turn rate
	// @Param Rate: This is normalised so 1.0 is a full turn rate and <1.0 for a lesser turn rate (good for physical game controllers)
	void TurnAtRate(float Rate);

	// Function used to look up/down at a specific turn rate
	// @Param Rate: This is normalised so 1.0 is a full turn rate and <1.0 for a lesser turn rate (good for physical game controllers)
	void LookupAtRate(float Rate);


public:
	// Returns the camera boom sub-object
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom;  }
	// Returns the camera sub-object
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};
