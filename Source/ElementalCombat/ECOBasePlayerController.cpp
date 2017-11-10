// Property of Avrlon, all rights reserved.

#include "ElementalCombat.h"
#include "ECOBasePlayerController.h"

AECOBasePlayerController::AECOBasePlayerController(const FObjectInitializer & ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsInLobby = false;
}

void AECOBasePlayerController::PreClientTravel(const FString & PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	UE_LOG(LogTemp, Warning, TEXT("PreClientTravel was called."))
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("PreClientTravel was called."));

	/// Get the possessed pawn
	APawn* ECOPawn = AController::GetPawn();

	/// Only if we have a possessed pawn do we attemp to destroy it before travelling
	if (bIsInLobby)
	{
		if (ECOPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("Possessed pawn is not nullptr"))
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Possessed pawn is not nullptr"));
			ECOPawn->Destroy();
		}
		bIsInLobby = false;
	}
}
