#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

#include "THPlayerState.generated.h"

class UGameplayEffect;
class UTHAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNicknameUpdated, const FString&, NewNickname);

UCLASS()
class TREASUREHUNTER_API ATHPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATHPlayerState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

#pragma region GAS
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TSubclassOf<UGameplayEffect> GetStaminaRegenEffectClass() const { return StaminaRegenEffect; }
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	UTHAttributeSet* GetAttributeSet() const;

	void InitializeAbilityActorInfo(APawn* NewPawn);

	void GiveStartupAbilities();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UTHAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;

private:
	bool bStartupAbilitiesGiven;
#pragma endregion

#pragma region Matchmaking
public:
	void Server_SetSlotTag(int32 SlotIdx); // 0=First(Bunny), 1=Second(Mouse)

	void Server_SetReady(bool bReady);

	bool HasReadyTag() const;
#pragma endregion

#pragma region Nickname
public:
	UFUNCTION()
	void OnRep_Nickname();

	UPROPERTY(BlueprintAssignable)
	FOnNicknameUpdated OnNicknameUpdated;

public:
	UPROPERTY(ReplicatedUsing=OnRep_Nickname, BlueprintReadOnly)
	FString Nickname;
#pragma endregion

#pragma region PlayerData
public:
	UPROPERTY()
	FString PlayerUniqueId;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	void OrganizeAbilitySystemComponent();
#pragma endregion
};
