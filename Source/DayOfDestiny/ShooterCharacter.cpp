// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	// Create a follow camera attached to the camera boom
	FollowCamera = CreateDefaultSubobject <UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);  // Attach camera to the end of the boom
	FollowCamera->bUsePawnControlRotation = false; // We don't want the camera to rotate relative to the arm

	// For now, don't rotate the character as the controller rotates - Let the controller rotate with the camera only
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	// Configure character movement (grabbing hold of the character movement component
	GetCharacterMovement()->bOrientRotationToMovement = true;  // Character moves in the direction of input
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

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRate * GetWorld()->GetDeltaSeconds());   // deg/sec * sec/frame  = deg/frame
}

void AShooterCharacter::LookupAtRate(float Rate)
{
	AddControllerPitchInput(Rate * LookupRate * GetWorld()->GetDeltaSeconds());  // deg/sec * sec/frame = deg/frame
}

