/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "HierarchicalInstancedSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "HierarchicalInstancedActor.generated.h"

class AStaticMeshActor;

UCLASS(BlueprintType, Blueprintable, hidecategories = (Object, Collision, Display, Input, Blueprint, Transform, Physics))
class HIERARCHICALINSTANCED_API AHierarchicalInstancedActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Instancing")
	UHierarchicalInstancedStaticMeshComponent* HISMComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Instancing")
	FHierarchicalInstancedSettings InstancedSettings;

	UPROPERTY(EditAnywhere, Category = "Instancing")
	TArray<AStaticMeshActor*>      ReplacementActors;

	UPROPERTY(VisibleAnywhere, Category = "Instancing")
	TMap<int32, FCustomPrimitiveData> CustomPrimitiveDatas;
#endif // WITH_EDITORONLY_DATA

	UHierarchicalInstancedStaticMeshComponent* GetHISMComponent()const;
	int32 GetInstanceCount();

#if WITH_EDITOR
	//~ Begin UObject Interface.
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished);
	//~ End UObject Interface.
	float GetPrimitiveMaxDrawDistances(UPrimitiveComponent* PrimitiveComponent);
	void SetInstancedSettings(const FHierarchicalInstancedSettings& InSettings);
	bool AddReplacementActor(AStaticMeshActor* InActor);
	bool RemoveReplacementActor(AStaticMeshActor* InActor);
	bool HasAnyReplacementActors() const;
	bool UpdateInstances();
	void RevertActorsToLevel();
#endif // WITH_EDITOR
};

/** Helper struct representing a spawned ISMC */
struct FInstanceComponentsEntry
{
	FInstanceComponentsEntry(UStaticMeshComponent* InComponent)
	{
		InComponent->GetUsedMaterials(Materials);
		StaticMesh           = InComponent->GetStaticMesh();
		bReverseCulling      = InComponent->GetComponentTransform().ToMatrixWithScale().Determinant() < 0.0f;
		CollisionProfileName = InComponent->GetCollisionProfileName();
		CollisionEnabled     = InComponent->GetCollisionEnabled();
		NumCustomDataFloats  = InComponent->GetCustomPrimitiveData().Data.Num();
		Add(InComponent);
	}

	bool operator==(const FInstanceComponentsEntry& InOther) const
	{
		return StaticMesh           == InOther.StaticMesh &&
			   Materials            == InOther.Materials &&
			   bReverseCulling      == InOther.bReverseCulling &&
			   NumCustomDataFloats  == NumCustomDataFloats &&
			   CollisionProfileName == InOther.CollisionProfileName &&
			   CollisionEnabled     == InOther.CollisionEnabled;
	}

	bool operator!=(const FInstanceComponentsEntry& InOther) const
	{
		return !(*this == InOther);
	}

	void Add(UStaticMeshComponent* InComponent)
	{
		const FCustomPrimitiveData& CustomPrimitiveData = InComponent->GetCustomPrimitiveData();
		if (!OriginalComponents.Contains(InComponent) && CustomPrimitiveData.Data.Num() == NumCustomDataFloats)
		{
			OriginalComponents.Add(InComponent);
		}
	}

	friend uint32 GetTypeHash(const FInstanceComponentsEntry& This)
	{
		uint32 KeyHash = GetTypeHash(This.StaticMesh);
		for (UMaterialInterface* Material : This.Materials)
		{
			KeyHash = HashCombine(KeyHash, PointerHash(Material));
		}
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.bReverseCulling));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.CollisionProfileName));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.CollisionEnabled));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.NumCustomDataFloats));
		return KeyHash;
	}

	UStaticMesh*                  StaticMesh;
	TArray<UMaterialInterface*>   Materials;
	bool                          bReverseCulling;
	FName                         CollisionProfileName;
	ECollisionEnabled::Type       CollisionEnabled;
	TArray<UStaticMeshComponent*> OriginalComponents;
	int32                         NumCustomDataFloats;
};