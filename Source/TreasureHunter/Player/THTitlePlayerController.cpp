#include "Player/THTitlePlayerController.h"
#include "Game/THWidgetManager.h"

#include "Kismet/GameplayStatics.h"

void ATHTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		return;
	}

	WidgetManager = NewObject<UTHWidgetManager>(this);
}

void ATHTitlePlayerController::AssignPlayerUniqueId(FString InStr)
{
	
}

void ATHTitlePlayerController::JoinServer(const FString& InIPAddress)
{
	//IP
	FName NextLevelName = FName(*InIPAddress);
	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
}

void ATHTitlePlayerController::SetCustomId(const FString& CustomId)
{
}

FString ATHTitlePlayerController::GetCustomId() const
{
	return FString();
}
