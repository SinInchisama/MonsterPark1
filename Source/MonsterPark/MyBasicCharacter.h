// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AMyDetectionActor.h"
#include "MyBasicCharacter.generated.h"

UCLASS()
class MONSTERPARK_API AMyBasicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyBasicCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float value);
	void MoveRight(float value);

	void Set_PlayerMoney(int32 value);
	int32 Get_PlayerMoney();

	void SetSummonedActor(AActor* InActor);

	void Attack_Melee();
	void Attack_End();

protected:
	FTimerHandle TH_Attack_End;

public:
	// ���� ī�޶� �κ�
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category=Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;
		
protected:
	// ���� �÷��̾� ��, ����ġ, ������, ������ �ִ� �������� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerMoney = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AAMyDetectionActor* MySummonedActor;
};
