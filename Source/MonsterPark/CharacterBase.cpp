// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "MonsterAttributeSet.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add the ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackPowerAttribute(), DefaultAttackPower);
		AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackSpeedAttribute(), DefaultAttackSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetRangeAttribute(), DefaultRange);
		AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetCostAttribute(), DefaultCost);
	}
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACharacterBase::Attack_Melee);
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::Attack_Melee()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack_Melee Called!"));
	GetWorldTimerManager().SetTimer(TH_Attack_End, this, &ACharacterBase::Attack_End, 1.0f, false);
}

void ACharacterBase::Attack_End()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack_End Called!"));
}