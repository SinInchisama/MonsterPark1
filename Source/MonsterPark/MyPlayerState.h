// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterBase.h"
#include "HeroChanceRow.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_ShopHeroes)
	TArray<TSubclassOf<ACharacterBase>> MyShopHeroes;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shop")
	FHeroChanceRow CurrentLevelChance;

	UPROPERTY()
	class UMyUserWidget* MyStoreWidget;
	
	UFUNCTION()
	void OnRep_ShopHeroes();

	UFUNCTION(Client, Reliable)
	void Client_NotifyShopRefreshed(const TArray<TSubclassOf<ACharacterBase>>& NewHeroes);
};
