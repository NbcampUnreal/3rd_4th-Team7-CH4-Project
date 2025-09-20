#include "Item/Item_Ability/THItemGenericAbility.h"
#include "Item/Item_Ability/THItemAbilityBase.h"
#include "Item/Item_Data/THItemData.h"
#include "Item/Item_Data/THItemRowSource.h"
#include "Item/Item_Data/THItemDataManager.h"
#include "Item/Spawn/THSpawnObject.h"
#include "Item/Spawn/THSlowTrap.h"


#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter/THPlayerCharacter.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

UTHItemGenericAbility::UTHItemGenericAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; // 서버에서만 처리되도록 근데 아래에서 한 번 더 서버 처리 로직을 넣어서 예방 
}

void UTHItemGenericAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!HasAuthority(&ActivationInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const UObject* SourceObj = GetCurrentSourceObject();
    const FName RowName = ResolveRowNameFromSourceObject(SourceObj);
    const FTHItemData* Row = FindRow(RowName, ActorInfo);
    if (!Row)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (Row->Targeting == EItemTargetingPolicy::WorldSpawnAtFeet && Row->SpawnObjectClass) // 오브젝트 스폰은 따로 처리 
    {
        SpawnAtFeet(ActorInfo, Row->SpawnObjectClass, *Row);
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    // 그 외 타겟형은 GE 적용
    TArray<UAbilitySystemComponent*> Targets = ResolveTargets(*Row, ActorInfo);
    for (UAbilitySystemComponent* TargetASC : Targets)
    {
        if (!TargetASC) continue;
        if (TargetBlocked(TargetASC, Row->BlockedByTags)) continue;
        if (!Row->CleanseTags.IsEmpty()) Cleanse(TargetASC, Row->CleanseTags);
        ApplyUniqueEffect(TargetASC, *Row);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UTHItemGenericAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEnd, bool bWasCancelled)
{
    if (HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEnd, bWasCancelled);
}

FName UTHItemGenericAbility::ResolveRowNameFromSourceObject(const UObject* SourceObj)
{
    if (!SourceObj) return NAME_None;

    if (const UTHItemRowSource* RowSrc = Cast<UTHItemRowSource>(SourceObj)) // 안전하게 받아 온 Row 받아냄 
    {
        return RowSrc->RowName;
    }
    return SourceObj->GetFName();
}

const FTHItemData* UTHItemGenericAbility::FindRow(FName RowName, const FGameplayAbilityActorInfo* ActorInfo)
{
    if (ItemDataTable && RowName != NAME_None)
    {
        return ItemDataTable->FindRow<FTHItemData>(RowName, TEXT("UTHItemGenericAbility"));
    }

    // Ability에 테이블이 비어있으면 매니저에게서 가져온다
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (ATHItemDataManager* DM = ATHItemDataManager::Get(ActorInfo->AvatarActor->GetWorld()))
        {
            if (DM->ItemDataTable)
            {
                ItemDataTable = DM->ItemDataTable; // 캐시
                return ItemDataTable->FindRow<FTHItemData>(RowName, TEXT("UTHItemGenericAbility(DM)"));
            }
        }
    }

    return nullptr;
}

// 기존 사용하셨던 아이템 스폰 로직을 제너릭 어빌리티로 가져와서 함수 하나로 제작 
void UTHItemGenericAbility::SpawnAtFeet(const FGameplayAbilityActorInfo* Info, TSubclassOf<AActor> SpawnClass, const FTHItemData& Row)
{ 
    if (!Info || !SpawnClass) return;
    AActor* Avatar = Info->AvatarActor.Get();
    if (!Avatar || !Avatar->GetWorld()) return;

    const FVector Start = Avatar->GetActorLocation();
    const FVector End = Start - FVector(0, 0, 500.f);

    FHitResult Hit;
    FCollisionQueryParams P; P.bTraceComplex = false; P.AddIgnoredActor(Avatar);

    FVector SpawnLoc = Start;
    if (Avatar->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, P))
        SpawnLoc = Hit.ImpactPoint;

    FActorSpawnParameters S;
    S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* Spawned = Avatar->GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnLoc, Avatar->GetActorRotation(), S);

    // 설치 ! 
    if (auto* Trap = Cast<ATHSpawnObject>(Spawned))
    {
        Trap->SetPlacerAndOwner(Avatar);
        // 필요하면 Row.DurationSec 등 넘기려면 Trap에 세터 하나 더 열어도 됨
        if (auto* SlowTrap = Cast<ATHSlowTrap>(Trap))
        {
            SlowTrap->InitFromItemRow(Row.GameplayEffectClass, Row.WalkDelta, Row.SprintDelta, Row.DurationSec);
        }
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// [헬퍼들을 static 으로 두는 이유]
// - 객체 상태/this 에 전혀 의존하지 않는 "순수 유틸" 함수들이라서 / 의도를 명확하게 하기 위해 
// 그냥 이렇게 해두면 캡슐화로 최적화 (인라인) 할 수 있게끔 하는 것 뿐이긴 함 -> 컴파일러가 판단 
// - 파일 내부 연결(Internal linkage)로 제한하여, 이 .cpp 밖으로 심볼이 노출되지 않게 함 
//   → 동일 이름 유틸이 다른 TU(.cpp) 에 있어도 충돌(ODR) 없음, 네임스페이스 오염 방지

// - 여러 .cpp 에서 재사용해야 한다면:
//   1) 공용 헤더에 `inline` 함수로 옮김
//   2) 클래스의 `private static` 멤버로 제공
//   3) 무명 네임스페이스 `namespace {}` 를 써서 내부 연결

// 아래 헬퍼들은 이 TU 안에서만 쓰이는 순수 함수들이므로 static
// ─────────────────────────────────────────────────────────────────────────────
static void Resolve_Self(TArray<UAbilitySystemComponent*>& Out, const FTHItemData& /*Row*/, const FGameplayAbilityActorInfo* Info)
{
    if (UAbilitySystemComponent* ASC = (Info && Info->AbilitySystemComponent.IsValid()) ? Info->AbilitySystemComponent.Get() : nullptr)
        Out.Add(ASC);
}

static void Gather_OtherPlayers(const AActor* Self, TArray<ATHPlayerCharacter*>& OutPlayers)
{
    if (!Self) return;
    UWorld* W = Self->GetWorld();
    if (!W) return;

    TArray<AActor*> Found; Found.Reserve(16);
    UGameplayStatics::GetAllActorsOfClass(W, ATHPlayerCharacter::StaticClass(), Found);
    for (AActor* A : Found)
        if (A && A != Self)
            if (ATHPlayerCharacter* P = Cast<ATHPlayerCharacter>(A))
                OutPlayers.Add(P);
}

static void Resolve_AllOthers(TArray<UAbilitySystemComponent*>& Out, const FTHItemData& /*Row*/, const FGameplayAbilityActorInfo* Info)
{
    const AActor* Self = Info && Info->AvatarActor.IsValid() ? Info->AvatarActor.Get() : nullptr;
    TArray<ATHPlayerCharacter*> Others; Others.Reserve(8);
    Gather_OtherPlayers(Self, Others);
    for (ATHPlayerCharacter* P : Others)
        if (UAbilitySystemComponent* ASC = P->GetAbilitySystemComponent())
            Out.Add(ASC);
}

static void Resolve_Sphere(TArray<UAbilitySystemComponent*>& Out, const FTHItemData& Row, const FGameplayAbilityActorInfo* Info)
{
    const AActor* Self = Info && Info->AvatarActor.IsValid() ? Info->AvatarActor.Get() : nullptr;
    if (!Self || Row.Radius <= 0.f) return;

    const FVector SLoc = Self->GetActorLocation();
    const float R2 = FMath::Square(Row.Radius);

    TArray<ATHPlayerCharacter*> Others; Others.Reserve(8);
    Gather_OtherPlayers(Self, Others);

    for (ATHPlayerCharacter* P : Others)
        if (P && FVector::DistSquared(P->GetActorLocation(), SLoc) <= R2)
            if (UAbilitySystemComponent* ASC = P->GetAbilitySystemComponent())
                Out.Add(ASC);
}

static void Resolve_Nearest(TArray<UAbilitySystemComponent*>& Out, const FTHItemData& /*Row*/, const FGameplayAbilityActorInfo* Info)
{
    const AActor* Self = Info && Info->AvatarActor.IsValid() ? Info->AvatarActor.Get() : nullptr;
    if (!Self) return;

    const FVector SLoc = Self->GetActorLocation();
    float Best = TNumericLimits<float>::Max();
    UAbilitySystemComponent* BestASC = nullptr;

    TArray<ATHPlayerCharacter*> Others; Others.Reserve(8);
    Gather_OtherPlayers(Self, Others);

    for (ATHPlayerCharacter* P : Others)
    {
        const float D2 = FVector::DistSquared(P->GetActorLocation(), SLoc);
        if (D2 < Best)
        {
            Best = D2;
            BestASC = P->GetAbilitySystemComponent();
        }
    }
    if (BestASC) Out.Add(BestASC);
}

TArray<UAbilitySystemComponent*> UTHItemGenericAbility::ResolveTargets(const FTHItemData& Row, const FGameplayAbilityActorInfo* Info)
{
    TArray<UAbilitySystemComponent*> Out;
    if (!Info) return Out;

    AActor* Self = Info->AvatarActor.Get();
    switch (Row.Targeting)
    {
    case EItemTargetingPolicy::Self:               Resolve_Self(Out, Row, Info); break;
    case EItemTargetingPolicy::AllOtherPlayers:    Resolve_AllOthers(Out, Row, Info); break;
    case EItemTargetingPolicy::SphereAroundSelf:   Resolve_Sphere(Out, Row, Info); break;
    case EItemTargetingPolicy::NearestOtherPlayer: Resolve_Nearest(Out, Row, Info); break;
    default: break;
    }
    return Out;
}