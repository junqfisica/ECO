// Property of Avrlon, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SteamGameInstance.generated.h"

/**
*
*/
USTRUCT(BlueprintType)
struct FCustomBPSessionResults
{
	GENERATED_USTRUCT_BODY()

		FOnlineSessionSearchResult SessionResultInternal;
};

/// These are delegates types for BP binding
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsSuccessful, TArray<FOnlineSessionSearchResult>, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFindSessionsNotSuccessful);

/**
 * 
 */
UCLASS()
class ELEMENTALCOMBAT_API USteamGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	/// Fields
	/** Session properties used in creation and results */
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
public:
	/// Fields
	/** Map name to use in the StartSession callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FName ArenaMapName;

	/** Map name to use in the DestroySession callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MainMenuMapName;

	/** Map name to use in the DestroySession callback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameSessionName;

	/**
	* Delegates used in the callbacks of the OnlineSessionInterface; 
	* bound in BPs
	*/
	FOnFindSessionsSuccessful OnFindSessionsSuccessfulDelegate;       /// Successful
	FOnFindSessionsNotSuccessful OnFindSessionsNotSuccessfulDelegate; /// Not successful

	/** Delegates for the callbacks of the OnlineSessionInterface */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;   /// Create call
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;     /// Start call
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;     /// Find call
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;       /// Join call
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate; /// Destroy call

	/** Handles to registered delegates for all actions involving a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;  /// Create
	FDelegateHandle OnStartSessionCompleteDelegateHandle;   /// Start
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;   /// Find
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;    /// Join
	FDelegateHandle OnDestroySessionCompleteDelegateHandle; /// Destroy

	/// Methods
	USteamGameInstance(const FObjectInitializer& ObjectInitializer);
	
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

	/// Session methods
	/**
	*   Function creates a session using the OnlineSessionInterface
	*
	*  	@Param		UserID			User that started the request
    *	@Param		SessionName		Name of the Session
    *	@Param		bIsLAN			Is this is LAN Game?
    *	@Param		bIsPresence		Is the Session to create a presence Session?
    *	@Param		MaxNumPlayers	Maximum number of players allowed on this Session
	*/
	bool CreateSessionInternal(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);

	/**
	*	Find an online session
	*
	*	@param UserId user that initiated the request
	*	@param bIsLAN are we searching LAN matches
	*	@param bIsPresence are we searching presence sessions
	*/
	void FindSessionsInternal(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);

	/// Callbacks for the OnlineSessionInterface
	/**
	*   Function fired when a session create request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session start request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Delegate fired when a session search query has completed
	*
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	*	Delegate fired when a session join request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	*	Delegate fired when a destroying an online session has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
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
	*   Function wraps IOnlineSubSystemInt::DestroySession function as a BP node
	*
	*	@Param		SessionName		Name of the Session
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void DestroySession(FName SessionName);

	/**
	*   Function wraps the FindSessionInternal method as a BP node
	*/
	UFUNCTION(BlueprintCallable, Category = "Network")
	void FindGames(bool IsLan, bool IsPresence);
};
