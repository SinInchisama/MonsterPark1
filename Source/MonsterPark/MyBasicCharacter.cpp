// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBasicCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"
#include "BasicGameMode.h"
#include "MyPlayerState.h"

#include "Net/UnrealNetwork.h"

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
    PlayerInputComponent->BindAction("HeroMixture", IE_Pressed, this, &AMyBasicCharacter::HeroMixture);
}

void AMyBasicCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyBasicCharacter, MySummonedHero);
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

void AMyBasicCharacter::Server_HeroMoveForward_Implementation(ACharacterBase* TargetHero, float val)
{
    if (TargetHero)
    {
        TargetHero->MoveForward(val);
    }
}

void AMyBasicCharacter::Server_HeroMoveRight_Implementation(ACharacterBase* TargetHero, float val)
{
    if (TargetHero)
    {
        TargetHero->MoveRight(val);
    }
}

void AMyBasicCharacter::HeroMoveForward(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        if (SelectHero) {
            SelectHero->MoveForward(value);

            Server_HeroMoveForward(SelectHero, value);
        }
    }
}

void AMyBasicCharacter::HeroMoveRight(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        if (SelectHero) {
            SelectHero->MoveRight(value);

            Server_HeroMoveRight(SelectHero, value);
        }
    }
}



int32 AMyBasicCharacter::Get_PlayerLife()
{
    return PlayerLife;
}

void AMyBasicCharacter::Miu_PlayerLife(int32 value)
{
    PlayerLife -= value;
    OnLifeChanged.Broadcast(PlayerLife);
}

int32 AMyBasicCharacter::Get_PlayerExp()
{
    return PlayerExp;
}

void AMyBasicCharacter::Set_PlayerExp(int32 value)
{
    PlayerExp += value;
}

bool AMyBasicCharacter::CheckLevelUp()
{
    return  (PlayerExp+2) == PlayerMaxExp;
}

bool AMyBasicCharacter::PlayerLevelUp()
{
    PlayerExp = 0;
    PlayerLevel += 1;

    ABasicGameMode* GM = Cast<ABasicGameMode>(GetWorld()->GetAuthGameMode());
    if(GM)
    {
        //GM->OnLevelUp(CurrentLevelChance, PlayerLevel);
    }

    return PlayerLevel == 3;
}

void AMyBasicCharacter::SetSummonedActor(AActor* InActor)
{
	if (ACharacterBase* Character = Cast<ACharacterBase>(InActor))
	{
		MySummonedHero.Add(Character);
	}
}

void AMyBasicCharacter::Server_RequestShopRefresh_Implementation()
{
    ABasicGameMode* GM = GetWorld()->GetAuthGameMode<ABasicGameMode>();
    if (GM)
    {
        GM->RefreshIndividualShop(Cast<APlayerController>(GetController()));
    }
}

bool AMyBasicCharacter::Server_RequestPurchaseHero_Validate(int32 SlotIndex) {
    return true; 
}

void AMyBasicCharacter::Server_RequestPurchaseHero_Implementation(int32 SlotIndex)
{
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (!PS || !PS->MyShopHeroes.IsValidIndex(SlotIndex)) return;

    TSubclassOf<ACharacterBase> SelectedClass = PS->MyShopHeroes[SlotIndex];
    if (!SelectedClass) return;

    int32 Price = 0;
    if (ACharacterBase* DefaultHero = SelectedClass->GetDefaultObject<ACharacterBase>())
    {
        Price = static_cast<int32>(DefaultHero->DefaultCost);
    }

    if (PS->Money >= Price)
    {
        PS->Money-=Price;

        if (ABasicGameMode* GM = GetWorld()->GetAuthGameMode<ABasicGameMode>())
        {
            GM->SpawnHeroFromShop(SelectedClass, this);
        }

        PS->MyShopHeroes[SlotIndex] = nullptr; 
        
        PS->Client_NotifyShopRefreshed(PS->MyShopHeroes);
    }
}

void AMyBasicCharacter::OnMouseLeftClick()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FHitResult HitResult;
        if (PC->GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
        {
            ACharacterBase* TouchedHero = Cast<ACharacterBase>(HitResult.GetActor());

            if (TouchedHero)
            {
                if (MySummonedHero.Contains(TouchedHero))
                {
                    SelectHero = TouchedHero;
                }
            }
        }
    }
}

void AMyBasicCharacter::HeroMixture()
{
    if (SelectHero)
    {
        FString Name = SelectHero->UnitName.ToString();
        TArray<ACharacterBase*> Mixture;
        for (auto Hero : MySummonedHero)
        {
            if (Name == Hero->UnitName.ToString())
            {
                Mixture.Add(Hero);
            }
        }
        if (Mixture.Num() >= 3)
        {
            for (int32 i = 0; i < 3; ++i)
            {
                ACharacterBase* TargetHero = Mixture[i];

                if (IsValid(TargetHero))
                {
                    MySummonedHero.Remove(TargetHero);

                    TargetHero->Destroy();
                }

                SelectHero = nullptr;
            }
            Mixtured.Broadcast(1);
        }
    }
}

