// Fill out your copyright notice in the Description page of Project Settings.


#include "ArchAngelMinion.h"
#include "GameFramework/CharacterMovementComponent.h"

void AArchAngelMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const FVector MyLocation = GetActorLocation();
	const float FollowDistanceSq = FMath::Square(FollowDistance);

	if (FVector::DistSquared(MyLocation, OwnerLocation) > FollowDistanceSq)
	{
		FVector Direction = (OwnerLocation - MyLocation).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
			{
				MoveComponent->MaxWalkSpeed = FollowSpeed;
			}
			AddMovementInput(Direction, 1.0f);
		}
	}
}

