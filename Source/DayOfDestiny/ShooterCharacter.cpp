// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :

	TurnRate(45.f),
	LookupRate(30.f)

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;  // Camera is following at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;  // Rotate the arm based on the controller's rotation
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);  // Moving the camera boom 50 cms to the right (Y) and 50 cm up (Z) so we can see our crosshairs in the centre

	// Create a follow camera attached to the camera boom
	FollowCamera = CreateDefaultSubobject <UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);  // Attach camera to the end of the boom
	FollowCamera->bUsePawnControlRotation = false; // We don't want the camera to rotate relative to the arm

	// For now, We will rotate the character as the controller rotates (we don't want the character to pitch and roll with the controller though)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// Configure character movement (grabbing hold of the character movement component
	GetCharacterMovement()->bOrientRotationToMovement = false;  // We no longer want to rotate the character to our movement
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);  // ... at this rotation rate.
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

}

// Called when the game starts or when spawned 
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void AShooterCharacter::MoveForward(float Value)
{
	FRotator ControlRotation{ Controller->GetControlRotation() };
	FRotator YawRotation{ 0.f, ControlRotation.Yaw, 0.f };

	FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };
	AddMovementInput(Direction, Value);
}


void AShooterCharacter::MoveRight(float Value)
{
	FRotator ControlRotation{ Controller->GetControlRotation() };
	FRotator YawRotation{ 0.f, ControlRotation.Yaw, 0.f };

	FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };
	AddMovementInput(Direction, Value);

}

void AShooterCharacter::FireWeapon()
{
	if (FireSound) {
		UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
	}

	// Spawning particle system - we need to get the barrel socket, then its transform which is used as a param for SpawnEmmiterAtLocation
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName(TEXT("BarrelSocket"));   

	if (BarrelSocket) {
		const FTransform BarrelSocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, BarrelSocketTransform);
		}

		/*  --------- Not using the code below anymore -----------------
		
		FHitResult FireHit;
		FVector Start{ BarrelSocketTransform.GetLocation() };
		FQuat Rotation{ BarrelSocketTransform.GetRotation() };
		const FVector RotationAxis{ Rotation.GetAxisX() };
		FVector End{ Start + RotationAxis * 50'000.f };

		FVector BeamEndPoint{ End };

		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (FireHit.bBlockingHit) {
			if (ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}

			BeamEndPoint = FireHit.Location;

			if (BeamParticles) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, BarrelSocketTransform);
				if (Beam) {
					Beam->SetVectorParameter(TEXT("Target"), BeamEndPoint);  // We get "Target" from the particle system itself
				}
			}
		}

		*/

		// We are going to line trace from the world position of the crosshairs to the FHitResult's location
		// We'll also spawn the beamparticles but this will be from the barrel socket  and will extend it to the FHitResult location

		// Get the current size of the viewport
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport) {
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		FVector2D CrosshairLocations(ViewportSize.X / 2, ViewportSize.Y / 2); // initially, get the centre of the screen
		CrosshairLocations.Y -= 50.f;  // Remember, we subtracted 50 cm from the Y location in the HUD BP

		// The next 3 lines will get the world position and direction of the crosshairs 
		FVector CrosshairsWorldPosition;
		FVector CrosshairsWorldDirection;
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocations, CrosshairsWorldPosition, CrosshairsWorldDirection);

		// Set up the line trace
		if (bScreenToWorld) { // If the project from screen to world is successful .....
			FHitResult ScreenTraceHit;
			const FVector Start{ CrosshairsWorldPosition };
			const FVector End{ CrosshairsWorldPosition + CrosshairsWorldDirection * 50'000.f };
			FVector BeamEndPoint{ End };
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

			// If we get a blocking hit, spawn the impact particles and the FHitResult location
			if (ScreenTraceHit.bBlockingHit) {
				BeamEndPoint = ScreenTraceHit.Location;

				if (ImpactParticles) {
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ScreenTraceHit.Location);
				}

			}

			// Spawn the beam particles from the barrel of the gun and extend to the FHitResult's location
			if (BeamParticles) {
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, BarrelSocketTransform);
				if (Beam) {
					Beam->SetVectorParameter(TEXT("Target"), BeamEndPoint);
				}
			}

		}


	}


	// Play the 'hip fire' montage (using functions from the anim instance)
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (AnimInstance && HipFireMontage) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(TEXT("StartFire"));
	}

}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent)    // Use this macro to halt execution if the PlayerInputComponent is null

		PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
		PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);

		PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AShooterCharacter::TurnAtRate);
		PlayerInputComponent->BindAxis(TEXT("LookupAtRate"), this, &AShooterCharacter::LookupAtRate);

		PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
		PlayerInputComponent->BindAxis(TEXT("Lookup"), this, &APawn::AddControllerPitchInput);

		PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
		PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ACharacter::StopJumping);

		PlayerInputComponent->BindAction(TEXT("FireButton"), EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRate * GetWorld()->GetDeltaSeconds());   // deg/sec * sec/frame  = deg/frame
}

void AShooterCharacter::LookupAtRate(float Rate)
{
	AddControllerPitchInput(Rate * LookupRate * GetWorld()->GetDeltaSeconds());  // deg/sec * sec/frame = deg/frame
}

