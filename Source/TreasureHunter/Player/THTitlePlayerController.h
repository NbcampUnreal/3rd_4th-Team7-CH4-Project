#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "THTitlePlayerController.generated.h"

class UTHWidgetManager;

UCLASS()
class TREASUREHUNTER_API ATHTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:

	TObjectPtr<UTHWidgetManager> WidgetManager;

	FString CustomUniqueId;

public:
	virtual void BeginPlay() override;

	//Join
	void JoinServer(const FString& InIPAddress);

	//CustomId
	void AssignPlayerUniqueId(FString InStr);

	void SetCustomId(const FString& CustomId);

	FString GetCustomId() const;
};