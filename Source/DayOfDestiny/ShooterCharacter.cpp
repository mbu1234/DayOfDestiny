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

	// Base rates for turning / looking up - by default, we are not aiming
	BaseTurnRate(45.f),
	BaseLookupRate(30.f),
	BaseMouseTurnRate(45.f),
	BaseMouseLookupRate(30.f),

	// True when aiming with the weapon
	bIsAiming(false),

	// Turn and lookup rates for aiming / not aiming
	HipTurnRate(90.f),
	HipLookupRate(90.f),
	AimingTurnRate(20.f),
	AimingLookupRate(20.f),

	// Turn and lookup rates for aiming / not aiming - only when using the mouse
	HipMouseTurnRate(90.f),
	HipMouseLookupRate(90.f),
	AimingMouseTurnRate(20.f),
	AimingMouseLookupRate(20.f),

	// Camera FOV values
	CameraDefaultFOV(0.f),    // Only 0 temporarily to initialize it, will set it in BeginPlay (when we know we have the camera)
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),   // Only a temp value (will set in BeginPlay once the camera is valid)
	ZoomInterpSpeed(20.f)




{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;  // Camera is following at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;  // Rotate the arm based on the controller's rotation
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);  // Moving the camera boom 50 cms to the right (Y) and 50 cm up (Z) so we can see our crosshairs in the centre

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
	CameraDefaultFOV = FollowCamera->FieldOfView;
	CameraCurrentFOV = CameraDefaultFOV;
	
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


		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(BarrelSocketTransform.GetLocation(), BeamEnd);

			if (bBeamEnd) {
				if (ImpactParticles) {
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
				}


				if (BeamParticles) {
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, BarrelSocketTransform);

					if (Beam) {
						Beam->SetVectorParameter(FName("Target"), BeamEnd);
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




bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{

	// We are going to line trace from the world position of the crosshairs to the FHitResult's location
   // We'll also spawn the beamparticles but this will be from the barrel socket  and will extend it to the FHitResult location
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

	if (bScreenToWorld) { // If the project from screen to world is successful .....
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairsWorldPosition };
		const FVector End{ CrosshairsWorldPosition + CrosshairsWorldDirection * 50'000.f };
		OutBeamLocation = End;

		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// If we get a blocking hit, spawn the impact particles and the FHitResult location
		if (ScreenTraceHit.bBlockingHit) {
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// We need to perform a second trace from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };

		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit) {  // Is there an object between the barrel and BeamEndPoint?
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;   // i.e. return true if the projection is successfull which guarantees we have done a line trace
	}

	return false;  // By default return false - shouldn't happen but if the de-projection is not successfull, false is returned
}

void AShooterCharacter::AimingButtonPressed()
{
	bIsAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bIsAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bIsAiming) {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else {
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bIsAiming) {
		BaseTurnRate = AimingTurnRate;
		BaseLookupRate = AimingLookupRate;

		BaseMouseTurnRate = AimingMouseTurnRate;
		BaseMouseLookupRate = AimingMouseLookupRate;
	}
	else {
		BaseTurnRate = HipTurnRate;
		BaseLookupRate = HipLookupRate;

		BaseMouseTurnRate = HipMouseTurnRate;
		BaseMouseLookupRate = HipMouseLookupRate;
	}
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set camera current field of view via interpolation
	CameraInterpZoom(DeltaTime);

	SetLookRates(); // Change look sensitivity based on Aiming
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

		PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AShooterCharacter::Turn);
		PlayerInputComponent->BindAxis(TEXT("Lookup"), this, &AShooterCharacter::Lookup);

		PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
		PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ACharacter::StopJumping);

		PlayerInputComponent->BindAction(TEXT("FireButton"), EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);

		PlayerInputComponent->BindAction(TEXT("AimingButton"), EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
		PlayerInputComponent->BindAction(TEXT("AimingButton"), EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());   // deg/sec * sec/frame  = deg/frame
}

void AShooterCharacter::LookupAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookupRate * GetWorld()->GetDeltaSeconds());  // deg/sec * sec/frame = deg/frame
}

void AShooterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * BaseMouseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Lookup(float Value)
{
	AddControllerPitchInput(Value * BaseMouseLookupRate * GetWorld()->GetDeltaSeconds());
}

