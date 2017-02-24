// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "MotionTrackSaveSystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class ELEMENTALCOMBAT_API UMotionTrackSaveSystem : public USaveGame
{
	GENERATED_BODY()
	
public:
	// Public methods
	// See if the file exists
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static bool DoesFileExist(const FString Dir, const FString SaveFile, const FString FileExtension = ".dat");
	// Create Save File
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static bool CreateSaveFile(const FString Dir, const FString SaveFile, const FString FileExtension = ".dat");
	// Save Game State after serialization
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static bool SaveGameState(USaveGame* SaveGameObject, const FString Dir, const FString SaveFile, const FString FileExtension = ".dat");
	// Load Game State after deserialization
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static USaveGame* LoadGameState(const FString Dir, const FString SaveFile, const FString FileExtension = ".dat");
};
