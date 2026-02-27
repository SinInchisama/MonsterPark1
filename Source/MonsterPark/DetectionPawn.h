#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "DetectionPawn.generated.h"

UCLASS(Blueprintable, BlueprintType, ShowCategories=("Animation", "Collision"))
class MONSTERPARK_API ADetectionPawn : public ACharacterBase
{
    GENERATED_BODY()

public:
    ADetectionPawn();
};
