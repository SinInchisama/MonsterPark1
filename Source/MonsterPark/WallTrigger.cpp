// Fill out your copyright notice in the Description page of Project Settings.


#include "WallTrigger.h"
#include "Components/StaticMeshComponent.h"
#include "CharacterBase.h"

// Sets default values
AWallTrigger::AWallTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	RootComponent = TriggerMesh;

	TriggerMesh->SetCollisionProfileName(TEXT("Trigger"));
	TriggerMesh->SetGenerateOverlapEvents(true);

	TriggerMesh->SetHiddenInGame(true);

	TriggerMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	TriggerMesh->OnComponentBeginOverlap.AddDynamic(this, &AWallTrigger::OnMeshOverlap);
}


void AWallTrigger::OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		if (ACharacterBase* Hero = Cast<ACharacterBase>(OtherActor))
		{
			Hero->SetIsOutside(bIsOutsideTrigger);
		}
	}
}