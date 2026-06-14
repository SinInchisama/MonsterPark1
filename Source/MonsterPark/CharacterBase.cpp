// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"
#include "Async/Async.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/DecalComponent.h"

#include "TimerManager.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "MyAnimInstance.h"

#include "GameplayAbilitySpec.h"

#include "MonsterAttributeSet.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"

#include "MassEntityView.h"

ACharacterBase::ACharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;


    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

    AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));

    GetCapsuleComponent()->InitCapsuleSize(50.f, 100.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    GetCharacterMovement()->MaxWalkSpeed = 200.f;
    GetCharacterMovement()->bOrientRotationToMovement = true; 

    GetCharacterMovement()->SetWalkableFloorAngle(80.0f);

    GetCharacterMovement()->MaxStepHeight = 100.0f;

    SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
    SelectionDecal->SetupAttachment(RootComponent);

    SelectionDecal->SetHiddenInGame(true);

    SelectionDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        CachedPlayerChar = Cast<AMyBasicCharacter>(PC->GetPawn());
    }

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackPowerAttribute(), DefaultAttackPower);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackSpeedAttribute(), DefaultAttackSpeed);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetRangeAttribute(), DefaultRange);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetCostAttribute(), DefaultCost);
    }

    GrantDefaultAbilities();

    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (EntitySubsystem)
    {
        FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
        EnemyQuery = FMassEntityQuery(EntityManager.AsShared());

        EnemyOutsideQuery = FMassEntityQuery(EntityManager.AsShared());
    }

    UWorld* World = GetWorld();
    if (World)
    {
        PlaySubsystem = World->GetSubsystem<UPlaySubSystem>();
    }

    EnemyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);

    EnemyQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::None);
    EnemyQuery.AddTagRequirement<FMonsterDyingTag>(EMassFragmentPresence::None);

    TargetQueryPtr = &EnemyQuery;

    if (!AnimClass)
    {
        AnimClass = LoadDefaultAnimClass();
    }

    if (World )
    {
        if (PlaySubsystem)
        {
            FName SearchName = FName(*UnitName.ToString());
            int32 CurrentGlobalLevel = PlaySubsystem->GetHeroUpgradeLevel(SearchName);

            if (CurrentGlobalLevel > 0)
            {
                ApplyUpgradeStats(CurrentGlobalLevel);
            }
        }
    }

    CurrentHealth = MaxHealth;

    SetMoveAnimClassIfNeeded();
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (PlaySubsystem && LastGridKey != -1)
    {
        PlaySubsystem->RemoveHeroFromGrid(this, LastGridKey);
    }
    Super::EndPlay(EndPlayReason);
}

void ACharacterBase::OnResumeAction()
{
    Attacking = true;
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector MoveDirection = FVector(CurrentForwardInput, CurrentRightInput, 0.0f);

    if (!MoveDirection.IsNearlyZero() && Attacking)
    {
        bIsMovingOnPath = false;
        PathToFollow.Empty();

        AddMovementInput(MoveDirection.GetSafeNormal(), 1.0f);

        if (PlaySubsystem)
        {
            PlaySubsystem->UpdateHeroLocation(this, LastGridKey, GetActorLocation());
        }

        UpdateAnimBPSpeed(1);
    }
    else if (bIsMovingOnPath && Attacking && PathToFollow.IsValidIndex(CurrentPathIndex))
    {
        FVector TargetPoint = PlaySubsystem->GridToPos(PathToFollow[CurrentPathIndex]);
        TargetPoint.Z = GetActorLocation().Z; 

        FVector Direction = (TargetPoint - GetActorLocation());
        float DistanceToTarget = Direction.Size2D();

        if (DistanceToTarget <= 20.f) 
        {
            CurrentPathIndex++;
            if (CurrentPathIndex >= PathToFollow.Num())
            {
                bIsMovingOnPath = false;
                UpdateAnimBPSpeed(0);
            }
        }
        else
        {
            AddMovementInput(Direction.GetSafeNormal(), 1.0f);
            UpdateAnimBPSpeed(1);

            if (PlaySubsystem)
            {
                PlaySubsystem->UpdateHeroLocation(this, LastGridKey, GetActorLocation());
            }
        }
    }
    else
    {
        SearchTimer -= DeltaTime;
        if (Attacking && SearchTimer <= 0.0f) {
            FindEnemiesInArea();
            SearchTimer = SearchInterval;
        }
        if (bEnemyDetected) Attack();

        if (!bIsMovingOnPath)
        {
            UpdateAnimBPSpeed(0);
        }
    }

    CurrentForwardInput = 0.0f;
    CurrentRightInput = 0.0f;
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACharacterBase::Attack_Melee);
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ACharacterBase::Attack_Melee()
{
    GetWorldTimerManager().SetTimer(TH_Attack_End, this, &ACharacterBase::Attack_End, 1.0f, false);
}

void ACharacterBase::Attack_End()
{
}

void ACharacterBase::UseSkill()
{
    if (AbilitySystemComponent && SkillAbilityClass)
    {
        AbilitySystemComponent->TryActivateAbilityByClass(SkillAbilityClass);
    }
}

void ACharacterBase::FindEnemiesInArea()
{
    
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        return;
    }

    const float RangeValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
        : DefaultRange;

    const float AttackPowerValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
        : DefaultAttackPower;

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

    FVector MyLocation = GetActorLocation();
    float RadiusSq = FMath::Square(RangeValue);

    if (!bIsOutsideWall) {
        FMassExecutionContext ExecContext(EntityManager, 0.0f);
        bool bFoundInChunk = false;

        TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq, AttackPowerValue, &bFoundInChunk](FMassExecutionContext& Context)
            {
                if (bFoundInChunk) return; 

                const int32 NumEntities = Context.GetNumEntities();
                auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
                auto Conditions = Context.GetMutableFragmentView<FMonsterConditionFragment>();

                for (int32 i = 0; i < NumEntities; ++i)
                {
                    FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

                    if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
                    {
                        bEnemyDetected = true;
                        Conditions[i].Damage += AttackPowerValue;
                        Attacking = false;

                        FVector Direction = (EnemyLoc - MyLocation).GetSafeNormal2D();
                        if (!Direction.IsNearlyZero())
                        {
                            SetActorRotation(Direction.Rotation());
                        }

                        bFoundInChunk = true; 
                        break;
                    }
                }
            });
    }
    else {
        int64 MyKey = PlaySubsystem->GetGridKey(MyLocation);
        int32 CenterX = (int32)(MyKey >> 32);
        int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

        bool bFoundInGrid = false;

        for (int32 x = -1; x <= 1; ++x)
        {
            if (bFoundInGrid) break;

            for (int32 y = -1; y <= 1; ++y)
            {
                int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);

                if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
                {
                    for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
                    {
                        if (FVector::DistSquared(MyLocation, MInfo.Location) <= RadiusSq)
                        {
                            if (!EntityManager.IsEntityValid(MInfo.MonsterHandle)) continue;

                            FMassEntityView EntityView(EntityManager, MInfo.MonsterHandle);
                            if (FMonsterConditionFragment* Condition = EntityView.GetFragmentDataPtr<FMonsterConditionFragment>())
                            {
                                Condition->Damage += AttackPowerValue;
                                bEnemyDetected = true;
                                Attacking = false;

                                FVector Direction = (MInfo.Location - MyLocation).GetSafeNormal2D();
                                if (!Direction.IsNearlyZero())
                                {
                                    SetActorRotation(Direction.Rotation());
                                }

                                bFoundInGrid = true;
                                break;
                            }
                        }
                    }
                }
                if (bFoundInGrid) break;
            }
        }
    }
}

void ACharacterBase::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected)
    {
        return;
    }

    if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
        {
            UAnimMontage* TargetMontage = GetDetectedMontage();
            if (TargetMontage)
            {
                AnimInstance->Montage_Play(TargetMontage);
            }
        }
    }
}

void ACharacterBase::PlayDetectedMontageSection(UAnimMontage* TargetMontage, bool& bHasPlayedPassive)
{
    if (!bEnemyDetected || !TargetMontage)
    {
        return;
    }

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh)
    {
        return;
    }

    UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance();
    if (!AnimInst)
    {
        return;
    }

    const FName SectionName = bHasPlayedPassive ? FName("Attack") : FName("Passive");
    if (!AnimInst->Montage_IsPlaying(TargetMontage))
    {
        AnimInst->Montage_Play(TargetMontage);
    }

    AnimInst->Montage_JumpToSection(SectionName, TargetMontage);
    bHasPlayedPassive = true;
}

void ACharacterBase::SetMoveAnimClassIfNeeded()
{
    if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        TSubclassOf<UAnimInstance> MoveAnimClass = GetMoveAnimClass();
        if (MoveAnimClass && SkeletalMesh->GetAnimClass() != MoveAnimClass)
        {
            SkeletalMesh->SetAnimInstanceClass(MoveAnimClass);
        }
    }
}

TSubclassOf<UMyAnimInstance> ACharacterBase::LoadDefaultAnimClass() const
{
    const FString ClassName = GetClass()->GetName();
    const TCHAR* AnimPath = nullptr;

    if (ClassName.Contains(TEXT("ArchAngelMinion")))
    {
        AnimPath = TEXT("/Game/Hero/ArchAngel/ArchAngelOrb_AnimBP.ArchAngelOrb_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("ArchAngel")))
    {
        AnimPath = TEXT("/Game/Hero/ArchAngel/ArchAngel_AnimBP.ArchAngel_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Archer")))
    {
        AnimPath = TEXT("/Game/Hero/Archer/Archer_AnimBP.Archer_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Berserker")))
    {
        AnimPath = TEXT("/Game/Hero/Berserker/Berserker_AnimBP.Berserker_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Cat")) || ClassName.Contains(TEXT("BlackCat")))
    {
        AnimPath = TEXT("/Game/Hero/BlackCat/BlackCat_AnimBP.BlackCat_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("DragonKnight")))
    {
        AnimPath = TEXT("/Game/Hero/DragonKnight/DragonKnight_AnimBP.DragonKnight_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Fighter")))
    {
        AnimPath = TEXT("/Game/Hero/Fighter/Fighter_AnimBP.Fighter_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Knight")))
    {
        AnimPath = TEXT("/Game/Hero/Knight/Knight_AnimBP.Knight_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Lancer")))
    {
        AnimPath = TEXT("/Game/Hero/Lancer/Lancer_AnimBP.Lancer_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Sage")))
    {
        AnimPath = TEXT("/Game/Hero/Sage/Sage_AnimBP.Sage_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Shielder")))
    {
        AnimPath = TEXT("/Game/Hero/Shielder/Shielder_AnimBPP.Shielder_AnimBPP_C");
    }
    else if (ClassName.Contains(TEXT("Thief")))
    {
        AnimPath = TEXT("/Game/Hero/Thief/Thief_AnimBP.Thief_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Valkyrie")))
    {
        AnimPath = TEXT("/Game/Hero/Valkyrie/Valkyrie_AnimBP.Valkyrie_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("WeaponMaster")))
    {
        AnimPath = TEXT("/Game/Hero/WeaponMaster/WeaponMaster_AnimBP.WeaponMaster_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Witch")))
    {
        AnimPath = TEXT("/Game/Hero/Witch/Witch_AnimBP.Witch_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Wizard")))
    {
        AnimPath = TEXT("/Game/Hero/Wizard/Wizard_AnimBP.Wizard_AnimBP_C");
    }
    else if (ClassName.Contains(TEXT("Wyvern")))
    {
        AnimPath = TEXT("/Game/Hero/Wyvern/Wyvern_AnimBP.Wyvern_AnimBP_C");
    }

    if (!AnimPath)
    {
        return nullptr;
    }

    UClass* LoadedClass = StaticLoadClass(UMyAnimInstance::StaticClass(), nullptr, AnimPath);
    return LoadedClass ? TSubclassOf<UMyAnimInstance>(LoadedClass) : nullptr;
}

void ACharacterBase::Attack()
{
    PlayDetectedMontageIfNeeded();
    bEnemyDetected = false;

    float AttackSpeedValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackSpeedAttribute())
        : DefaultAttackSpeed;

    float AttackInterval = 1.0f / FMath::Max(AttackSpeedValue, KINDA_SMALL_NUMBER);

    GetWorldTimerManager().SetTimer(AttackDelayTimerHandle, this, &ACharacterBase::ResetAttack, AttackInterval, false);
}

void ACharacterBase::ResetAttack()
{
    Attacking = true;
}

UAnimMontage* ACharacterBase::GetDetectedMontage() const
{
    return AnimMontage;
}

TSubclassOf<UAnimInstance> ACharacterBase::GetMoveAnimClass() const
{
    return AnimClass;
}

void ACharacterBase::TakeMonsterDamage(float DamageAmount, FVector AttackerLocation)
{
    bool bIsCheatActive = (CachedPlayerChar && CachedPlayerChar->bIsHeroInvincible);

    if (!bIsCheatActive)
    {
        CurrentHealth -= DamageAmount;
    }

    FVector HeroLoc = GetActorLocation();
    FVector Direction = (AttackerLocation - HeroLoc).GetSafeNormal2D();
    FRotator SpawnRotation = Direction.Rotation();

    AsyncTask(ENamedThreads::GameThread, [this, SpawnRotation]()
        {
            if (HitEffectTemplate)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectTemplate, GetActorLocation(), SpawnRotation);
            }
        });

    if (CurrentHealth <= 0.0f)
    {
        CurrentHealth = MaxHealth;

        FVector SpawnLocation(-1500.0f, 1500.0f, 110.0f);

        SetActorLocation(SpawnLocation);

        bEnemyDetected = false;
        Attacking = true;

       if( PlaySubsystem->GetCurrentRound() <= PlaySubsystem->GetMaxRound())
       {
           bIsOutsideWall = false;
       }

        bIsMovingOnPath = false;
        PathToFollow.Empty();

        if (PlaySubsystem)
        {
            PlaySubsystem->UpdateHeroLocation(this, LastGridKey, SpawnLocation);
        }
    }
}

void ACharacterBase::SetSelectedHero(bool bIsSelected)
{
    if (SelectionDecal)
    {
        SelectionDecal->SetHiddenInGame(!bIsSelected);
    }
}

void ACharacterBase::CommandMoveToLocation(FVector TargetLocation)
{
    if (!PlaySubsystem)
    {
        return;
    }

    FIntVector StartGrid = PlaySubsystem->PosToGrid(GetActorLocation());
    FIntVector TargetGrid = PlaySubsystem->PosToGrid(TargetLocation);
    PathToFollow = PlaySubsystem->FindPath(StartGrid, TargetGrid);

    PathToFollow = PlaySubsystem->SmoothPath(PathToFollow);

    if (PathToFollow.Num() > 0)
    {
        CurrentPathIndex = 0;
        bIsMovingOnPath = true;
    }
    else
    {
        bIsMovingOnPath = false;
        PathToFollow.Empty();
    }
}

void ACharacterBase::MoveForward(float val)
{
	CurrentForwardInput = val;
}

void ACharacterBase::MoveRight(float val)
{
	CurrentRightInput = val;
}

void ACharacterBase::UpdateAnimBPSpeed(int val)
{
    USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>();
    if (!CharacterMesh)
    {
        return;
    }

    UAnimInstance* CurrentInst = CharacterMesh->GetAnimInstance();
    if (!CurrentInst)
    {
        return;
    }

    if (CurrentInst->IsA(GetMoveAnimClass()))
    {
        if (UMyAnimInstance* MyInst = Cast<UMyAnimInstance>(CurrentInst))
        {
            MyInst->speed = (val != 0) ? 200.0f : 0.0f;
        }
    }
}

void ACharacterBase::SetIsOutside(bool bOutside)
{
    bIsOutsideWall = bOutside;
}

void ACharacterBase::GrantDefaultAbilities()
{
    if (!AbilitySystemComponent || !HasAuthority())
    {
        return;
    }

    if (SkillAbilityClass)
    {
        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SkillAbilityClass, 1, 1, this));
    }

}

void ACharacterBase::ApplyUpgradeStats(int32 NewLevel)
{
    DefaultAttackPower = DefaultAttackPower + (DefaultUpgradeAttack * NewLevel);


    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackPowerAttribute(), DefaultAttackPower);

    }
}