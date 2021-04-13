// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Multithreaded.h"
#include "Async/AsyncWork.h"
#include "MultithreadedLibrary.generated.h"


/**
 * 
 */

class MultithreadedTask : public FNonAbandonableTask {
	
public:
	MultithreadedTask(UObject* object) { this->object = object; }

	UObject* object;

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(MultithreadedTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork() {
		IMultithreaded::Execute_MultithreadedFunction(object);
	}
};


UCLASS()
class PROCEDURALWORLD_API UMultithreadedLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BLueprintCallable)
		static void CallMultithreadedFunction(UObject* object) {
		(new FAutoDeleteAsyncTask<MultithreadedTask>(object))->StartBackgroundTask();
	}
		
};
