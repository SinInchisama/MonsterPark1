// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterPark/HitInterface.h"
#include "NiagaraSystem.h" 
#include "NiagaraFunctionLibrary.h"
#include "WallActor.generated.h"

UCLASS()
class MONSTERPARK_API AWallActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWallActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	class UBoxComponent* AttackZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	class UStaticMeshComponent* GateMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	class UBoxComponent* GateTriggerZone;

	UPROPERTY(EditAnywhere, Category = "Gate|Settings")
	float GateOpenHeight = 300.f;

	UPROPERTY(EditAnywhere, Category = "Gate|Settings")
	float GateMoveSpeed = 5.f; 

public:
	void TakeMonsterDamage(float DamageAmount, FVector AttackerLocation);
	FVector GetTargetLocation(FVector AttackerLocation);

private:
	bool bIsGateOpen = false;
	FVector InitialGateLocation;
	int32 OverlappingHeroCount = 0; 

	UFUNCTION()
	void OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnGateTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	float Health = 500.f;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* DestructionEffect;
};
