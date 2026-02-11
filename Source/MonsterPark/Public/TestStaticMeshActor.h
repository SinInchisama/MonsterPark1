#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestStaticMeshActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class MONSTERPARK_API ATestStaticMeshActor : public AActor
{
	GENERATED_BODY()

public:
	ATestStaticMeshActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;
};
