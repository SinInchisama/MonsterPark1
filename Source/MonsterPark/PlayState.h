// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BasicGameMode.h"
#include "PlayState.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API APlayState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	APlayState();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentRound, BlueprintReadOnly, Category = "Match")
	int32 CurrentRound;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "Match")
	int32 RemainingTime;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "Match")
	EMatchState ReplicatedMatchState;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_MatchState();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChangedSignature, int32, NewValue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerChangedSignature, int32, NewValue);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundChangedSignature OnRoundChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimerChangedSignature OnTimerChanged;
};
