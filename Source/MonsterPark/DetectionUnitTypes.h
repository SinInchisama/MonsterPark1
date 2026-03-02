#pragma once

#include "CoreMinimal.h"
#include "DetectionUnitTypes.generated.h"

UENUM(BlueprintType)
enum class EDetectionUnitType : uint8
{
    BlackCat,
    Knight,
    Thief,
    Archer,
	Wizard,
    Shielder,
	Berserker,
	Fighter,
};
