/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedBuilder.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/UObjectToken.h"
#include "Misc/ScopedSlowTask.h"
#include "Logging/MessageLog.h"
#include "Kismet/GameplayStatics.h"
#include "ScopedTransaction.h"
#include "HierarchicalInstanced/HierarchicalInstancedActor.h"
#include "HierarchicalInstanced/HierarchicalInstancedSettings.h"
#include "HierarchicalInstancedVolume.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedBuilder"

struct FClusterPoint
{
	FClusterPoint()
		: Location()
		, bIsVisited(false)
		, PointType(EPointType::None)
		, ClusterID(INDEX_NONE)
		, Actor(nullptr)
	{}

	FClusterPoint(AStaticMeshActor* InActor) : FClusterPoint()
	{
		Actor = InActor;
		Location = Actor->GetActorLocation();
	}

	FVector Location;
	bool bIsVisited;
	EPointType PointType;
	int32 ClusterID;
	AStaticMeshActor* Actor;
};

class FDBScan
{
public:
	FDBScan() {}
	~FDBScan() {}

	void BuildCluster(TArray<FClusterPoint>& Points, float Epsilon, int32 MinPoints)
	{
		int32 ClusterID = 0;
		for (FClusterPoint& Point : Points)
		{
			if (Point.bIsVisited)
			{
				continue;
			}

			Point.bIsVisited = true;
			TArray<FClusterPoint*> NeighborPoints = RegionQuery(&Point, Points, Epsilon);
			if (NeighborPoints.Num() < MinPoints)
			{
				Point.PointType = EPointType::NoisePoint;
			}
			else
			{
				++ClusterID;
				Point.ClusterID = ClusterID;
				Point.PointType = EPointType::CorePoint;
				for (int32 NeighborIndex = 0; NeighborIndex < NeighborPoints.Num(); ++NeighborIndex)
				{
					FClusterPoint* NPoint = NeighborPoints[NeighborIndex];
					if (!NPoint->bIsVisited)
					{
						NPoint->bIsVisited = true;
						TArray<FClusterPoint*> NeighborPoints1 = RegionQuery(NPoint, Points, Epsilon);
						if (NeighborPoints1.Num() >= MinPoints)
						{
							NPoint->PointType = EPointType::CorePoint;
							NeighborPoints.Append(NeighborPoints1);
						}
						else
						{
							NPoint->PointType = EPointType::BorderPoint;
						}
					}
					if (NPoint->ClusterID == INDEX_NONE)
					{
						NPoint->ClusterID = ClusterID;
					}
				}
			}
		}
	}

private:
	TArray<FClusterPoint*> RegionQuery(const FClusterPoint* InPoint, TArray<FClusterPoint>& Points, int32 Epsilon)
	{
		TArray<FClusterPoint*> ResultPoints;
		for (FClusterPoint& Point : Points)
		{
			if ((InPoint->Location - Point.Location).Size() <= Epsilon)
			{
				ResultPoints.Add(&Point);
			}
		}
		return ResultPoints;
	}
};

FHierarchicalInstancedBuilder::FHierarchicalInstancedBuilder(class UInstancedClusterSettings* InSettings, UWorld* InWorld)
	: InstancedClusterSettings(InSettings)
	, World(InWorld)
{

}

void FHierarchicalInstancedBuilder::Build()
{
	FScopedSlowTask SlowTask(100.f, FText::FromString(TEXT("Build static mesh actors to instancing.")));
	SlowTask.MakeDialog();
	bool bVisibleLevelsWarning = false;
	TArray<ULevel*> Levels;
	if (InstancedClusterSettings->TargetLevelsType == ETargetLevelType::SelectedLevels)
	{
		Levels = World->GetSelectedLevels();
	}
	else
	{
		Levels = World->GetLevels();
	}

	for (ULevel* LevelIter : Levels)
	{
		// Only build clusters for levels that are visible, and throw warning if any are hidden
		if (LevelIter->bIsVisible)
		{
			BuildClusters(LevelIter);
		}
		bVisibleLevelsWarning |= !LevelIter->bIsVisible;
		SlowTask.EnterProgressFrame(100.f / Levels.Num());
	}

	if (bVisibleLevelsWarning)
	{
		FMessageLog MapCheck("HIResults");
		MapCheck.Warning()
			->AddToken(FUObjectToken::Create(InstancedClusterSettings))
			->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_NoBuildHIHiddenLevels", "Certain levels are marked as hidden, Hierarchical instanced will not be built for hidden levels.")));
	}
}

void FHierarchicalInstancedBuilder::RevertActorsToLevel()
{
	const TArray<ULevel*>& Levels = World->GetLevels();
	for (ULevel* LevelIter : Levels)
	{
		// Only build clusters for levels that are visible, and throw warning if any are hidden
		if (LevelIter->bIsVisible)
		{
			DeleteHIActors(LevelIter);
		}
	}
}

bool FHierarchicalInstancedBuilder::ShouldBuildToCluster(class AStaticMeshActor* InActor)
{
    if (InActor->bIsEditorOnlyActor ||
        InActor->IsHidden() ||
        InActor->bHiddenEd ||
        InActor->IsTemporarilyHiddenInEditor())
    {
        return false;
    }

    if (InActor->HasAnyFlags(RF_Transient))
    {
        return false;
    }

    if (InActor->IsTemplate())
    {
        return false;
    }

    if (InActor->IsPendingKill())
    {
        return false;
    }

	if (!InActor->GetStaticMeshComponent())
	{
		return false;
	}

	if (!InActor->GetStaticMeshComponent()->GetStaticMesh())
	{
		return false;
	}

    if (!InstancedClusterSettings->InstancedSettings.bSkipMeshesWithVertexColors && HasInstanceVertexColors(InActor->GetStaticMeshComponent()))
    {
        return false;
    }


    return true;
}

void FHierarchicalInstancedBuilder::DeleteHIActors(ULevel* InLevel)
{
	for (int32 ActorId = InLevel->Actors.Num() - 1; ActorId >= 0; --ActorId)
	{
		AHierarchicalInstancedActor* HIActor = Cast<AHierarchicalInstancedActor>(InLevel->Actors[ActorId]);
		if (HIActor)
		{
			const FScopedTransaction Transaction(LOCTEXT("UndoAction_DeleteLODActor", "Delete LOD Actor"));
			HIActor->RevertActorsToLevel();
			UWorld* CurrentWorld = HIActor->GetWorld();
			CurrentWorld->Modify();
			HIActor->Modify();
			CurrentWorld->DestroyActor(HIActor);
		}
	}
}

void FHierarchicalInstancedBuilder::BuildClusters(ULevel* InLevel)
{
	auto GenerateClustersForVolumes = [&](TArray<AStaticMeshActor*>& InOutActors)
	{
        TMap<AHierarchicalInstancedVolume*, TArray<AStaticMeshActor*>> HIVolumeActorsMapping;
        for (AHierarchicalInstancedVolume* Volume : HIVolumeActors)
        {
            FBox HIVolumeBox = Volume->GetComponentsBoundingBox(true);

            for (int32 ActorId = InOutActors.Num() - 1; ActorId >= 0; --ActorId)
            {
                AStaticMeshActor* StaticMeshActor = InOutActors[ActorId];
                FBox ActorBox = StaticMeshActor->GetComponentsBoundingBox(true);
                if (HIVolumeBox.IsInside(ActorBox) || (Volume->bIncludeOverlappingActors && HIVolumeBox.Intersect(ActorBox)))
                {
                    HIVolumeActorsMapping.FindOrAdd(Volume).Add(StaticMeshActor);
                    InOutActors.Remove(StaticMeshActor);
                }
            }
        }

        for (auto& VolumePair : HIVolumeActorsMapping)
        {
            if (VolumePair.Value.Num() > InstancedClusterSettings->InstancedSettings.MinNumberOfActorsToInstanced)
            {
                VolumePair.Key->HIActor = SpawnHierarchicalInstancedActor(InLevel, VolumePair.Value);
            }
        }
	};

    auto GenerateClustersForDBScan = [&](TArray<AStaticMeshActor*>& InOutActors)
    {
        TArray<FClusterPoint> Points;
        for (AStaticMeshActor* StaticMeshActor : InOutActors)
        {
            Points.Add(FClusterPoint(StaticMeshActor));
        }

        FDBScan DBScan;
        DBScan.BuildCluster(Points, InstancedClusterSettings->InstancedSettings.ActorDensityDistance, InstancedClusterSettings->InstancedSettings.MinNumberOfActorsToInstanced);

        TMap<int32, TArray<int32> > ClusterPoints;
        for (int32 Index = 0; Index < Points.Num(); ++Index)
        {
            FClusterPoint& Point = Points[Index];
            TArray<int32>& ClassifiedPoints = ClusterPoints.FindOrAdd(Point.ClusterID);
            ClassifiedPoints.Add(Index);
        }

        for (auto& ClusterPair : ClusterPoints)
        {
            if (ClusterPair.Key != INDEX_NONE && ClusterPair.Value.Num() >= InstancedClusterSettings->InstancedSettings.MinNumberOfActorsToInstanced)
            {
                TArray<AStaticMeshActor*> ReplacementActors;
                for (int32 PointIndex : ClusterPair.Value)
                {
                    ReplacementActors.Add(Points[PointIndex].Actor);
                }
                SpawnHierarchicalInstancedActor(InLevel, ReplacementActors);
            }
        }
    };

    HIVolumeActors.Empty();
	UWorld* CurrentWorld = InLevel->GetWorld();
	FScopedSlowTask SlowTask(100.f, FText::FromString(TEXT("Build instance clusters.")));
	SlowTask.MakeDialog();
	DeleteHIActors(InLevel);
	SlowTask.EnterProgressFrame(10);
	TMap<FInstanceComponentsEntry, TArray<AStaticMeshActor*> > ClassifiedActors;
	for (AActor* Actor : InLevel->Actors)
	{
		if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
		{
			if(ShouldBuildToCluster(StaticMeshActor))
			{
				FInstanceComponentsEntry ComponentEntry(StaticMeshActor->GetStaticMeshComponent());
				TArray<AStaticMeshActor*>& Actors = ClassifiedActors.FindOrAdd(ComponentEntry);
				Actors.Add(StaticMeshActor);
			}
		}
		else if (AHierarchicalInstancedVolume* VolumeActor = Cast<AHierarchicalInstancedVolume>(Actor))
		{
            HIVolumeActors.Add(VolumeActor);
		}
	}
	SlowTask.EnterProgressFrame(10);
	for (auto& ClassifiedPiar : ClassifiedActors)
	{
		SlowTask.EnterProgressFrame(80.0f/ ClassifiedActors.Num());

		if (ClassifiedPiar.Value.Num() >= InstancedClusterSettings->InstancedSettings.MinNumberOfActorsToInstanced)
		{
            if (InstancedClusterSettings->bOnlyGenerateClustersForVolumes)
            {
                GenerateClustersForVolumes(ClassifiedPiar.Value);
            }
            else
            {
                GenerateClustersForVolumes(ClassifiedPiar.Value);
                GenerateClustersForDBScan(ClassifiedPiar.Value);
            }
		}
	}
}

bool FHierarchicalInstancedBuilder::HasInstanceVertexColors(UStaticMeshComponent* StaticMeshComponent)
{
	for (const FStaticMeshComponentLODInfo& CurrentLODInfo : StaticMeshComponent->LODData)
	{
		if (CurrentLODInfo.OverrideVertexColors != nullptr || CurrentLODInfo.PaintedVertices.Num() > 0)
		{
			return true;
		}
	}
	return false;
}

AHierarchicalInstancedActor* FHierarchicalInstancedBuilder::SpawnHierarchicalInstancedActor(class ULevel* InLevel, const TArray<class AStaticMeshActor*> InActors)
{
    FActorSpawnParameters ActorSpawnParameters;
    ActorSpawnParameters.OverrideLevel = InLevel;
    AHierarchicalInstancedActor* HierarchicalInstancedActor = InLevel->GetWorld()->SpawnActor<AHierarchicalInstancedActor>(AHierarchicalInstancedActor::StaticClass(), FTransform::Identity, ActorSpawnParameters);
    HierarchicalInstancedActor->SetInstancedSettings(InstancedClusterSettings->InstancedSettings);
    for (AStaticMeshActor* StaticMeshActor : InActors)
    {
        HierarchicalInstancedActor->AddReplacementActor(StaticMeshActor);
    }
    HierarchicalInstancedActor->UpdateInstances();
    HierarchicalInstancedActor->SetFolderPath(TEXT("HierarchicalInstanced"));
	HierarchicalInstancedActor->PostEditChange();
    HierarchicalInstancedActor->InvalidateLightingCache();
    HierarchicalInstancedActor->MarkPackageDirty();
    InLevel->GetWorld()->UpdateCullDistanceVolumes(HierarchicalInstancedActor);
    return HierarchicalInstancedActor;
}

#undef LOCTEXT_NAMESPACE


