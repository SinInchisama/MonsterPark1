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
    // �������Ʈ���� 'speed' ������ ����� �� �ְ� �����մϴ�.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float speed;

    // �� ������ ������Ʈ�� �ʿ��ϴٸ� �Ʒ� �Լ��� �������̵��Ͽ� ����� �� �ֽ��ϴ�.
    // virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
