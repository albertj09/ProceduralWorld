// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelActor.h"







const int32 bTriangles[] = { 2,1,0,0,3,2 };
//const int32 bTriangles[] = { 2,3,0,0,1,2 };


const FVector2D bUVs[] = { FVector2D(0.000000, 0.000000), FVector2D(0.00000, 1.00000), FVector2D(1.00000, 1.00000), FVector2D(1.00000, 0.000000) };
const FVector bNormals0[] = { FVector(0,0,1), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1) };
const FVector bNormals1[] = { FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1) };
const FVector bNormals2[] = { FVector(0,1,0),FVector(0,1,0) ,FVector(0,1,0) ,FVector(0,1,0) };
const FVector bNormals3[] = { FVector(0,-1,0),FVector(0,-1,0), FVector(0,-1,0), FVector(0,-1,0) };
const FVector bNormals4[] = { FVector(1,0,0),FVector(1,0,0), FVector(1,0,0), FVector(1,0,0) };
const FVector bNormals5[] = { FVector(-1,0,0),FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0) };
const FVector bMask[] = { FVector(0.00000,0.00000,1.00000),FVector(0.00000,0.00000,-1.00000) ,FVector(0.00000,1.00000,0.00000) ,FVector(0.00000,-1.00000,0.00000), FVector(1.00000,0.0000,0.00000),FVector(-1.00000,0.0000,0.00000) };

// Sets default values
AVoxelActor::AVoxelActor()
{
	PrimaryActorTick.bCanEverTick = false;

	_Noise = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise"));

	_Noise2 = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("Noise2"));


}

// Called when the game starts or when spawned
void AVoxelActor::BeginPlay()
{

	Super::BeginPlay();

	

	//The noise settings need to be initialized first and foremost however the constructor can sometimes initialize when the game world is not yet loaded which in turn doesn't properly set the noise settings.
	//So we set the noise settings on the BeginPlay.

	//Terrain noise settings
	noise_settings.Seed = randomSeed;
	noise_settings.NoiseType = EFastNoise_NoiseType::Simplex;
	noise_settings.Frequency = 0.9f;
	noise_settings.Interp = EFastNoise_Interp::Quintic;
	noise_settings.FractalType = EFastNoise_FractalType::FBM;
	noise_settings.Octaves = 3;
	noise_settings.Lacunarity = 2.0f;
	noise_settings.Gain = 0.5f;	//DEFAULT 0.5
	noise_settings.CellularJitter = 0.4f;
	noise_settings.CellularDistanceFunction = EFastNoise_CellularDistanceFunction::Euclidean;
	noise_settings.CellularReturnType = EFastNoise_CellularReturnType::CellValue;

	//Biomes noise settings
	biomes_noise_settings.Seed = 123;
	biomes_noise_settings.NoiseType = EFastNoise_NoiseType::PerlinFractal;
	biomes_noise_settings.Frequency = 0.1f;
	biomes_noise_settings.Interp = EFastNoise_Interp::Quintic;
	biomes_noise_settings.FractalType = EFastNoise_FractalType::FBM;
	biomes_noise_settings.Octaves = 1;
	biomes_noise_settings.Lacunarity = 1.0f;
	biomes_noise_settings.Gain = 0.0f;	
	biomes_noise_settings.CellularJitter = 0.1f;
	biomes_noise_settings.CellularDistanceFunction = EFastNoise_CellularDistanceFunction::Euclidean;
	biomes_noise_settings.CellularReturnType = EFastNoise_CellularReturnType::CellValue;

}

// Called every frame
void AVoxelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Construction script
void AVoxelActor::OnConstruction(const FTransform& Transform) {

	Super::OnConstruction(Transform); //So we override the actual construction script with this custom one, and then we also need to call the parent construction script itself

	//chunk variables
	chunkZElements = 80;
	chunkLineElementsExt = chunkLineElements + 2;
	chunkTotalElements = chunkLineElementsExt * chunkLineElementsExt * chunkZElements;
	chunkLineElementsP2 = chunkLineElements * chunkLineElements;
	chunkLineElementsP2Ext = chunkLineElementsExt * chunkLineElementsExt;
	VoxelSizeHalf = VoxelSize / 2;


	FString string = "Voxel_" + FString::FromInt(chunkXindex) + "_" + FString::FromInt(chunkYindex);
	FName name = FName(*string);
	proceduralComponent = NewObject<UProceduralMeshComponent>(this, name);
	proceduralComponent->RegisterComponent();

	RootComponent = proceduralComponent; //set the proceduralComponent as root 
	RootComponent->SetWorldTransform(Transform); //set the world transform which we pass as our parameter

	proceduralComponent->bUseAsyncCooking = true;

}

void ExecuteOnOtherThread::GenerateChunk() {
	
	FRandomStream RandomStream = FRandomStream(_Chunk->randomSeed);
	TArray<FIntVector> treeCenters;

	_Chunk->chunkFields.SetNumUninitialized(_Chunk->chunkTotalElements);	//A one dimensional array. Size is the total number of chunk elements


	//Initialize the noise for the biomes
	SetupBiomesNoise();

	//For the this current chunk, determine which biome it is in based on the noise value
	 _Chunk->biome_noise_value = (_Chunk->_Noise2->GetNoise2D(_Chunk->chunkXindex * 0.15f, _Chunk->chunkYindex * 0.15f) - -1) / 2;	//for the biomes, I opted for the 0-1 noise value range, hence the conversion
	
	int biome = 0;
	
	//BIOMES NUMERATION CODE
	
	//0 - OAK FOREST
	if (_Chunk->biome_noise_value >= 0.35 && _Chunk->biome_noise_value < 0.55) {
		biome = 0;
	}
	//1 - GRASSLANDS
	else if(_Chunk->biome_noise_value >= 0 && _Chunk->biome_noise_value < 0.35){
		biome = 1;
	}
	//2 - BIRCH FOREST
	else {
		biome = 2;
	}
	 
	//Calculate noise for terrain generation 
	TArray<int32> noise = calculateNoise(biome);


	for (int32 x = 0; x < _Chunk->chunkLineElementsExt; x++) {
		for (int32 y = 0; y < _Chunk->chunkLineElementsExt; y++) {
			for (int32 z = 0; z < _Chunk->chunkZElements; z++) {
						
				int32 index = x + (y * _Chunk->chunkLineElementsExt) + (z * _Chunk->chunkLineElementsP2Ext);
				if (x + y * _Chunk->chunkLineElementsExt < noise.Num()) {

					if (z == 30 + noise[x + y * _Chunk->chunkLineElementsExt]) {
						_Chunk->chunkFields[index] = 11;	//GRASS
					}
					else if (z == 29 + noise[x + y * _Chunk->chunkLineElementsExt]) {
						_Chunk->chunkFields[index] = 12;	//DIRT
					}
					else if (z < 29 + noise[x + y * _Chunk->chunkLineElementsExt]) {
						_Chunk->chunkFields[index] = 13;	//STONE
					}
					else {
						_Chunk->chunkFields[index] = 0;		//EMPTY SPACE
					}

				}

			}
		}
	}


	//Range for the instances of trees and other foliage
	for (int32 x = 2; x < _Chunk->chunkLineElementsExt - 2; x++) {
		for (int32 y = 2; y < _Chunk->chunkLineElementsExt - 2; y++) {
			for (int32 z = 2; z < _Chunk->chunkZElements; z++) {

				int32 index = x + (y * _Chunk->chunkLineElementsExt) + (z * _Chunk->chunkLineElementsP2Ext);


				//Adjust probabilities of instances depending on the biome of the chunk

				//----------------GRASS INSTANCES
				float grass_probability;
				if (biome == 0) {	//OAK FOREST
					grass_probability = 0.1;
				}
				else if (biome == 1) {	//GRASSLANDS
					grass_probability = 0.2;
				}
				else if (biome == 2) {	//BIRCH FOREST
					grass_probability = 0.1;
				}

				//Add grass instances based on the probability
				if (RandomStream.FRand() < grass_probability && z == 31 + noise[x + y * _Chunk->chunkLineElementsExt]) {
					_Chunk->chunkFields[index] = -1;
				}

				//----------------FLOWER INSTANCES
				float flower_probability;
				if (biome == 0) {	//OAK FOREST
					flower_probability = 0.1;
				}
				else if (biome == 1) {	//GRASSLANDS
					flower_probability = 0.2;
				}
				else if (biome == 2) {	//BIRCH FOREST
					flower_probability = 0.1;
				}
				
				//Add flower instances based on the probability
				if (RandomStream.FRand() < flower_probability && z == 31 + noise[x + y * _Chunk->chunkLineElementsExt]) {
					_Chunk->chunkFields[index] = -2;
				}

				//----------------TREE LOCATIONS & FALLING LEAVES EFFECT
				float tree_probability;
				if (biome == 0) {	//OAK FOREST
					tree_probability = 0.01;
				}
				else if (biome == 1) {	//GRASSLANDS
					tree_probability = 0.004;
				}
				else if (biome == 2) {	//BIRCH FOREST
					tree_probability = 0.01;
				}

				//Add tree and falling leaves instances based on the probability
				if (RandomStream.FRand() < tree_probability && z == 31 + noise[x + y * _Chunk->chunkLineElementsExt]) {
					//Check at what z location the tree is, and do not add it if it's way below the water level
					//Water level is hardcoded at 1430.0f world space Z coordinate
					if (z * _Chunk->VoxelSize > (1430.0f - _Chunk->VoxelSize)) {	//trees can spawn at maximum of 1 block below the water level
						treeCenters.Add(FIntVector(x, y, z));	//We are adding these locations in local space because the trees themselves are voxels
						
						//Make sure that the instances of falling leaves are dependent on the trees, so we spawn them at the same locations as the trees
						_FallingLeavesLocations.Add(FVector(x * _Chunk->VoxelSize, y * _Chunk->VoxelSize, z * _Chunk->VoxelSize)); //Get and transform the locations into world space
					}				
				}
			}
		}
	}

	
	//For each tree

	int32 tree_num = 0;

	for (FIntVector treeCenter : treeCenters) {
		int32 tree_height = RandomStream.RandRange(3, 12);
		int32 randomX = RandomStream.RandRange(0, 3);
		int32 randomY = RandomStream.RandRange(0, 3);
		int32 randomZ = RandomStream.RandRange(0, 3);

		

		//Configure the leaves to be dependant on the tree height, so we don't get really short trees with too many leaves which look extremely unnatural 
		for (int32 tree_x = (tree_height > 4 ? -3 : -2); tree_x < (tree_height > 4 ? 4 : 3); tree_x++) {
			for (int32 tree_y = (tree_height > 4 ? -3 : -2); tree_y < (tree_height > 4 ? 4 : 3); tree_y++) {
				for (int32 tree_z = (tree_height > 4 ? -3 : -2); tree_z < (tree_height > 4 ? 4 : 3); tree_z++) {
					if (inRange(tree_x + treeCenter.X + 1, _Chunk->chunkLineElements + 1) && inRange(tree_y + treeCenter.Y + 1, _Chunk->chunkLineElements + 1) && inRange(tree_z + treeCenter.Z, _Chunk->chunkZElements)) {
						float radius = FVector(tree_x * randomX, tree_y * randomY, tree_z * randomZ).Size();

						if (radius <= 4.0) {
							if (RandomStream.FRand() < 0.5 || radius < 0.8) {
								_Chunk->chunkFields[treeCenter.X + tree_x + (_Chunk->chunkLineElementsExt * (treeCenter.Y + tree_y)) + (_Chunk->chunkLineElementsP2Ext * (treeCenter.Z + tree_z + tree_height))] = 1;	//LEAVES MATERIAL
							}
						}
					}

				}
			}
		}
		
		//tree trunk
		for (int32 h = 0; h < tree_height; h++) {
			_Chunk->chunkFields[treeCenter.X + (treeCenter.Y * _Chunk->chunkLineElementsExt) + ((treeCenter.Z + h) * _Chunk->chunkLineElementsP2Ext)] = (biome == 2 ? 15 : 14);		//BIRCH OR OAK DEPENDING ON THE BIOME
		}

		//adjust the height of the falling leaves effects per each tree, so the effect will be sourced from somewhere around the leaves area
		_FallingLeavesLocations[tree_num].Z += ((tree_height * 0.75) * _Chunk->VoxelSize);

		tree_num++;
		
	}

}

void ExecuteOnOtherThread::UpdateMesh() {

	
	_MeshSections.SetNum(_Chunk->Materials.Num());
	int32 element_num = 0;


	for (int32 x = 0; x < _Chunk->chunkLineElements; x++) {
		for (int32 y = 0; y < _Chunk->chunkLineElements; y++) {
			for (int32 z = 0; z < _Chunk->chunkZElements; z++) {

				int32 index = (x + 1) + (_Chunk->chunkLineElementsExt * (y + 1)) + (_Chunk->chunkLineElementsP2Ext * z);
				int32 meshIndex = _Chunk->chunkFields[index]; //value of a voxel

				if (meshIndex > 0) {
					meshIndex--;

					//We need references to our original objects
					TArray<FVector>& Vertices = _MeshSections[meshIndex].Vertices;
					TArray<int32>& Triangles = _MeshSections[meshIndex].Triangles;
					TArray<FVector>& Normals = _MeshSections[meshIndex].Normals;
					TArray<FVector2D>& UVs = _MeshSections[meshIndex].UVs;
					TArray<FProcMeshTangent>& Tangents = _MeshSections[meshIndex].Tangents;
					TArray<FColor>& VertexColors = _MeshSections[meshIndex].VertexColors;
					int32 elementID = _MeshSections[meshIndex].elementID;

					int triangle_num = 0;
					for (int i = 0; i < 6; i++)
					{
						int newIndex = index + bMask[i].X + (bMask[i].Y * _Chunk->chunkLineElementsExt) + (bMask[i].Z * _Chunk->chunkLineElementsP2Ext);
						bool flag = false;	//should be FALSE!!!!!!!!!!!!!!!!!


						if (meshIndex >= 20) {
							flag = true;
						}
						//else if ((x + bMask[i].X < chunkLineElements) && (x + bMask[i].X >= 0) && (y + bMask[i].Y < chunkLineElements) && (y + bMask[i].Y >= 0) /*&& (z + bMask[i].Z >= 0) && (z + bMask[i].Z < chunkZElements)*/)
						//{
						else if (newIndex < _Chunk->chunkFields.Num() && newIndex >= 0) {
							if (_Chunk->chunkFields[newIndex] < 10) flag = true;
						}
						//}
						/*else {
							flag = true;
						} */

						if (flag)
						{
							Triangles.Add(bTriangles[0] + triangle_num + elementID);
							Triangles.Add(bTriangles[1] + triangle_num + elementID);
							Triangles.Add(bTriangles[2] + triangle_num + elementID);
							Triangles.Add(bTriangles[3] + triangle_num + elementID);
							Triangles.Add(bTriangles[4] + triangle_num + elementID);
							Triangles.Add(bTriangles[5] + triangle_num + elementID);
							triangle_num += 4;

							switch (i)
							{
							case 0: {
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals0, UE_ARRAY_COUNT(bNormals0));
								break;
							}
							case 1: {
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals1, UE_ARRAY_COUNT(bNormals1));
								break;
							}
							case 2: {
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals2, UE_ARRAY_COUNT(bNormals2));
								break;
							}
							case 3: {
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals3, UE_ARRAY_COUNT(bNormals3));
								break;
							}
							case 4: {
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals5, UE_ARRAY_COUNT(bNormals4));
								break;
							}
							case 5: {
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));
								Vertices.Add(FVector(-_Chunk->VoxelSizeHalf + (x * _Chunk->VoxelSize), -_Chunk->VoxelSizeHalf + (y * _Chunk->VoxelSize), _Chunk->VoxelSizeHalf + (z * _Chunk->VoxelSize)));

								Normals.Append(bNormals4, UE_ARRAY_COUNT(bNormals5));
								break;
							}
							}

							UVs.Append(bUVs, UE_ARRAY_COUNT(bUVs));

							FColor color = FColor(255, 255, 255, i);
							VertexColors.Add(color); VertexColors.Add(color); VertexColors.Add(color); VertexColors.Add(color);
						}

					}
					element_num += triangle_num;
					_MeshSections[meshIndex].elementID += triangle_num;
				}

				else if (meshIndex == -1) {

					_GrassLocations.Add(FVector(x * _Chunk->VoxelSize, y * _Chunk->VoxelSize, z * _Chunk->VoxelSize)); //Get and transform the locations into world space

				}
				else if (meshIndex == -2) {

					_FlowerLocations.Add(FVector(x * _Chunk->VoxelSize, y * _Chunk->VoxelSize, z * _Chunk->VoxelSize)); //Get and transform the locations into world space

				}
			}
		}
	}

	

}


bool AVoxelActor::CheckGeneratedStatus() {
	if (_IsGenerated) {
		proceduralComponent->ClearAllMeshSections();

		for (int i = 0; i < _MeshSections.Num(); i++) {
			if (_MeshSections[i].Vertices.Num() > 0) {
				proceduralComponent->CreateMeshSection(i, _MeshSections[i].Vertices, _MeshSections[i].Triangles, _MeshSections[i].Normals, _MeshSections[i].UVs, _MeshSections[i].VertexColors, _MeshSections[i].Tangents, true);
			}
		}


		int s = 0;
		//Apply the materials. All the materials are defined in the blueprint.
		while (s < Materials.Num()) {
			proceduralComponent->SetMaterial(s, Materials[s]);
			s++;
		}

		_MeshSections.Empty();	//Need to empty mesh sections after the chunk has been created

		return true;
	}
	else {
		return false;
	}
}


TArray<int32> ExecuteOnOtherThread::calculateNoise(int currentBiome) {

	//Noise for the terrain generation
	TArray<int32> noises;
	noises.Reserve(_Chunk->chunkLineElementsExt * _Chunk->chunkLineElementsExt);

	SetupNoise();

	
	for (int32 y = -1; y <= _Chunk->chunkLineElements; y++) {
		for (int32 x = -1; x <= _Chunk->chunkLineElements; x++) {

			float noiseValue = _Chunk->_Noise->GetNoise2D((_Chunk->chunkXindex * _Chunk->chunkLineElements + x) * 0.01f, (_Chunk->chunkYindex * _Chunk->chunkLineElements + y) * 0.01f) * 4 +
				_Chunk->_Noise->GetNoise2D((_Chunk->chunkXindex * _Chunk->chunkLineElements + x) * 0.01f, (_Chunk->chunkYindex * _Chunk->chunkLineElements + y) * 0.01f) * 8 +
				_Chunk->_Noise->GetNoise2D((_Chunk->chunkXindex * _Chunk->chunkLineElements + x) * 0.004f, (_Chunk->chunkYindex * _Chunk->chunkLineElements + y) * 0.004f) * 16 +
				FMath::Clamp(_Chunk->_Noise->GetNoise2D((_Chunk->chunkXindex * _Chunk->chunkLineElements + x) * 0.05f, (_Chunk->chunkYindex * _Chunk->chunkLineElements + y) * 0.05f), 0.0f, 5.0f) * 4 +
				FMath::Clamp(_Chunk->_Noise->GetNoise2D((_Chunk->chunkXindex * _Chunk->chunkLineElements + x) * 0.07f, (_Chunk->chunkYindex * _Chunk->chunkLineElements + y) * 0.07f), 0.0f, 0.5f) * 2;

			noises.Add(FMath::FloorToInt(noiseValue));


		}
	}
			
		
	return noises;
}

void ExecuteOnOtherThread::DoWork() {
	_IsGenerated = false;

	GenerateChunk();
	UpdateMesh();

	_IsGenerated = true;

}

void AVoxelActor::StartGeneration() {
	//Start the parallel thread to do all of our chunk generations and calculations for the voxels
	(new FAutoDeleteAsyncTask<ExecuteOnOtherThread>(_IsGenerated, _MeshSections, _GrassLocations, _FlowerLocations, _FallingLeavesLocations ,this))->StartBackgroundTask();
}


bool ExecuteOnOtherThread::inRange(int32 value, int32 range) {
	return (value >= 0 && value < range);

}


void ExecuteOnOtherThread::SetupNoise() {
	if (IsValid(_Chunk->_Noise) && !_Chunk->_Noise->IsInitialized())
	{
		_Chunk->_Noise->SetupFastNoise(_Chunk->noise_settings.NoiseType, _Chunk->noise_settings.Seed, _Chunk->noise_settings.Frequency, _Chunk->noise_settings.Interp,
			_Chunk->noise_settings.FractalType, _Chunk->noise_settings.Octaves, _Chunk->noise_settings.Gain, _Chunk->noise_settings.Lacunarity,
			_Chunk->noise_settings.CellularJitter, _Chunk->noise_settings.CellularDistanceFunction, _Chunk->noise_settings.CellularReturnType);
	}
	else
	{
		_Chunk->_Noise = NewObject<UFastNoiseWrapper>();
		if (IsValid(_Chunk->_Noise) && !_Chunk->_Noise->IsInitialized())
			_Chunk->_Noise->SetupFastNoise(_Chunk->noise_settings.NoiseType, _Chunk->noise_settings.Seed, _Chunk->noise_settings.Frequency, _Chunk->noise_settings.Interp,
				_Chunk->noise_settings.FractalType, _Chunk->noise_settings.Octaves, _Chunk->noise_settings.Gain, _Chunk->noise_settings.Lacunarity,
				_Chunk->noise_settings.CellularJitter, _Chunk->noise_settings.CellularDistanceFunction, _Chunk->noise_settings.CellularReturnType);
		else
			UE_LOG(LogTemp, Warning, TEXT("Noise can't be initialized"));
	}
}

void ExecuteOnOtherThread::SetupBiomesNoise() {
	if (IsValid(_Chunk->_Noise2) && !_Chunk->_Noise2->IsInitialized())
	{
		_Chunk->_Noise2->SetupFastNoise(_Chunk->biomes_noise_settings.NoiseType, _Chunk->biomes_noise_settings.Seed, _Chunk->biomes_noise_settings.Frequency, _Chunk->biomes_noise_settings.Interp,
			_Chunk->biomes_noise_settings.FractalType, _Chunk->biomes_noise_settings.Octaves, _Chunk->biomes_noise_settings.Gain, _Chunk->biomes_noise_settings.Lacunarity,
			_Chunk->biomes_noise_settings.CellularJitter, _Chunk->biomes_noise_settings.CellularDistanceFunction, _Chunk->biomes_noise_settings.CellularReturnType);
	}
	else
	{
		_Chunk->_Noise2 = NewObject<UFastNoiseWrapper>();
		if (IsValid(_Chunk->_Noise2) && !_Chunk->_Noise2->IsInitialized())
			_Chunk->_Noise2->SetupFastNoise(_Chunk->biomes_noise_settings.NoiseType, _Chunk->biomes_noise_settings.Seed, _Chunk->biomes_noise_settings.Frequency, _Chunk->biomes_noise_settings.Interp,
				_Chunk->biomes_noise_settings.FractalType, _Chunk->biomes_noise_settings.Octaves, _Chunk->biomes_noise_settings.Gain, _Chunk->biomes_noise_settings.Lacunarity,
				_Chunk->biomes_noise_settings.CellularJitter, _Chunk->biomes_noise_settings.CellularDistanceFunction, _Chunk->biomes_noise_settings.CellularReturnType);
		else
			UE_LOG(LogTemp, Warning, TEXT("Biomes noise can't be initialized"));
	}

}

