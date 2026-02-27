// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBasicCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyBasicCharacter::AMyBasicCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = true;

    FVector CurrentLocation = GetActorLocation();
    CurrentLocation.Z = 1000.0f;
    SetActorLocation(CurrentLocation);
}

// Called when the game starts or when spawned
void AMyBasicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyBasicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyBasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("GameraMoveForward",this,&AMyBasicCharacter::GameraMoveForward);
	PlayerInputComponent->BindAxis("GameraMoveRight", this, &AMyBasicCharacter::GameraMoveRight);

    PlayerInputComponent->BindAxis("HeroMoveForward", this, &AMyBasicCharacter::HeroMoveForward);
    PlayerInputComponent->BindAxis("HeroMoveRight", this, &AMyBasicCharacter::HeroMoveRight);

    PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyBasicCharacter::OnMouseLeftClick);
}

void AMyBasicCharacter::GameraMoveForward(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, value);
    }
}

void AMyBasicCharacter::GameraMoveRight(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, value);
    }
}

void AMyBasicCharacter::HeroMoveForward(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        if (SelectHero) {
            SelectHero->MoveForward(value);
        }
    }
}

void AMyBasicCharacter::HeroMoveRight(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        if (SelectHero)
            SelectHero->MoveRight(value);
    }
}

void AMyBasicCharacter::Set_PlayerMoney(int32 value)
{
    PlayerMoney += value;
}

int32 AMyBasicCharacter::Get_PlayerMoney()
{
    return PlayerMoney;
}

void AMyBasicCharacter::SetSummonedActor(AActor* InActor)
{
	if (ACharacterBase* Character = Cast<ACharacterBase>(InActor))
	{
		MySummonedHero.Add(Character);
	}
}

void AMyBasicCharacter::OnMouseLeftClick()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FHitResult HitResult;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            ACharacterBase* TouchedHero = Cast<ACharacterBase>(HitResult.GetActor());

            if (TouchedHero)
            {
                SelectHero = TouchedHero;
            }
        }
    }
}