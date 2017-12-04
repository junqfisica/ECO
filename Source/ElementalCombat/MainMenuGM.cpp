// Property of Avrlon, all rights reserved.

#include "MainMenuGM.h"

void AMainMenuGM::Logout(AController * Exiting)
{
	if (Exiting)
	{
		/// In a listen server, we do nothing besides calling Super
		/*if (!Exiting->IsLocalPlayerController())
		{
			/// Return the match state machine to the beginning of the match
			SetMatchState(MatchState::WaitingToStart);
		}*/

		/// Call Super to handle the rest of the logout procedures
		Super::Logout(Exiting);
	}
}

void AMainMenuGM::BP_SetMatchState(FName NewState)
{
	/// This boolean protects the state machine from changes to leaving or entering map
	bool bIsValidState = NewState != MatchState::EnteringMap && NewState != MatchState::LeavingMap;

	/// Only when we know it is safe to change the state do e change to the new one
	if (bIsValidState && bCanChangeFromThisState)
	{
		SetMatchState(NewState);
	}
}

