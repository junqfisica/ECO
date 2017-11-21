// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainMenuGM.generated.h"

/**
 * 
 */
UCLASS()
class ELEMENTALCOMBAT_API AMainMenuGM : public AGameMode
{
	GENERATED_BODY()

	/// Methods
	/**
	*   Function overrides Logout function from AGameMode - handles any 
	*   operations when a player leaves the match
	*
	*   @param		Exitting		The controller leaving the match 
	*/
	virtual void Logout(AController* Exiting) override;

protected:
	/// Members
	/** Boolean is set to true only for states that are acceptable to change from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	bool bCanChangeFromThisState;

	/// Methods
	/**
	*   Function wrapper for SetMatchState - used in BPs
	*
	*   @param		NewState		The new state the match should transition to
	*/
	UFUNCTION(BlueprintCallable, Category = "Game")
	void BP_SetMatchState(FName NewState);

};
