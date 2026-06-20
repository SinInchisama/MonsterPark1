// Fill out your copyright notice in the Description page of Project Settings.


#include "WallActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MonsterPark/PlaySubSystem.h"
#include "Math/UnrealMathUtility.h"
#include "MonsterPark/CharacterBase.h"

// Sets default values
AWallActor::AWallActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = MeshComponent;

	AttackZone = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackZone"));
	AttackZone->SetupAttachment(RootComponent);

	GateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMeshComponent->SetupAttachment(RootComponent);

	GateTriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GateTriggerZone"));
	GateTriggerZone->SetupAttachment(GateMeshComponent);
	GateTriggerZone->SetCollisionProfileName(TEXT("Trigger"));

	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	GateMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	GateTriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AWallActor::OnGateTriggerBeginOverlap);
	GateTriggerZone->OnComponentEndOverlap.AddDynamic(this, &AWallActor::OnGateTriggerEndOverlap);
}

// Called when the game starts or when spawned
void AWallActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->ActiveWalls.Add(this);
		}
	}

	if (GateMeshComponent)
	{
		InitialGateLocation = GateMeshComponent->GetRelativeLocation();
	}
}

void AWallActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->ActiveWalls.RemoveSwap(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWallActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GateMeshComponent) return;

	FVector TargetLocation = bIsGateOpen ? (InitialGateLocation + FVector(0.f, 0.f, GateOpenHeight)) : InitialGateLocation;
	FVector CurrentLocation = GateMeshComponent->GetRelativeLocation();

	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, GateMoveSpeed);
	GateMeshComponent->SetRelativeLocation(NewLocation);

	if (CurrentLocation.Equals(TargetLocation, 1.0f))
	{
		GateMeshComponent->SetRelativeLocation(TargetLocation); 
		SetActorTickEnabled(false); 
	}
}

void AWallActor::TakeMonsterDamage(float DamageAmount, FVector AttackerLocation)
{
	Health -= DamageAmount;

	if (Health <= 0.f)
	{
		FVector ExplosionLocation = AttackZone ? AttackZone->GetComponentLocation() : GetActorLocation();

		if (DestructionEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				DestructionEffect,
				ExplosionLocation, 
				FRotator::ZeroRotator
			);
		}
		Destroy();
	}
}

FVector AWallActor::GetTargetLocation(FVector AttackerLocation)
{
	if (AttackZone)
	{
		FVector ClosestPoint;
		AttackZone->GetClosestPointOnCollision(AttackerLocation, ClosestPoint);
		return ClosestPoint;
	}

	return GetActorLocation();
}

void AWallActor::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	if (ACharacterBase* Hero = Cast<ACharacterBase>(OtherActor))
	{
		OverlappingHeroCount++;

		if (OverlappingHeroCount > 0 && !bIsGateOpen)
		{
			bIsGateOpen = true;
			SetActorTickEnabled(true); 
		}
	}
}

void AWallActor::OnGateTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this) return;

	if (ACharacterBase* Hero = Cast<ACharacterBase>(OtherActor))
	{
		OverlappingHeroCount--;

		if (OverlappingHeroCount <= 0 && bIsGateOpen)
		{
			OverlappingHeroCount = 0; 
			bIsGateOpen = false;
			SetActorTickEnabled(true); 
		}
	}
}

