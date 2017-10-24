// Property of Avrlon, all rights reserved.

#include "ElementalCombat.h"
#include "SteamGameInstance.h"

USteamGameInstance::USteamGameInstance(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	ArenaMapName = "";
	MainMenuMapName = "";
	GameSessionName = "TestSession";
		
	/// Initialization of the SessionSetting/Search here to have available at the very begining
	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

    /// Binding the session callbacks to our methods
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnStartOnlineGameComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnJoinSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &USteamGameInstance::OnDestroySessionComplete);
}

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

/// Helper methods
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

		SessionSettings->Set(SETTING_MAPNAME, FString(""), EOnlineDataAdvertisementType::ViaOnlineService);

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

void USteamGameInstance::FindSessionsInternal(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && UserId.IsValid())
	{
		/*
		Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
		*/
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		
		SessionSearch->bIsLanQuery = bIsLAN;
		SessionSearch->MaxSearchResults = 20;
		SessionSearch->PingBucketSize = 9999;

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
		Sessions->FindSessions(*UserId, SearchSettingsRef);
	}
	else
	{
		/// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
	}
}

bool USteamGameInstance::JoinSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult & SearchResult)
{
	// Return bool
	bool bSuccessful = false;

	// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid() && UserId.IsValid())
	{
		// Set the Handle again
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

		// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
		// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
		bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
	}

	return bSuccessful;
}

/// Callbacks
void USteamGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnCreateSessionComplete %s, %s"), *SessionName.ToString(), *BoolValue));

    /// Get the Session Interface to call the StartSession function
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		/// Clear the SessionComplete delegate handle, since we finished this call
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		if (bWasSuccessful)
		{
			/// Set the StartSession delegate handle
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			
			/// Our StartSessionComplete delegate should get called after this
			Sessions->StartSession(SessionName);
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
	}

	/// If the start was successful, we can open MapName if we want. Make sure to use "listen" as a parameter!
	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), ArenaMapName, true, "listen");
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

void USteamGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result))
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));

	// Get SessionInterface from the OnlineSubsystem
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if (Sessions.IsValid())
	{
		// Clear the Delegate again
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

		// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
		// This is something the Blueprint Node "Join Session" does automatically!
		APlayerController * const PlayerController = GetFirstLocalPlayerController();

		// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
		// String for us by giving him the SessionName and an empty String. We want to do this, because
		// Every OnlineSubsystem uses different TravelURLs
		FString TravelURL;

		if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
		{
			// Finally call the ClienTravel. If you want, you could print the TravelURL to see
			// how it really looks like
			PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		}
	}
}

void USteamGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString BoolValue = BoolToFString(bWasSuccessful);
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), *BoolValue)
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), *BoolValue));

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
				UGameplayStatics::OpenLevel(GetWorld(), MainMenuMapName, true);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("OnDestroySessionComplete was not successful."))
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete was not successful.")));
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

void USteamGameInstance::FindGames(bool IsLan, bool IsPresence)
{
	/// Call our custom FindSessionInternal function
	FindSessionsInternal(GetLocalPlayer()->GetPreferredUniqueNetId(), IsLan, IsPresence);
}

void USteamGameInstance::JoinGame(FName SessionName, FCustomBPSessionResult Result)
{
	/// Unpack the FOnlineSessionSearchResult from Result to call our custom JoinSessionInternal
	FOnlineSessionSearchResult ActualResult = Result.SessionResultInternal;

	/// Just call JoinSessionInternal with the right parameters
	JoinSessionInternal(GetLocalPlayer()->GetPreferredUniqueNetId(), SessionName, ActualResult);
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
