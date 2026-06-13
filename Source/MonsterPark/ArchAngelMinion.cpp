// Fill out your copyright notice in the Description page of Project Settings.


#include "ArchAngelMinion.h"
#include "GameFramework/CharacterMovementComponent.h"

AArchAngelMinion::AArchAngelMinion()
{
	DefaultAttackPower = 300.0f;
	if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
	{
		MoveComponent->bRunPhysicsWithNoController = true;
	}
}

void AArchAngelMinion::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		InitialRelativeOffset = OwnerActor->GetActorTransform().InverseTransformPosition(GetActorLocation());
		bHasInitialRelativeOffset = true;
	}
}

void AArchAngelMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (!bHasInitialRelativeOffset)
	{
		InitialRelativeOffset = OwnerActor->GetActorTransform().InverseTransformPosition(GetActorLocation());
		bHasInitialRelativeOffset = true;
	}

	const FVector TargetLocation = OwnerActor->GetActorTransform().TransformPosition(InitialRelativeOffset);
	const FVector MyLocation = GetActorLocation();
	const float FollowAcceptanceRadiusSq = FMath::Square(FollowAcceptanceRadius);

	if (FVector::DistSquared(MyLocation, TargetLocation) > FollowAcceptanceRadiusSq)
	{
		const FVector Direction = (TargetLocation - MyLocation).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
			{
				MoveComponent->MaxWalkSpeed = FollowSpeed;
			}

			const FVector NewLocation = FMath::VInterpConstantTo(MyLocation, TargetLocation, DeltaTime, FollowSpeed);
			SetActorLocation(NewLocation, true);
			SetActorRotation(Direction.Rotation());
			UpdateAnimBPSpeed(1);
		}
	}
	else
	{
		UpdateAnimBPSpeed(0);
	}
}

