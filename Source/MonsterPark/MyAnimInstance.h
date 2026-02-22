// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    // 블루프린트에서 'speed' 변수를 사용할 수 있게 노출합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float speed;

    // 매 프레임 업데이트가 필요하다면 아래 함수를 오버라이드하여 사용할 수 있습니다.
    // virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
