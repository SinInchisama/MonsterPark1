// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBasicCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DefaultPawn.h"

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

	// "Attack" 액션을 바인딩합니다. 언리얼 에디터의 Project Settings -> Input에서 "Attack" 액션을 설정해야 합니다.
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyBasicCharacter::Attack_Melee);
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
        MySummonedActor = Cast<AAMyDetectionActor>(InActor);
    }
}

void AMyBasicCharacter::Attack_Melee()
{
    UE_LOG(LogTemp, Warning, TEXT("Attack_Melee Called!"));
    GetWorldTimerManager().SetTimer(TH_Attack_End, this, &AMyBasicCharacter::Attack_End, 1.0f, false);
}

void AMyBasicCharacter::Attack_End()
{
    UE_LOG(LogTemp, Warning, TEXT("Attack_End Called!"));
    // Implement animation ending logic here
}
