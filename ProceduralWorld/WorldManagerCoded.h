// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelActor.h"
#include "WorldManagerCoded.generated.h"

UCLASS()
class PROCEDURALWORLD_API AWorldManagerCoded : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldManagerCoded();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 renderRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkElementsXY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 voxelSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 randomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkSizeHalf;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	FVector characterPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkXindex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	int32 chunkYindex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	float xTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	float yTransform;

	//Arrays
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings")
	TArray<AVoxelActor*> chunkArray;
	//TArray<TSubclassOf<AActor*>> chunkArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
	TArray<FVector2D> chunkCoordinates;

	DECLARE_DYNAMIC_DELEGATE(FSpawnObjectFunction);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Variables/")
		TArray<AVoxelActor*>	_WaitingChunkList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Variables/")
		TArray<AVoxelActor*>	_ChunkList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Variables/")
		TArray<AActor*>	_WaterPlaneList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Variables/")
		TArray<AActor*>	_FallingLeavesEffectList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Variables/")
		TArray<AActor*>	_WindEffectList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
		int viewDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
		int Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldManagerCoded/Settings/")
		int PointOffset;

	//DECLARE_DYNAMIC_DELEGATE(FRemoveObjectFunction);

	//Functions
	UFUNCTION(BlueprintCallable, Category = "WorldManagerCoded/Functions/")
		void AddChunk(FSpawnObjectFunction SpawnChunkFunction);

	UFUNCTION(BlueprintCallable, Category = "WorldManagerCoded/Functions/")
		void RemoveChunk(/*FRemoveObjectFunction RemoveChunkFunction*/);

	UFUNCTION(BlueprintCallable, Category = "WorldManagerCoded/Functions/")
		bool CheckRadius(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "WorldManagerCoded/Functions/")
		void Initialize();

	UFUNCTION(BlueprintCallable, Category = "WorldManagerCoded/Functions/")
		bool UpdatePosition();


};
