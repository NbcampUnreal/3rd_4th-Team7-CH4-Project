#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EGameFlow : uint8
{
	Wait UMETA(DisplayName = "Wait"),
	Match UMETA(DisplayName = "Match"),
	Load UMETA(DisplayName = "Load"),
	Play UMETA(DisplayName = "Play"),
	Finish UMETA(DisplayName = "Finish"),
	Result UMETA(DisplayName = "Result")
};

class TREASUREHUNTER_API THGameModeEnum
{

};
