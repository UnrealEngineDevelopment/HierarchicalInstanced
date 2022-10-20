/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"

class ULevel;
class UWorld;
class UStaticMeshComponent;

class FHierarchicalInstancedBuilder
{
public:
	FHierarchicalInstancedBuilder(class UInstancedClusterSettings* InSettings, UWorld* InWorld);
	void Build();
	void RevertActorsToLevel();
    bool ShouldBuildToCluster(class AStaticMeshActor* InActor);
    class AHierarchicalInstancedActor* SpawnHierarchicalInstancedActor(class ULevel* InLevel, const TArray<class AStaticMeshActor*> InActors);
private:
	void DeleteHIActors(ULevel* InLevel);
	void BuildClusters(ULevel* InLevel);
	bool HasInstanceVertexColors(UStaticMeshComponent* StaticMeshComponent);
private:
    class UInstancedClusterSettings*            InstancedClusterSettings;
	/** Owning world HIActors are created for */
	UWorld*	                                    World;
    TArray<class AHierarchicalInstancedVolume*> HIVolumeActors;
};