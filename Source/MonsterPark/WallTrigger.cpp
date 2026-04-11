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
		// 영웅 캐릭터인지 확인
		if (ACharacterBase* Hero = Cast<ACharacterBase>(OtherActor))
		{
			// 영웅의 타겟팅 쿼리 포인터를 변경하는 함수 호출
			Hero->SetIsOutside(bIsOutsideTrigger);

			UE_LOG(LogTemp, Warning, TEXT("Hero crossed the wall! Outside: %s"), bIsOutsideTrigger ? TEXT("True") : TEXT("False"));
		}
	}
}