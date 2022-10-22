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

	// Called when the fire button is pressed
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	void AimingButtonPressed();
	void AimingButtonReleased();


	void CameraInterpZoom(float DeltaTime);


	// Set TurnRate and LookupRate (i.e. the base turn rate and lookup rate) when aiming
	void SetLookRates();


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
	float BaseTurnRate;

	// This is the scaling parameter for looking up/down
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookupRate;

	// This is the scaling parameter when turning (only when using the mouse)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseMouseTurnRate;

	// This is the scaling parameter when looking up (only when using the mouse)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseMouseLookupRate;


	// Turn rate when hip firing (i.e. not aiming)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	// Lookup rate when hip firing (i.e. not aiming)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookupRate;

	// Turn rate when shoulder firing (i.e. when you are aiming while zoomed in and the gun is at shoulder height)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;


	// Lookup rate when shoulder firing (i.e. when you are aiming while zoomed in and the gun is at shoulder height)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookupRate;

	// Turn rate when hip firing (i.e. not aiming) - only when using the mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin="20.0", ClampMax="180.0", UIMin = "20.0", UIMax = "180.0"))
	float HipMouseTurnRate;

	// Lookup rate when hip firing (i.e. not aiming) - only when using the mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "20.0", ClampMax = "180.0", UIMin = "20.0", UIMax = "180.0"))
	float HipMouseLookupRate;

	// Turn rate when hip firing (i.e. not aiming) - only when using the mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "1.0", ClampMax = "60.0", UIMin = "1.0", UIMax = "60.0"))
	float AimingMouseTurnRate;

	// Lookup rate when hip firing (i.e. not aiming) - only when using the mouse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "1.0", ClampMax = "60.0", UIMin = "1.0", UIMax = "60.0"))
	float AimingMouseLookupRate;


	// Sound played when the weapon is fired
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	// Particle system that will be spawned when the fire button is pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	// Stores the montage to play when firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;


	// Stores the impact particle when something visible is hit by a line trace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;   // already forward declared above


	// Beam particles for the smoke trail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	// Are we aiming or not (will be used to zoom in on a key press
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;


	// Camera zoom when zoomed out
	float CameraDefaultFOV;

	// Camera zoom when zoomed in
	float CameraZoomedFOV;

	// Camera zoom value this frame
	float CameraCurrentFOV;

	// Camera zoom interpolation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;



	// Function used to look right/left at a specific turn rate
	// @Param Rate: This is normalised so 1.0 is a full turn rate and <1.0 for a lesser turn rate (good for physical game controllers)
	void TurnAtRate(float Rate);

	// Function used to look up/down at a specific turn rate
	// @Param Rate: This is normalised so 1.0 is a full turn rate and <1.0 for a lesser turn rate (good for physical game controllers)
	void LookupAtRate(float Rate);


	// Turn function used only when using the mouse
	void Turn(float Value);


	// Lookup function used only when using the mouse
	void Lookup(float Value);




public:
	// Returns the camera boom sub-object
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom;  }
	// Returns the camera sub-object
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

};
