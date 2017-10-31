// Property of Avrlon, all rights reserved.

#include "ElementalCombat.h"
#include "SteamGameInstance.h"

USteamGameInstance::USteamGameInstance(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	ArenaMapName = "";
	MainMenuMapName = "";
	GameSessionName = "";
	bIsQuitting = false;
	bIsMatchMaking = false;
	WhoCalled = "";
		
	/// Initialization of the SessionSetting/Search here to have available at the very begining
	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

    /// Binding the session callbacks to our methods
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnCreateSessionComplete);
	OnUpdateSessionCompleteDelegate = FOnUpdateSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnUpdateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnStartOnlineGameComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnFindSessionsComplete);
	OnCancelFindSessionsCompleteDelegate = FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnCancelFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnJoinSessionComplete);
	OnRegisterPlayersCompleteDelegate = FOnRegisterPlayersCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnRegisterPlayersComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnDestroySessionComplete);
}

/// Accessor methods for FCustomBPSessionResult struct
FString USteamGameInstance::GetSessionName(FCustomBPSessionResult Result)
{
	FString SessionName = "";

	/// Have to pass the FString as an argument for it to be set the value
	Result.SessionResultInternal.Session.SessionSettings.Settings.Find(FName("SessionName"))->Data.GetValue(SessionName);

	return SessionName;
}

/// Helper methods
void USteamGameInstance::PrintSessionSearch(const FOnlineSessionSearch & Search)
{
	/// Each of the properties need to be printed directly... as I don't know how to do it differently
	UE_LOG(LogTemp, Warning, TEXT("bIsLanQuery = %s"), Search.bIsLanQuery ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("MaxSearchResults = %d"), Search.MaxSearchResults)

	/// Prints each of the key/value pairs in Search.QuerySettings
	for (FSearchParams::TConstIterator It(Search.QuerySettings.SearchParams); It; ++It) 
	{
		UE_LOG(LogTemp, Warning, TEXT("%s => %s"),*It.Key().ToString(),*It.Value().ToString())
	}
}

void USteamGameInstance::PrintSessionSettings(const FOnlineSessionSettings & Settings)
{
	/// Each of the properties need to be printed directly... as I don't know how to do it differently
	UE_LOG(LogTemp, Warning, TEXT("bAllowInvites = %s"), Settings.bAllowInvites ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bAllowJoinInProgress = %s"), Settings.bAllowJoinInProgress ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bAllowJoinViaPresence = %s"), Settings.bAllowJoinViaPresence ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bAllowJoinViaPresenceFriendsOnly = %s"), Settings.bAllowJoinViaPresenceFriendsOnly ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bAntiCheatProtected = %s"), Settings.bAntiCheatProtected ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bIsDedicated = %s"), Settings.bIsDedicated ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bIsLANMatch = %s"), Settings.bIsLANMatch ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bShouldAdvertise = %s"), Settings.bShouldAdvertise ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("BuildUniqueId = %d"), Settings.BuildUniqueId)
	UE_LOG(LogTemp, Warning, TEXT("bUsesPresence = %s"), Settings.bUsesPresence ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("bUsesStats = %s"), Settings.bUsesStats ? TEXT("True") : TEXT("false"))
	UE_LOG(LogTemp, Warning, TEXT("NumPrivateConnections = %d"), Settings.NumPrivateConnections)
	UE_LOG(LogTemp, Warning, TEXT("NumPublicConnections = %d"), Settings.NumPublicConnections)

	/// Prints each of the key/value pairs in Settings.Settings
	for (FSessionSettings::TConstIterator It(Settings.Settings); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s => %s"), *It.Key().ToString(), *It.Value().ToString())
	}
}

IOnlineSessionPtr USteamGameInstance::GetSessionInterface()
{
	/// Get the Online Subsystem set in DefaultEngine.ini
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		return OnlineSub->GetSessionInterface();
	}
	else
		return IOnlineSessionPtr();
}

ULocalPlayer * const USteamGameInstance::GetLocalPlayer()
{
	return GetFirstGamePlayer();
}

FString USteamGameInstance::BoolToFString(bool bBoolToConvert)
{
	return bBoolToConvert ? FString("TRUE") : FString("FALSE");
}

FString USteamGameInstance::ToString(const EOnJoinSessionCompleteResult::Type Value)
{
		switch (Value)
		{
		case EOnJoinSessionCompleteResult::Success:
			return TEXT("Success");
		case EOnJoinSessionCompleteResult::SessionIsFull:
			return TEXT("SessionIsFull");
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			return TEXT("SessionDoesNotExist");
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			return TEXT("CouldNotretrieveAddress");
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			return TEXT("AlreadyInSession");
		case EOnJoinSessionCompleteResult::UnknownError:
			; // Intentional fall-through
		}

		return TEXT("UnknownError");
}

const TArray<FCustomBPSessionResult> USteamGameInstance::PackageSessionResults(TArray<FOnlineSessionSearchResult> Results)
{
	TArray<FCustomBPSessionResult> TempList;

	/// Adding each result to the end of the list makes sure that the closest result is the first
	for (TArray<FOnlineSessionSearchResult>::TIterator It(Results); It; ++It)
	{
		FCustomBPSessionResult TempResult = { *It };
		TempList.Add(TempResult);
	}

	return TempList;
}

/// Session methods
bool USteamGameInstance::CreateSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	/// I am calling destroy for some reason
	WhoCalled = "CreateSessionInternal";

	/// Get the Session Interface to call CreateSession on it
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && UserId.IsValid())
	{
		/*
		Fill in all the Session Settings that we want to use.
		There are more with SessionSettings.Set(...);
		For example the Map or the GameMode/Type.
		*/
		SessionSettings->bIsLANMatch = bIsLAN;
		SessionSettings->bUsesPresence = bIsPresence;
		SessionSettings->NumPublicConnections = MaxNumPlayers;
		SessionSettings->NumPrivateConnections = 0;
		SessionSettings->bAllowInvites = true;
		SessionSettings->bAllowJoinInProgress = true;
		SessionSettings->bShouldAdvertise = true;
		SessionSettings->bAllowJoinViaPresence = true;
		SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

		SessionSettings->Set(SETTING_MAPNAME, ArenaMapName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("SessionName"), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

		PrintSessionSettings(*SessionSettings);

		/// Set the delegate to the Handle of the SessionInterface
		OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		/// Our delegate should get called when this is complete (doesn't need to be successful!)
		return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No OnlineSubsytem found!"))
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));

		return false;
	}
}

bool USteamGameInstance::FindSessionsInternal(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	/// I am calling destroy for some reason
	WhoCalled = "FindSessionsInternal";

	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && UserId.IsValid())
	{
		/*
		Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
		*/
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		
		SessionSearch->bIsLanQuery = bIsLAN;
		SessionSearch->MaxSearchResults = 20;
		SessionSearch->PingBucketSize = 70;

		/// We only want to set this Query Setting if "bIsPresence" is true
		if (bIsPresence)
		{
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
		}

		PrintSessionSearch(*SessionSearch);

		TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

		/// Set the Delegate to the Delegate Handle of the FindSession function
		OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

		/// Finally call the SessionInterface function. The Delegate gets called once this is finished
		return Sessions->FindSessions(*UserId, SearchSettingsRef);
	}
	else
	{
		/// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
		return false;
	}
}

bool USteamGameInstance::CancelFindSessionsInternal()
{
	/// I am calling destroy for some reason
	WhoCalled = "CancelFindSessionsInternal";

	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Set the Delegate to the Delegate Handle of the CancelFindSessions function
		OnCancelFindSessionsCompleteDelegateHandle = Sessions->AddOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionsCompleteDelegate);

		/// Call the OnlineSessionInterface CancelFindSessions function. Async call will trigger the delegate when task is done
		return Sessions->CancelFindSessions();
	}
	else
	{
		/// Fail immediately if anything goes wrong
		OnCancelFindSessionsComplete(false);
		return false;
	}
}

bool USteamGameInstance::JoinSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult & SearchResult)
{
	/// I am calling destroy for some reason
	WhoCalled = "JoinSessionInternal";

	/// Return bool
	bool bSuccessful = false;

	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && UserId.IsValid())
	{
		/// Set the Handle again
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

		/// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
		/// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
		bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
	}

	return bSuccessful;
}

bool USteamGameInstance::RegisterPlayerInternal(FName SessionName, TSharedPtr<const FUniqueNetId> PlayerId, bool bWasInvited)
{
	/// I am calling destroy for some reason
	//WhoCalled = "RegisterPlayerInternal";

	/// Return bool
	bool bSuccessful = false;

	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && PlayerId.IsValid())
	{
		/// Set the Handle for registering all players since there is no OnRegisterPlayerComplete delegate
		OnRegisterPlayersCompleteDelegateHandle = Sessions->AddOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegate);

		/// Here the call has to be to RegisterPlayer
		bSuccessful = Sessions->RegisterPlayer(SessionName, *PlayerId, bWasInvited);
	}

	return bSuccessful;
}

/// Callbacks
void USteamGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnCreateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

    /// Get the Session Interface to call the StartSession function
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the SessionComplete delegate handle, since we finished this call
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

		if (bWasSuccessful)
		{
			/// Add a new session name for later destruction
			SessionNames.AddUnique(SessionName);

			if (bIsMatchMaking)
			{
				/// Register this player in this session
				//RegisterPlayerInternal(SessionName, GetLocalPlayer()->GetPreferredUniqueNetId(), false);
				/// When matchmaking let BP code be run to decide when to start the match
				OnCreateSessionSuccessfulDelegate.Broadcast();

			}
			else
			{
				/// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				/// Our StartSessionComplete delegate should get called after this
				Sessions->StartSession(SessionName);
			}
		}
	}
}

void USteamGameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnUpdateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnUpdateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue));

	/// Get the Session Interface to call the StartSession function
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the SessionComplete delegate handle, since we finished this call
		Sessions->ClearOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegateHandle);

		/// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
		/// This is something the Blueprint Node "Join Session" does automatically!
		APlayerController * const PlayerController = GetFirstLocalPlayerController();

		/// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
		/// String for us by giving him the SessionName and an empty String. We want to do this, because
		/// Every OnlineSubsystem uses different TravelURLs
		FString TravelURL;

		/// When successfully updating the session, call the OnJoinSessionSuccessful delegate
		if(PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL) && bWasSuccessful)
		{
			/// When matchmaking, allow BP code to deal with joining and traveling
			OnJoinSessionSuccessfulDelegate.Broadcast(TravelURL, ETravelType::TRAVEL_Absolute);
		}
	}
}

void USteamGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnStartSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnStartSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue));

	/// Get the Session Interface to clear the Delegate
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		/// Clear the delegate, since we are done with this call
		Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);

		/// If the start was successful, we can open MapName if we want. Make sure to use "listen" as a parameter!
		if (bWasSuccessful)
		{
			UGameplayStatics::OpenLevel(GetWorld(), ArenaMapName, true, "listen");
		}
	}
}

void USteamGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnFindtSessionsComplete, %s"), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnFindSessionsComplete bSuccess: %s"), *BoolValue));

	/// Get SessionInterface of the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions.IsValid())
	{
		/// Clear the Delegate handle, since we finished this call
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

		UE_LOG(LogTemp, Warning, TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num())
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

		/// If we have found at least 1 session, call the successful delegate, otherwise call the not successful delegate
		if (SessionSearch->SearchResults.Num() > 0)
		{
    		OnFindSessionsSuccessfulDelegate.Broadcast(PackageSessionResults(SessionSearch->SearchResults));
		}
		else
		{
			OnFindSessionsNotSuccessfulDelegate.Broadcast();
		}

	}
}

void USteamGameInstance::OnCancelFindSessionsComplete(bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnCancelFindSessionsComplete, %s"), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnCancelFindSessionsComplete, %s"), *BoolValue));

	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the Delegate again
		Sessions->ClearOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionsCompleteDelegateHandle);

		/// Only when find is unsuccessful, do we deal with the created sessions
		if (Sessions->GetNamedSession(GameSessionName))
		{
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
		}

	}
}

void USteamGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString test = ToString(Result);
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s, %s"), *SessionName.ToString(), *test)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnJoinSessionComplete %s, %s"), *SessionName.ToString(), *test));

	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the Delegate again
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

		/// Register the joining player here
		//RegisterPlayerInternal(SessionName, GetLocalPlayer()->GetPreferredUniqueNetId(), false);

		/// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
		/// This is something the Blueprint Node "Join Session" does automatically!
		APlayerController * const PlayerController = GetFirstLocalPlayerController();

		/// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
		/// String for us by giving him the SessionName and an empty String. We want to do this, because
		/// Every OnlineSubsystem uses different TravelURLs
		FString TravelURL;

		/// Add a new session name for later destruction
		SessionNames.AddUnique(SessionName);

		if (bIsMatchMaking)
		{
			/// When matchmaking, allow BP code to deal with joining and traveling
			//OnJoinSessionSuccessfulDelegate.Broadcast(TravelURL, ETravelType::TRAVEL_Absolute);

			/// Add a new setting telling the server we connected to it
			//TSharedPtr<FOnlineSessionSettings> NewSettings = MakeShareable(&Sessions->GetNamedSession(SessionName)->SessionSettings);
			//NewSettings->Set(FName("ClientConnected"), true, EOnlineDataAdvertisementType::ViaOnlineService);

			/// When matchmaking, update the session to let the server now we are connected to this session
			//OnUpdateSessionCompleteDelegateHandle = Sessions->AddOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegate);
			//Sessions->UpdateSession(SessionName, *NewSettings, true);
		}
		else
		{
			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
				/// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				/// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void USteamGameInstance::OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnRegisterPlayersComplete %s, %s, %d"), *SessionName.ToString(), *BoolValue, Players.Num())
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnRegisterPlayersComplete %s, %s, %d"), *SessionName.ToString(), *BoolValue, Players.Num()));

	/// Who is calling
	UE_LOG(LogTemp, Warning, TEXT("Calling function: %s"), *WhoCalled)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Calling function: %s"), *WhoCalled));

	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the Delegate again
		Sessions->ClearOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegateHandle);

		/// When matchmaking let BP code be run to decide when to start the match
		OnCreateSessionSuccessfulDelegate.Broadcast();

		/// When in the client, update the settings and broadcast it
		if (bWasSuccessful && !GIsServer)
		{
			/// Set the UpdateSession delegate handle and call UpdateSession
			//OnUpdateSessionCompleteDelegateHandle = Sessions->AddOnUpdateSessionCompleteDelegate_Handle(OnUpdateSessionCompleteDelegate);
			TSharedPtr<FOnlineSessionSettings> NewSettings = MakeShareable(&Sessions->GetNamedSession(SessionName)->SessionSettings);
			//Sessions->UpdateSession(SessionName, *NewSettings, true);
		}
	}
}

void USteamGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s, %s"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnDestroySessionComplete %s, %s"), *SessionName.ToString(), *BoolValue));

	/// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		/// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			/// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			/// If it was successful, we just load another level (could be a MainMenu!)
			if (bWasSuccessful)
			{
				/// when successful, make sure we won't attemp to destroi this session again
				SessionNames.Remove(SessionName);

				/// In BPs one has to block any functionality from executing when quitting the game
				///OnDestroySessionSuccessfulDelegate.Broadcast(bIsQuitting);

				/// Go to main menu when not quitting
				if (!bIsQuitting)
				{
					UGameplayStatics::OpenLevel(GetWorld(), MainMenuMapName, true);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("OnDestroySessionComplete was not successful."))
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete was not successful.")));
			}
		}

		BoolValue = BoolToFString(bIsQuitting);
		UE_LOG(LogTemp, Error, TEXT("bIsQuitting = %s"), *BoolValue)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("bIsQuitting = %s"), *BoolValue));

		/// Session cleanup before quitting
		if (bIsQuitting)
		{
			UE_LOG(LogTemp, Error, TEXT("Inside quit"))
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Inside quit")));

			/// Only destroy sessions with names inside SessionNames array
			if (SessionNames.Num() > 0)
			{
				DestroySession(SessionNames[0]);

				/// Remove the name to avoid trying to destroy it again
				SessionNames.Remove(SessionName);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Quitting now"))
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Quitting now")));
				/// Only quits after destroying all sessions
				UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit);
			}
		}
	}
}

/// BP wrappers
void USteamGameInstance::HostGame(FName SessionName, bool bLanMode, bool bPresence, int32 MaxPlayers)
{
	/// Call our custom CreateSessionInternal function. 
	CreateSessionInternal(GetLocalPlayer()->GetPreferredUniqueNetId(), SessionName, bLanMode, bPresence, MaxPlayers);
}

void USteamGameInstance::StartGame(FName SessionName)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();

    /// This is only called if we are matchmaking and using listen servers
	if (Sessions.IsValid())
	{
		Sessions->StartSession(SessionName);
	}
}

bool USteamGameInstance::FindGames(bool IsLan, bool IsPresence)
{
	/// Call our custom FindSessionInternal function
	return FindSessionsInternal(GetLocalPlayer()->GetPreferredUniqueNetId(), IsLan, IsPresence);
}

bool USteamGameInstance::CancelFindGames()
{
	/// Call our custom CancelFindSessionInternal function
	return CancelFindSessionsInternal();
}

void USteamGameInstance::JoinGame(FName SessionName, FCustomBPSessionResult Result)
{
	/// Unpack the FOnlineSessionSearchResult from Result to call our custom JoinSessionInternal
	FOnlineSessionSearchResult ActualResult = Result.SessionResultInternal;

	/// Just call JoinSessionInternal with the right parameters
	JoinSessionInternal(GetLocalPlayer()->GetPreferredUniqueNetId(), SessionName, ActualResult);
}

void USteamGameInstance::ClientTravelBP(APlayerController * const LocalPlayerController, FString TravelURL, ETravelType TravelType)
{
	LocalPlayerController->ClientTravel(TravelURL, TravelType);
}

FString USteamGameInstance::GetSessionStateBP(FName SessionName)
{
	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	return EOnlineSessionState::ToString(Sessions->GetSessionState(SessionName));
}

bool USteamGameInstance::GetClientConnectedBP(FName SessionName)
{
	/// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	bool bIsClientConnected = false;

	if (Sessions->GetNamedSession(SessionName))
	{
		if (Sessions->GetNamedSession(SessionName)->SessionSettings.Settings.Find("ConnectedClient"))
		{
			UE_LOG(LogTemp, Warning, TEXT("Session %s found."), *SessionName.ToString())
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Session %s found."), *SessionName.ToString()));
			Sessions->GetNamedSession(SessionName)->SessionSettings.Settings.Find("ConnectedClient")->Data.GetValue(bIsClientConnected);
		}
		return bIsClientConnected;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session %s not found."), *SessionName.ToString())
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Session %s not found."), *SessionName.ToString()));
		return bIsClientConnected;
	}

}

void USteamGameInstance::DestroySession(FName SessionName)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();

	/* 
	Since there is no other DestroySession function like the other session operations 
	the binding of the callback is done here
	*/
	if (Sessions.IsValid())
	{
		Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
		Sessions->DestroySession(SessionName);
	}
}
