// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterTargetFragment.generated.h"


/**
 * 
 */
USTRUCT()
struct MONSTERPARK_API FMonsterTargetFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector Target;

    int32 CurrentNodeIndex = -1;
    int32 PreviousNodeIndex = -1;

    int TargetIndex = 0;
    float SpawnTime = -1.f;
    bool bSpawned = false;
    bool Death = false;
    int MoveLocation = 0;
};

static const FVector NodeLocations[17] = {
    FVector(-1900, 1900, 60.f), FVector(0, 1900, 60.f), FVector(1900, 1900, 60.f),  
    FVector(-950, 950, 60.f),   FVector(0, 950, 60.f),  FVector(950, 950, 60.f),    
    FVector(-1900, 0, 60.f),    FVector(-950, 0, 60.f), FVector(0, 0, 60.f),        
    FVector(950, 0, 60.f),      FVector(1900, 0, 60.f),                             
    FVector(-950, -950, 60.f),  FVector(0, -950, 60.f), FVector(950, -950, 60.f),   
    FVector(-1900, -1900, 60.f),FVector(0, -1900, 60.f),FVector(1900, -1900, 60.f) 
};

static const int32 NodeNeighbors[17][4] = {
    {1, 6, -1, -1},      
    {0, 2, 4, -1},      
    {1, 10, -1, -1},      
    {4, 7, -1, -1},      
    {1, 3, 5, -1},        
    {4, 9, -1, -1},       
    {0, 7, 14, -1},       
    {3, 6, 11, -1},       
    {-1, -1, -1, -1},     
    {5, 10, 13, -1},     
    {2, 9, 16, -1},      
    {7, 12, -1, -1},      
    {11, 13, 15, -1},     
    {9, 12, -1, -1},      
    {6, 15, -1, -1},     
    {12, 14, 16, -1},     
    {10, 15, -1, -1}      
};