// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ElementalCombat.h"
#include "SteamGameInstance.generated.h"

/**
*  FCustomBPSessionResults - wrapper struct for session results to be passed to BPs
*/
USTRUCT(BlueprintType)
struct FCustomBPSessionResult
{
	GENERATED_USTRUCT_BODY()

	FOnlineSessionSearchResult SessionResultInternal;
};

/** Delegates to be implemented in BP */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateSessionSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsSuccessful, const TArray<FCustomBPSessionResult>&, BPSessionResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFindSessionsNotSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionSuccessful, FString, TravelURL, ETravelType, TravelType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestroySessionSuccessful, bool, bIsQuitting);

/**
 *  USteamGameInstance - UGameInstance class to be used as base for BP GameInstance; has all session systems
 */
UCLASS()
class ELEMENTALCOMBAT_API USteamGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	/// Fields
	/** Session properties used in creation and results */
	TSharedPtr<class FOnlineSessionSettings> SessionSettings; /// Creation
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;     /// Finding

	/** Array containing the names of all sessions created - used when quitting the game */
	TArray<FName> SessionNames;
	
public:
	/// Fields
	/** Map name to use in the StartSession callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FName ArenaMapName;

	/** Map name to use in the DestroySession callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FName MainMenuMapName;

	/** Default session name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FName GameSessionName;

	/** Determines if we are quitting the game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bIsQuitting;

	/** Determines if we are using matchmaking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bIsMatchMaking;

	/** Determines if we are using matchmaking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString WhoCalled;

	/**
	*  Delegates used in the callbacks of the OnlineSessionInterface; 
	*  bound in BPs
	*/
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnCreateSessionSuccessful OnCreateSessionSuccessfulDelegate;     /// Create was successful
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnFindSessionsSuccessful OnFindSessionsSuccessfulDelegate;       /// Find found sessions
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnFindSessionsNotSuccessful OnFindSessionsNotSuccessfulDelegate; /// Find found no sessions
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnJoinSessionSuccessful OnJoinSessionSuccessfulDelegate;         /// Join was successful
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnDestroySessionSuccessful OnDestroySessionSuccessfulDelegate;   /// Destroy was successful
	
	/** Delegates for the callbacks of the OnlineSessionInterface */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;           /// Create call
	FOnUpdateSessionCompleteDelegate OnUpdateSessionCompleteDelegate;           /// Update call
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;             /// Start call
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;             /// Find call
	FOnCancelFindSessionsCompleteDelegate OnCancelFindSessionsCompleteDelegate; /// Cancel Find call
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;               /// Join call
	FOnRegisterPlayersCompleteDelegate OnRegisterPlayersCompleteDelegate;       /// Register Players call
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;         /// Destroy call

	/** Handles to registered delegates for all actions involving a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;      /// Create
	FDelegateHandle OnUpdateSessionCompleteDelegateHandle;      /// Update
	FDelegateHandle OnStartSessionCompleteDelegateHandle;       /// Start
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;       /// Find
	FDelegateHandle OnCancelFindSessionsCompleteDelegateHandle; /// Cancel Find
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;        /// Join
	FDelegateHandle OnRegisterPlayersCompleteDelegateHandle;    /// Register Players
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;     /// Destroy

	/// Methods
	USteamGameInstance(const FObjectInitializer& ObjectInitializer); /// Contructor
	
	/// Accessor methods for FCustomBPSessionResult struct
	/**
	*  Function gets the session name from the session result wrapped by FCustomBPSessionResult
	*
	*  @param	Result		wrapped session result
	*/
	UFUNCTION(BlueprintPure, Category = "Network")
		static FString GetSessionName(FCustomBPSessionResult Result);

	/** 
	*  Function gets the ping in ms from the session result wrapped by FCustomBPSessionResult
	*
	*  @param	Result		wrapped session result
	*/
	UFUNCTION(BlueprintPure, Category = "Network")
	static int32 GetPingInMs(FCustomBPSessionResult Result) { return Result.SessionResultInternal.PingInMs; };

	/**
	*  Function gets the maximum namber of players possible from the session result wrapped by FCustomBPSessionResult
	*
	*  @param	Result		wrapped session result
	*/
	UFUNCTION(BlueprintPure, Category = "Network")
	static int32 GetMaxPlayers(FCustomBPSessionResult Result) { return Result.SessionResultInternal.Session.SessionSettings.NumPublicConnections; };

	/**
	*  Function gets current number of players from the session result wrapped by FCustomBPSessionResult
	*
	*  @param	Result		wrapped session result
	*/
	UFUNCTION(BlueprintPure, Category = "Network")
	static int32 GetCurrentPlayers(FCustomBPSessionResult Result) {
		return Result.SessionResultInternal.Session.SessionSettings.NumPublicConnections
			- Result.SessionResultInternal.Session.NumOpenPublicConnections; };

	/// Helper methods
	/**
	*   Function prints all session settings used to create a session
	*
	*  	@Param		Search  		Settings to be printed
	*/
	void PrintSessionSearch(const FOnlineSessionSearch& Search);

	/**
	*   Function prints all session settings used to create a session
	*
	*  	@Param		Settings		Settings to be printed
	*/
	void PrintSessionSettings(const FOnlineSessionSettings& Settings);

	/**
	*   Function returns the session interface the app is using
	*/
	IOnlineSessionPtr GetSessionInterface();

	/**
	*   Function returns current player
	*/
	ULocalPlayer* const GetLocalPlayer();

	/**
	*   Function returns converted bool in to FString
	*
	*   @param		bBoolToConvert		boolean to convert to FString
	*/
	FString BoolToFString(bool bBoolToConvert);

	/**
	*   Function returns converted EOnJoinSessionCompleteResult::Type to FString
	*
	*   @param		Value				boolean to convert to FString
	*/
	FString ToString(const EOnJoinSessionCompleteResult::Type Value);

	/**
	*   Function packs the FOnlineSessionSearchResult list into a list of FCustomBPSessionResult
	*
	*   @param		Results		List of session results
	*/
	const TArray<FCustomBPSessionResult> PackageSessionResults(TArray<FOnlineSessionSearchResult> Results);

	/// Session methods
	/**
	*   Function creates a session using the OnlineSessionInterface
	*
	*  	@Param		UserID			User that started the request
    *	@Param		SessionName		Name of the Session
    *	@Param		bIsLAN			Is this is LAN Game?
    *	@Param		bIsPresence		Is the Session to create a presence Session?
    *	@Param		MaxNumPlayers	Maximum number of players allowed on this Session
	*	@return		bool true if successful, false otherwise
	*/
	bool CreateSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	
	/**
	*	Find an online session
	*
	*	@param		UserId			User that initiated the request
	*	@param		bIsLAN			Are we searching LAN matches
	*	@param		bIsPresence		Are we searching presence sessions
	*	@return		bool true if successful, false otherwise
	*/
	bool FindSessionsInternal(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);

	/**
	*	Cancel finding online sessions 
	*/
	bool CancelFindSessionsInternal();

	/**
	*	Joins a session via a search result
	*
	*	@return		bool true if successful, false otherwise
	*/
	bool JoinSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	/**
	*	Registers the player in a given session
	*
	*	@param		SessionName			Name of session
	*	@param		PlayerId			ID for the registering player
	*   @param		bWasInvited			Was this player invited
	*	@return bool true if successful, false otherwise
	*/
	bool RegisterPlayerInternal(FName SessionName, TSharedPtr<const FUniqueNetId> PlayerId, bool bWasInvited);


	/// Callbacks for the OnlineSessionInterface
	/**
	*   Function fired when a session create request has completed
	*
	*	@param		SessionName			 the name of the session this callback is for
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*   Function fired when a session update request has completed
	*
	*	@param		SessionName			 the name of the session this callback is for
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session start request has completed
	*
	*	@param		SessionName			 the name of the session this callback is for
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Delegate fired when a session search query has completed
	*
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	*	Delegate fired when canceletion of a session search query has completed
	*
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnCancelFindSessionsComplete(bool bWasSuccessful);

	/**
	*	Delegate fired when a session join request has completed
	*
	*	@param		SessionName			 the name of the session this callback is for
	*	@param		bWasSuccessful		 true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	*	Delegate fired when regitering process has finished
	*
	*	@param		SessionName			the name of the session this callback is for
	*   @param		Players				Array of players in a given session
	*	@param		bWasSuccessful		true if the async action completed without error, false if there was an error
	*/
	void OnRegisterPlayersComplete(FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& Players, bool bWasSuccessful);

	/**
	*	Delegate fired when a destroying an online session has completed
	*
	*	@param		SessionName			the name of the session this callback is for
	*	@param		bWasSuccessful		true if the async action completed without error, false if there was an error
	*/
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/// BP friendly wrappers of the session methods
	/**
	*   Function wraps the CreateSessionInternal method as a BP node
	*
	*	@Param		SessionName		Name of the Session
	*	@Param		bLanMode    	Is this is LAN Game?
	*	@Param		bPresence		Is the Session to create a presence Session?
	*	@Param		MaxPlayers  	Maximum number of players allowed on this Session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HostGame(FName SessionName, bool bLanMode, bool bPresence, int32 MaxPlayers);

	/**
	*   Function wraps IOnlineSubSystemInt::StartSession function as a BP node
	*
	*	@Param		SessionName		Name of the Session
	*	@Param		bLanMode    	Is this is LAN Game?
	*	@Param		bPresence		Is the Session to create a presence Session?
	*	@Param		MaxPlayers  	Maximum number of players allowed on this Session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartGame(FName SessionName);

	/**
	*   Function wraps IOnlineSubSystemInt::DestroySession function as a BP node
	*
	*	@Param		SessionName		Name of the Session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySession(FName SessionName);

	/**
	*   Function wraps the FindSessionInternal method as a BP node
	*
	*   @param		IsLan			Determines if the session is a lan session
	*   @param		IsPresence		Is the session a presence session?
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool FindGames(bool IsLan, bool IsPresence);

	/**
	*   Function wraps the CancelFindSessionInternal method as a BP node
	*
	*   @param		IsLan			Determines if the session is a lan session
	*   @param		IsPresence		Is the session a presence session?
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool CancelFindGames();

	/**
	*   Function wraps the JoinSessionInternal method as a BP node
	*
	*   @param		SessionName		Name of the session
	*   @param		Result  		Session result wrapped by FCustomBPSessionResult; holds the session to join
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinGame(FName SessionName, FCustomBPSessionResult Result);

	/**
	*   Function wraps APlayerController::ClientTravel method as a BP node
	*
	*  	@Param		Settings		Settings to be printed
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ClientTravelBP(APlayerController * const LocalPlayerController, FString TravelURL, ETravelType TravelType);

	/**
	*   Function wraps the IOnlineSubSystemInt::GetSessionState method as a BP node
	*
	*   @param		SessionName		Name of the session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	FString GetSessionStateBP(FName SessionName);

	/**
	*   Function gets the current number of players from a named session
	*
	*   @param		SessionName		Name of the session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool GetClientConnectedBP(FName SessionName);
};
