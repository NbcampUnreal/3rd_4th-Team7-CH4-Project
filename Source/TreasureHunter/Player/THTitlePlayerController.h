#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "THTitlePlayerController.generated.h"


UCLASS()
class TREASUREHUNTER_API ATHTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	FString CustomUniqueId;

public:
	virtual void BeginPlay() override;

	void AssignPlayerUniqueId(FString InStr);

	void JoinServer(const FString& InIPAddress);

	FString GetCustomId() const;

	void SetCustomId(const FString& CustomId);
};
