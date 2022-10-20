/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/CullDistanceVolume.h"

#include "HierarchicalInstancedSettings.generated.h"

/** How to replace instanced */
UENUM()
enum class EHierarchicalInstancedReplacementMethod : uint8
{
	/** Destructive workflow: remove the original actors when replacing with instanced static meshes */
	RemoveOriginalActors,

	/** Non-destructive workflow: keep the original actors but hide them and set them to be editor-only */
	KeepOriginalActorsAsEditorOnly
};

UENUM()
enum class EHICullType : uint8
{
	Distance,
	ScreenSize,
};

/** Mesh instance-replacement settings */
USTRUCT(Blueprintable)
struct FHierarchicalInstancedSettings
{
	GENERATED_BODY()

	FHierarchicalInstancedSettings()
	: ActorDensityDistance(2000.0f)
    , MinNumberOfActorsToInstanced(3)
	, MeshReplacementMethod(EHierarchicalInstancedReplacementMethod::KeepOriginalActorsAsEditorOnly)
	, bSkipMeshesWithVertexColors(true)
	, CullType(EHICullType::Distance)
	, CullScreenSize(0.15f)
	{
		CullDistances.Add(FCullDistanceSizePair(64, 2000));
		CullDistances.Add(FCullDistanceSizePair(128, 3500));
		CullDistances.Add(FCullDistanceSizePair(192, 5000));
		CullDistances.Add(FCullDistanceSizePair(256, 6500));
		CullDistances.Add(FCullDistanceSizePair(384, 8000));
		CullDistances.Add(FCullDistanceSizePair(512, 9500));
		CullDistances.Add(FCullDistanceSizePair(768, 11000));
		CullDistances.Add(FCullDistanceSizePair(1024, 12500));
		CullDistances.Add(FCullDistanceSizePair(1536, 14000));
		CullDistances.Add(FCullDistanceSizePair(2048, 0));
	}

    /** */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float ActorDensityDistance;

	/** The number of static mesh instances needed before a mesh is replaced with an instanced version */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = 2))
	int32 MinNumberOfActorsToInstanced;

	/** How to replace the original actors when instancing */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EHierarchicalInstancedReplacementMethod MeshReplacementMethod;

	/**
	 * Whether to skip the conversion to an instanced static mesh for meshes with vertex colors.
	 * Instanced static meshes do not support vertex colors per-instance, so conversion will lose
	 * this data.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bSkipMeshesWithVertexColors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EHICullType CullType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CullScreenSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<struct FCullDistanceSizePair> CullDistances;
};

UENUM()
enum class EPointType : uint8
{
	None,
	// This is a point that has at least m points within distance n from itself.
	NoisePoint,

	// This is a point that has at least one Core point at a distance n.
	BorderPoint,

	// This is a point that is neither a Core nor a Border. And it has less than m points within distance n from itself.
	CorePoint
};

UENUM()
enum class ETargetLevelType : uint8
{
	AllLevels,
	SelectedLevels,
};

UCLASS(Blueprintable)
class HIERARCHICALINSTANCED_API UInstancedClusterSettings: public UObject
{
	GENERATED_BODY()
public:
	UInstancedClusterSettings()
		: bOnlyGenerateClustersForVolumes(false)
        , TargetLevelsType(ETargetLevelType::AllLevels)
	{
	}

    /** Only generate clusters for HI volumes */
    UPROPERTY(EditAnywhere, Category = "Instancing")
    uint8 bOnlyGenerateClustersForVolumes : 1;

	UPROPERTY(EditAnywhere, Category = "Instancing")
	ETargetLevelType TargetLevelsType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Instancing")
	FHierarchicalInstancedSettings InstancedSettings;
};
