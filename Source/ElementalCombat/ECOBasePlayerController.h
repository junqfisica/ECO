// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ECOBasePlayerController.generated.h"

/**
 *  AECOBasePlayerController - APlayerController class basis for every BP player controller
 */
UCLASS()
class ELEMENTALCOMBAT_API AECOBasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/// Members
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	bool bIsInLobby;

	/// Methods
	AECOBasePlayerController(const FObjectInitializer & ObjectInitializer); /// constructor

	/// Travelling methods
	/**
	* Called when the local player is about to travel to a new map or IP address.
	*/
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel);
	
	
};
