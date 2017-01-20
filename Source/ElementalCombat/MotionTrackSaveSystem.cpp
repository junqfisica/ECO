// Fill out your copyright notice in the Description page of Project Settings.

#include "ElementalCombat.h"
#include "MotionTrackSaveSystem.h"

bool UMotionTrackSaveSystem::DoesFileExist(const FString Dir, const FString SaveFile)
{
	FString TargetDir = FPaths::GameDir() + Dir;
	return FPaths::FileExists(TargetDir + SaveFile + ".sav");
}

bool UMotionTrackSaveSystem::CreateSaveFile(const FString Dir, const FString SaveFile)
{
	// Create directory
	FString TargetDir = FPaths::GameDir() + Dir;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*TargetDir))
	{
		PlatformFile.CreateDirectory(*TargetDir);

		if (!PlatformFile.DirectoryExists(*TargetDir))
		{
			UE_LOG(LogTemp, Error, TEXT("Directory could not be created."))
				return false;
		}
	}

	// Create empty save file
	IFileHandle* FileHandle = PlatformFile.OpenWrite(*(TargetDir + SaveFile + ".sav"));
	if (FileHandle)
	{
		delete FileHandle;
	}

	return FPaths::FileExists(TargetDir + SaveFile + ".sav");
}

bool UMotionTrackSaveSystem::SaveGameState(USaveGame * SaveGameObject, const FString Dir, const FString SaveFile)
{
	// Get the file for writting data 
	FString TargetDir = FPaths::GameDir() + Dir;

	// Serialization
	FBufferArchive MemoryWriter;

	// Write the class name so we know what class to load to
	FString SaveGameClassName = SaveGameObject->GetClass()->GetName();
	MemoryWriter << SaveGameClassName;

	// Then save the object state, replacing object refs and names with strings
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	SaveGameObject->Serialize(Ar);

	// Write the data to SaveFile in TargetDir
	return FFileHelper::SaveArrayToFile(MemoryWriter, *(TargetDir + SaveFile + ".sav"));
}

USaveGame * UMotionTrackSaveSystem::LoadGameState(const FString Dir, const FString SaveFile)
{
	// Get the file for reading data
	FString TargetDir = FPaths::GameDir() + Dir;

	// Read binary data to BinaryArray
	TArray<uint8> BinaryArray;
	FFileHelper::LoadFileToArray(BinaryArray, *(TargetDir + SaveFile + ".sav"));

	// Return deserialized data
	USaveGame * SaveData = NULL;
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);

	// Get the class name
	FString SaveGameClassName;
	FromBinary << SaveGameClassName;

	// Try and find it, and failing that, load it
	UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveGameClassName);
	if (SaveGameClass == NULL)
	{
		SaveGameClass = LoadObject<UClass>(NULL, *SaveGameClassName);
	}

	// If we have a class, try and load it.
	if (SaveGameClass != NULL)
	{
		SaveData = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);

		FObjectAndNameAsStringProxyArchive Ar(FromBinary, true);
		SaveData->Serialize(Ar);
	}

	return SaveData;
}
