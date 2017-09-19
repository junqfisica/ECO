// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SteamGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ELEMENTALCOMBAT_API USteamGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	// Session properties used in creation and results
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
public:
	USteamGameInstance(const FObjectInitializer& ObjectInitializer);
	
};
