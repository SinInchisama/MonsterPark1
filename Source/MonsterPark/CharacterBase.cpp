// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add the ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void ACharacterBase::PossessedBy(AController* NewController)
//{
//	Super::PossessedBy(NewController);
//	if (AbilitySystemComponent)
//	{
//		AbilitySystemComponent->RefreshAbilityActorInfo();
//	}
//}
//
//void ACharacterBase::OnRep_PlayerState()
//{
//	Super::OnRep_PlayerState();
//	if (AbilitySystemComponent)
//	{
//		AbilitySystemComponent->RefreshAbilityActorInfo();
//	}
//}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}