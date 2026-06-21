#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EntityHealthInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEntityHealthInterface : public UInterface
{
	GENERATED_BODY()
};

class MONSTERPARK_API IEntityHealthInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void UpdateHealthUI(float CurrentHealth);
};