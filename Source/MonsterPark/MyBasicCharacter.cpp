// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBasicCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"
#include "BasicGameMode.h"
#include "InputCoreTypes.h"
#include "Game_HUD.h"

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

    CurrentLevelChance.Cost1 = 100.0f;
}

// Called when the game starts or when spawned
void AMyBasicCharacter::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        MyHUD = Cast<AGame_HUD>(PC->GetHUD());
    }
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

    PlayerInputComponent->BindAxis("GameraMoveForward", this, &AMyBasicCharacter::GameraMoveForward);
    PlayerInputComponent->BindAxis("GameraMoveRight", this, &AMyBasicCharacter::GameraMoveRight);

    PlayerInputComponent->BindAxis("HeroMoveForward", this, &AMyBasicCharacter::HeroMoveForward);
    PlayerInputComponent->BindAxis("HeroMoveRight", this, &AMyBasicCharacter::HeroMoveRight);

    PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyBasicCharacter::OnLeftClickPressed);
    PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &AMyBasicCharacter::OnLeftClickReleased);
    PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AMyBasicCharacter::OnMouseRightClick);
    PlayerInputComponent->BindAction("HeroMixture", IE_Pressed, this, &AMyBasicCharacter::HeroMixture);
    PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AMyBasicCharacter::OnSkillPressed);

    PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AMyBasicCharacter::OpenMenu);
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
        for (ACharacterBase* Hero : SelectedHeroes)
        {
            if (IsValid(Hero)) Hero->MoveForward(value);
        }
    }
}

void AMyBasicCharacter::HeroMoveRight(float value)
{
    if ((Controller != nullptr) && (value != 0.0f))
    {
        for (ACharacterBase* Hero : SelectedHeroes)
        {
            if (IsValid(Hero)) Hero->MoveRight(value);
        }
    }
}

void AMyBasicCharacter::OnSkillPressed()
{
    if (SelectHero)
    {
        SelectHero->UseSkill();
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
    return  (PlayerExp + 2) == PlayerMaxExp;
}

bool AMyBasicCharacter::PlayerLevelUp()
{
    PlayerExp = 0;
    PlayerLevel += 1;

    ABasicGameMode* GM = Cast<ABasicGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->OnLevelUp(CurrentLevelChance, PlayerLevel);
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

void AMyBasicCharacter::OnLeftClickPressed()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        PC->GetMousePosition(StartMousePosition.X, StartMousePosition.Y);

        if (MyHUD)
        {
            MyHUD->StartDrawPoint = StartMousePosition;
            MyHUD->bIsDrawing = true;
        }
    }
}

void AMyBasicCharacter::OnLeftClickReleased()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (MyHUD)
    {
        MyHUD->bIsDrawing = false;
    }

    PC->GetMousePosition(EndMousePosition.X, EndMousePosition.Y);

    for (ACharacterBase* Hero : SelectedHeroes)
    {
        if (IsValid(Hero))
        {
            Hero->SetSelectedHero(false);
        }
    }
    SelectedHeroes.Empty();
    SelectHero = nullptr;

    float DragDistance = FVector2D::Distance(StartMousePosition, EndMousePosition);
    const float DragThreshold = 15.0f;

    if (DragDistance < DragThreshold)
    {
        FHitResult HitResult;
        if (PC->GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
        {
            ACharacterBase* TouchedHero = Cast<ACharacterBase>(HitResult.GetActor());
            if (TouchedHero)
            {
                SelectHero = TouchedHero;
                SelectHero->SetSelectedHero(true);
                SelectedHeroes.Add(SelectHero);
                OnUnitSelected.Broadcast(SelectHero, true);
            }
            else
            {
                OnUnitSelected.Broadcast(nullptr, false);
            }
        }
        else
        {
            OnUnitSelected.Broadcast(nullptr, false);
        }
    }
    else
    {
        float MinX = FMath::Min(StartMousePosition.X, EndMousePosition.X);
        float MaxX = FMath::Max(StartMousePosition.X, EndMousePosition.X);
        float MinY = FMath::Min(StartMousePosition.Y, EndMousePosition.Y);
        float MaxY = FMath::Max(StartMousePosition.Y, EndMousePosition.Y);

        for (ACharacterBase* Hero : MySummonedHero)
        {
            if (!IsValid(Hero)) continue;

            FVector2D ScreenPosition;
            if (PC->ProjectWorldLocationToScreen(Hero->GetActorLocation(), ScreenPosition))
            {
                if (ScreenPosition.X >= MinX && ScreenPosition.X <= MaxX &&
                    ScreenPosition.Y >= MinY && ScreenPosition.Y <= MaxY)
                {
                    Hero->SetSelectedHero(true);
                    SelectedHeroes.Add(Hero);
                }
            }
        }
        if (SelectedHeroes.Num() > 0)
        {
            SelectHero = SelectedHeroes[0];
            OnUnitSelected.Broadcast(SelectHero, true);
        }
        else
        {
            OnUnitSelected.Broadcast(nullptr, false);
        }
    }
}

void AMyBasicCharacter::OnMouseRightClick()
{
    if (SelectedHeroes.Num() == 0)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FHitResult HitResult;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            for (ACharacterBase* Hero : SelectedHeroes)
            {
                if (IsValid(Hero))
                {
                    Hero->CommandMoveToLocation(HitResult.Location);
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

void AMyBasicCharacter::OpenMenu()
{
    ABasicGameMode* GM = Cast<ABasicGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GM)
    {
        // ???(Toggle) ??????? ??? ???? ??? ???
        GM->ToggleMenuUI();
    }
}

