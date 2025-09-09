#include "Player/THTitlePlayerController.h"
#include "Kismet/GameplayStatics.h"
void ATHTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		return;
	}
}

void ATHTitlePlayerController::AssignPlayerUniqueId(FString InStr)
{
	
}

void ATHTitlePlayerController::JoinServer(const FString& InIPAddress)
{
	FName NextLevelName = FName(*InIPAddress);
	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
}

FString ATHTitlePlayerController::GetCustomId() const
{
	return FString();
}

void ATHTitlePlayerController::SetCustomId(const FString& CustomId)
{
}
