// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


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
		

		// --- Calculating the movement offset yaw (i.e. the difference between the direction we are moving in and the direction we are aiming used for strafing animations in the anim BP)

		// This is the rotator corresponding to the ****direction we are aiming in, i.e. direction we are facing****
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();

		// This is the rotator corresponding to the ****direction we are moving in, i.e. our velocity vector****
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());  // based on the world x axis

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;   // Get the difference betweeen rotators - just the Yaw is needed which is a float

		if (ShooterCharacter->GetVelocity().Size() > 0.f) {
			LastMovementOffsetYaw = MovementOffsetYaw;      
		}
		


		/* --- Debug only ....
		FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);   

		FString MovementOffsetYawMessage = FString::Printf(TEXT("MovementOffsetYaw: %f"), MovementOffsetYaw);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow, MovementOffsetYawMessage);
		}
		}
		*/


	}
}
