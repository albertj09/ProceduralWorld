// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldManagerCoded.h"

// Sets default values
AWorldManagerCoded::AWorldManagerCoded()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AWorldManagerCoded::Initialize() {
	chunkSize = chunkElementsXY * voxelSize;
	chunkSizeHalf = chunkSize / 1;	
}

bool AWorldManagerCoded::CheckRadius(float x, float y) {

	if ((FVector(x, y, 0.0f) - characterPos).Size() < (float)(chunkSize * renderRange)) {
		return true;
	}
	else {
		return false;
	}
}

bool AWorldManagerCoded::UpdatePosition() {
	characterPos = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() * FVector(1.0f, 1.0f, 0);

	if (floorf(characterPos.X / (float)chunkSizeHalf) != chunkX || floorf(characterPos.Y / (float)chunkSizeHalf) != chunkY) {
		chunkX = floorf(characterPos.X / (float)chunkSizeHalf);
		chunkY = floorf(characterPos.Y / (float)chunkSizeHalf);
		return true;
	}
	else {
		return false;
	}

}

void AWorldManagerCoded::AddChunk(FSpawnObjectFunction SpawnChunkFunction) {
	
	for (int32 x = renderRange * -1; x < renderRange; x++) {		
		for (int32 y = renderRange * -1; y < renderRange; y++) {	
			chunkXindex = x + chunkX;
			chunkYindex = y + chunkY;
			if (CheckRadius((float)(chunkXindex * chunkSize) + (float)chunkSizeHalf, (float)(chunkYindex * chunkSize) + (float)chunkSizeHalf)) {
				if (chunkCoordinates.Contains(FVector2D((float)(x + chunkX), (float)(y + chunkY)))){
					//Do nothing
				}
				else {
					chunkCoordinates.Add(FVector2D(FVector2D((float)(x + chunkX), (float)(y + chunkY))));
					
					xTransform = chunkXindex * chunkSize;
					yTransform = chunkYindex * chunkSize;
					SpawnChunkFunction.Execute();
					
				}
			}
		}
	}
}



void AWorldManagerCoded::RemoveChunk() {

	if (chunkCoordinates.Num() != 0) {
		for (int i = 0; i < chunkCoordinates.Num(); i++) {
			if (CheckRadius(chunkCoordinates[i].X * chunkSize + chunkSizeHalf, chunkCoordinates[i].Y * chunkSize + chunkSizeHalf)) {
				//Do nothing
			}
			else {

				_ChunkList[i]->K2_DestroyActor();
				_ChunkList.RemoveAt(i);
				_WaterPlaneList[i]->K2_DestroyActor();
				_WaterPlaneList.RemoveAt(i);
				chunkCoordinates.RemoveAt(i);
				
			}
		}
	}
}

void AWorldManagerCoded::ManageChunksQueue() {
	for (int32 index = 0; index < _WaitingChunkList.Num(); index++) {
		if (IsValid(_WaitingChunkList[index])) {
			if (_WaitingChunkList[index]->CheckGeneratedStatus()) {
				_WaitingChunkList.RemoveAt(index);
			}
			else {

			}
		}
		else {
			_WaitingChunkList.RemoveAt(index);
			_ChunkList.RemoveAt(index);
		}
	}
	RemoveChunk();	
}





