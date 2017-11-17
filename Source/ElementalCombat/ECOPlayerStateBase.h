// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ECOPlayerStateBase.generated.h"

/**
 * 
 */
UCLASS()
class ELEMENTALCOMBAT_API AECOPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "NetWork")
	void SetSessionName(FName SessionName);
	
};
