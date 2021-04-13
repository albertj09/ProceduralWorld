// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include <FastNoiseWrapper.h>
#include "VoxelChunk.generated.h"

USTRUCT(BlueprintType)
struct FProceduralBuffer {
	GENERATED_USTRUCT_BODY()

	TArray<FVector>	Vertices;
	TArray<int>	Triangles;
	TArray<FLinearColor> VertexColors;
	TArray<FVector2D> Uvs;
};

UENUM()
enum Density {
	Floor3D		UMETA(DIsplayName = "FLoor3D"),
	Floor		UMETA(DisplayName = "Floor"),
	Cave		UMETA(DisplayName = "Cave")
};

struct FChunkParameter {

	int	Scale;
	int	PointOffset;
	int	FloorValue;
	bool Interpolation;
	bool Water;
	float WaterLevel;
	TEnumAsByte<Density> DensityType;
};

struct FNoiseParameter {

	int	Amplitude;
	EFastNoise_NoiseType NoiseType;
	int	Seed;
	float Frequency;
	EFastNoise_Interp Interp;
	EFastNoise_FractalType FractalType;
	int Octaves;
	float Gain;
	float Lacunarity;
	float CellularJitter;
	EFastNoise_CellularDistanceFunction	CellularDistanceFunction;
	EFastNoise_CellularReturnType CellularReturnType;
};

struct FPointFNG {

	FVector	Pos;
	int	Value;
};

UCLASS()
class PROCEDURALWORLD_API AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelChunk();

	FChunkParameter chunk_settings;

	FNoiseParameter noise_settings;

	//TArray<FPointFNG> points;

	UFUNCTION(BlueprintCallable, Category = "VoxelChunkClass")
		void StartGeneration();
	UFUNCTION(BlueprintCallable, Category = "VoxelChunkClass")
		bool CheckGeneratedStatus();
	UFUNCTION(BlueprintCallable, Category = "VoxelChunkClass")
		int	GetPointValue(FVector PointLocation, FVector ActorLocation) const;
	UFUNCTION(BlueprintCallable, Category = "VoxelChunkClass")
		int	GetPointDensityFromLocation(FVector PointLocation, FVector ActorLocation) const;

	UPROPERTY(BlueprintReadOnly, Category = "VoxelChunkClass")
	bool _IsGenerated;

	const static int trigoTable[256][16];

	void FillBuffer(TArray<FPointFNG> pointsArray, int NumberOfPointIn1Axis);

protected:


	UPROPERTY(BlueprintReadOnly, Category = "VoxelChunkClass")
		UFastNoiseWrapper* _Noise;
	UPROPERTY(BlueprintReadOnly, Category = "VoxelChunkClass")
		UProceduralMeshComponent* _ProceduralMesh;

	UPROPERTY(BlueprintReadOnly, Category = "VoxelChunkClass")
		FProceduralBuffer _ProceduralMeshBuffer;
		
	UPROPERTY(BlueprintReadOnly, Category = "VoxelChunkClass")
		TMap<FVector, int> _PointDensityMap;

	int	ConvertPointsToTrigoIndex(TArray<FPointFNG> Points, int FloorValue) const;

	FVector	GetLocationOfTrigoPoint(int PointIndex, TArray<FPointFNG> Points) const;

};



class ParallelTask : public FNonAbandonableTask {



public:

	int32 chunkx;


public:

	ParallelTask();

	//The idea is to run the add/remove chunk functions together on a separate thread	

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ParallelTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork();


};
