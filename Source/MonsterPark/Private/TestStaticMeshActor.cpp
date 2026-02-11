#include "TestStaticMeshActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ATestStaticMeshActor::ATestStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(StaticMesh);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATestStaticMeshActor::OnBoxBeginOverlap);
}

void ATestStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->UpdateOverlaps();
}

void ATestStaticMeshActor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		UE_LOG(LogTemp, Log, TEXT("TestStaticMeshActor overlap with: %s"), *OtherActor->GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
				FString::Printf(TEXT("TestStaticMesh overlap: %s"), *OtherActor->GetName()));
		}
	}
}
