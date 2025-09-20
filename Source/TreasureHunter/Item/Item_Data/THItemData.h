#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "THItemData.generated.h"

class UTexture2D;
class UUserWidget;
class UGameplayEffect;
class AActor;

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable    UMETA(DisplayName = "Consumable"),
    Equipment     UMETA(DisplayName = "Equipment"),
    Material      UMETA(DisplayName = "Material")
};

UENUM(BlueprintType)
enum class EItemUIIndicator : uint8
{
    None,
    InventoryProgressBarBuff,
    TopRightInventoryBuffIcon,
    FullScreenOverlay
};

UENUM(BlueprintType)
enum class EItemUseKind : uint8
{
    Instant,
    DurationBuff,
    Deployable,
    TargetDebuff,
};

UENUM(BlueprintType) // -> 타겟팅 정책을 포함해서 아이템 확장성 고려
enum class EItemTargetingPolicy : uint8
{
    Self,
    NearestOtherPlayer, // 어떤 플레이어를 타겟팅할 건지 결정 
    AllOtherPlayers,
    SphereAroundSelf,
    WorldSpawnAtFeet // 아이템 스폰 시 사용 
};

USTRUCT(BlueprintType)
struct FTHItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    //---- 아이템 아이콘과 이름 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Display")
    FText ItemName = FText::GetEmpty();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Display")
    TSoftObjectPtr<UTexture2D> ItemIcon = nullptr;

    // 필드 드랍 시 스폰할 아이템 클래스 (ItemBox/DropItem이 사용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
    TSubclassOf<class ATHBaseItem> BaseItemClass = nullptr;

    //---- UI 사용 방식 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Use")
    EItemUIIndicator UiIndicator = EItemUIIndicator::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Use")
    EItemUseKind UseKind = EItemUseKind::Instant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Use", meta = (ClampMin = "0.0"))
    float DurationSec = 0.f;

    //---- GE ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    TSubclassOf<UGameplayEffect> GameplayEffectClass = nullptr;

    //---- 활성 태그/ 동일 태그 효과 제거 ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    FGameplayTag GrantedActiveTag;

    //---- 보유하면 적용 차단 ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    FGameplayTagContainer BlockedByTags;

    //---- 사용 시 제거할 태그들 ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    FGameplayTagContainer CleanseTags;

    //---- SetByCaller 수치들 ----
    /** 수치 파라미터(SetByCaller) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Params")
    float WalkDelta = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Params")
    float SprintDelta = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Params")
    float JumpDelta = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Params")
    float StaminaDelta = 0.f; // Instant 회복용 공용 GE가 따로 있다면 0으로 두면 됨

    //---- 타겟팅 정책용 Radius가 현재 프로젝트에선 필요 없지만 참고용으로 생성 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Targeting")
    EItemTargetingPolicy Targeting = EItemTargetingPolicy::Self;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Targeting", meta = (ClampMin = "0.0"))
    float Radius = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Spawn")
    TSubclassOf<AActor> SpawnObjectClass = nullptr;

    //---- 이전에 있었던 드롭 타입과 가중치 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Drop")
    EItemType ItemDropType = EItemType::Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Drop")
    int32 DropWeight = 0;

    // 오버레이(맞은 쪽) UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TSubclassOf<class UUserWidget> VictimOverlayWidgetClass = nullptr;
};
