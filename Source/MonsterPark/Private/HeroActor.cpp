// Fill out your copyright notice in the Description page of Project Settings.

#include "HeroActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

AHeroActor::AHeroActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(Root);
	Capsule->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionObjectType(ECC_Pawn);
	Capsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	Capsule->SetGenerateOverlapEvents(true);
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AHeroActor::OnCapsuleBeginOverlap);
	Capsule->OnComponentHit.AddDynamic(this, &AHeroActor::OnCapsuleHit);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(Capsule);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AHeroActor::BeginPlay()
{
	Super::BeginPlay();

	Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
}

void AHeroActor::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroActor overlap with: %s"), *OtherActor->GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
				FString::Printf(TEXT("HeroActor overlap: %s"), *OtherActor->GetName()));
		}
	}
}

void AHeroActor::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroActor hit: %s"), *OtherActor->GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
				FString::Printf(TEXT("HeroActor hit: %s"), *OtherActor->GetName()));
		}
	}
}

