// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include <FastNoiseWrapper.h>
#include "VoxelActor.generated.h"


struct FMeshSection {
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;
	int32 elementID = 0;
};

struct FBiomes {
	float biomesRangeValue[2];
	float bNoiseValue;
};


USTRUCT(BlueprintType)
struct FNoiseParameter {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	EFastNoise_NoiseType NoiseType;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	int	Seed;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	float Frequency;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	EFastNoise_Interp Interp;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	EFastNoise_FractalType FractalType;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	int Octaves;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	float Gain;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	float Lacunarity;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	float CellularJitter;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	EFastNoise_CellularDistanceFunction	CellularDistanceFunction;

	UPROPERTY(BlueprintReadWrite, Category = "Noise Parameters")
	EFastNoise_CellularReturnType CellularReturnType;
};

UCLASS()
class PROCEDURALWORLD_API AVoxelActor : public AActor
{
	GENERATED_BODY()

public:

	FNoiseParameter noise_settings;

	UPROPERTY(BlueprintReadOnly, Category = "VoxelActor")
		UFastNoiseWrapper* _Noise;

	UPROPERTY(BlueprintReadOnly, Category = "VoxelActor")
		UFastNoiseWrapper* _Noise2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray <UMaterialInterface*> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 randomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 VoxelSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 chunkLineElements = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 chunkXindex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		int32 chunkYindex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
		float biomes_noise_value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float xMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float yMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float zMult = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float freq = 1;

	UPROPERTY()
		int32 chunkLineElementsExt;

	UPROPERTY()
		int32 chunkTotalElements;

	UPROPERTY()
		int32 chunkLineElementsP2Ext;

	/*UPROPERTY()
		int32 chunkLineElementsExt;*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 chunkZElements;

	UPROPERTY()
		int32 chunkLineElementsP2;

	UPROPERTY()
		int32 VoxelSizeHalf;

	UPROPERTY()
		TArray<int32> chunkFields;

	UPROPERTY()
		UProceduralMeshComponent* proceduralComponent;

	// Sets default values for this actor's properties
	AVoxelActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "VoxelActor/Functions")
		void StartGeneration();

	TArray<FMeshSection> _MeshSections;

	UPROPERTY(BlueprintReadOnly, Category = "VoxelActor/Variables")
		bool _IsGenerated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelActor/Variables")
		TArray<FVector> _GrassLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelActor/Variables")
		TArray<FVector> _FlowerLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelActor/Variables")
		TArray<FVector> _FallingLeavesLocations;

	UFUNCTION(BlueprintCallable, Category = "VoxelActor/Functions")
		bool CheckGeneratedStatus();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelActor/Variables")
		float biome_noise_value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelActor/Variables/")
		FNoiseParameter biomes_noise_settings;

};

class ExecuteOnOtherThread : public FNonAbandonableTask {

protected:
	AVoxelActor* _Chunk;
	bool& _IsGenerated;
	TArray<FMeshSection> &_MeshSections;
	TArray<FVector>& _GrassLocations;
	TArray<FVector>& _FlowerLocations;
	TArray<FVector>& _FallingLeavesLocations;

	void SetupNoise();

	void SetupBiomesNoise();
	
	void GenerateChunk();

	void UpdateMesh();

	bool inRange(int32 value, int32 range);

	TArray<int32> calculateNoise(int currentBiome);
	

public:
	ExecuteOnOtherThread(bool& IsGenerated, TArray<FMeshSection> &MeshSections, TArray<FVector> &GrassLocations, TArray<FVector>& FlowerLocations, TArray<FVector>& FallingLeavesLocations, AVoxelActor* Chunk)
		: _IsGenerated(IsGenerated), _MeshSections(MeshSections), _GrassLocations(GrassLocations), _FlowerLocations(FlowerLocations), _FallingLeavesLocations(FallingLeavesLocations), _Chunk(Chunk) {}

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExecuteOnOtherThread, STATGROUP_ThreadPoolAsyncTasks);
	}

	void	DoWork();
};