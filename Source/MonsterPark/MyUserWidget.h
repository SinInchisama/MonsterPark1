// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "CharacterBase.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void Btn_LevelUp_Clicked();

	UFUNCTION()
	void Btn_Reload_Clicked();

	UFUNCTION()
	void Btn_BuyHero_Clicked();

	void ProcessHeroPurchase(int32 Btn_Num);

	void ReFreshStore();

	void ReFreshMoney();

	UFUNCTION()
	void UpdateLife(int32 CurrentLife);

	UFUNCTION()
	void UpdateTimer(float Timer);
protected:
	UPROPERTY()
	class ABasicGameMode* CachedGM;

	UPROPERTY()
	TArray<TSubclassOf<ACharacterBase>> CurrentSlotClasses;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_LevelUp = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_Reload = nullptr;


	UPROPERTY()
	TArray<UButton*> Btn_BuyHero_Array;			// ���� BuyHero�� �����ϱ� ���� �迭

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_BuyHero0 = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_BuyHero1 = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_BuyHero2 = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_BuyHero3 = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* Btn_BuyHero4 = nullptr;


	UPROPERTY()
	TArray<class UTextBlock*> Text_HeroName_Array;	// ���� HeroName�� �����ϱ� ���� �迭

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroName0;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroName1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroName2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroName3;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroName4;

	UPROPERTY()
	TArray<class UTextBlock*> Text_HeroPrice_Array;	// ���� HeroPrice�� �����ϱ� ���� �迭

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroPrice0;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroPrice1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroPrice2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroPrice3;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_HeroPrice4;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Money;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RemainTime;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RemainLife;
};
