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
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; // ���������� ó���ǵ��� �ٵ� �Ʒ����� �� �� �� ���� ó�� ������ �־ ���� 
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

    if (Row->Targeting == EItemTargetingPolicy::WorldSpawnAtFeet && Row->SpawnObjectClass) // ������Ʈ ������ ���� ó�� 
    {
        SpawnAtFeet(ActorInfo, Row->SpawnObjectClass, *Row);
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    // �� �� Ÿ������ GE ����
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

    if (const UTHItemRowSource* RowSrc = Cast<UTHItemRowSource>(SourceObj)) // �����ϰ� �޾� �� Row �޾Ƴ� 
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

    // Ability�� ���̺��� ��������� �Ŵ������Լ� �����´�
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (ATHItemDataManager* DM = ATHItemDataManager::Get(ActorInfo->AvatarActor->GetWorld()))
        {
            if (DM->ItemDataTable)
            {
                ItemDataTable = DM->ItemDataTable; // ĳ��
                return ItemDataTable->FindRow<FTHItemData>(RowName, TEXT("UTHItemGenericAbility(DM)"));
            }
        }
    }

    return nullptr;
}

// ���� ����ϼ̴� ������ ���� ������ ���ʸ� �����Ƽ�� �����ͼ� �Լ� �ϳ��� ���� 
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

    // ��ġ ! 
    if (auto* Trap = Cast<ATHSpawnObject>(Spawned))
    {
        Trap->SetPlacerAndOwner(Avatar);
        // �ʿ��ϸ� Row.DurationSec �� �ѱ���� Trap�� ���� �ϳ� �� ��� ��
        if (auto* SlowTrap = Cast<ATHSlowTrap>(Trap))
        {
            SlowTrap->InitFromItemRow(Row.GameplayEffectClass, Row.WalkDelta, Row.SprintDelta, Row.DurationSec);
        }
    }
}


// ����������������������������������������������������������������������������������������������������������������������������������������������������������
// [���۵��� static ���� �δ� ����]
// - ��ü ����/this �� ���� �������� �ʴ� "���� ��ƿ" �Լ����̶� / �ǵ��� ��Ȯ�ϰ� �ϱ� ���� 
// �׳� �̷��� �صθ� ĸ��ȭ�� ����ȭ (�ζ���) �� �� �ְԲ� �ϴ� �� ���̱� �� -> �����Ϸ��� �Ǵ� 
// - ���� ���� ����(Internal linkage)�� �����Ͽ�, �� .cpp ������ �ɺ��� ������� �ʰ� �� 
//   �� ���� �̸� ��ƿ�� �ٸ� TU(.cpp) �� �־ �浹(ODR) ����, ���ӽ����̽� ���� ����

// - ���� .cpp ���� �����ؾ� �Ѵٸ�:
//   1) ���� ����� `inline` �Լ��� �ű�
//   2) Ŭ������ `private static` ����� ����
//   3) ���� ���ӽ����̽� `namespace {}` �� �Ἥ ���� ����

// �Ʒ� ���۵��� �� TU �ȿ����� ���̴� ���� �Լ����̹Ƿ� static
// ����������������������������������������������������������������������������������������������������������������������������������������������������������
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