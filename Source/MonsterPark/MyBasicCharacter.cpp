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

	PlayerInputComponent->BindAxis("MoveForward",this,&AMyBasicCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyBasicCharacter::MoveRight);

    PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyBasicCharacter::OnMouseLeftClick);
}

void AMyBasicCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AMyBasicCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
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
    if (InActor)
    {
        MySummonedHero.Add ( Cast<AAMyDetectionActor>(InActor));
    } 
}

void AMyBasicCharacter::OnMouseLeftClick()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FHitResult HitResult;
        // 우리가 만든 SelectionSphere가 Visibility 채널을 Block 하도록 설정되어 있어야 합니다.
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            // 부딪힌 대상이 영웅 액터인지 확인
            AAMyDetectionActor* TouchedHero = Cast<AAMyDetectionActor>(HitResult.GetActor());

            if (TouchedHero)
            {
                SelectHero = TouchedHero;
                UE_LOG(LogTemp, Warning, TEXT("Hero Selected: %s"), *SelectHero->GetName());
            }
        }
    }
}