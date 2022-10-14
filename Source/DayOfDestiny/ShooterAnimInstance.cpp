// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UShooterAnimInstance::NativeInitializeAnimation() {

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)   // custom function that will be called from the child AnimBP (i.e. the usual Update Animation node)
{
	if (ShooterCharacter == nullptr) {
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter) {
		// Get the lateral speed of the character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;

		Speed = Velocity.Size();


		// Is the character in the air
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		// Is the character accelerating
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f) {
			bIsAccelerating = true;  // character is moving
		}
		else {
			bIsAccelerating = false;  // character is standing still
		}
		
	}
}
