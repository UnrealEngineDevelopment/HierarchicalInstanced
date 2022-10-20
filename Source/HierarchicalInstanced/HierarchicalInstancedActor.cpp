/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/

#include "HierarchicalInstancedActor.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/CollisionProfile.h"
#include "UObject/FrameworkObjectVersion.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "PhysicsEngine/BodySetup.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedActor"

AHierarchicalInstancedActor::AHierarchicalInstancedActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HISMComponent(nullptr)
{
	SetCanBeDamaged(false);
	HISMComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISMComponent_0"));
	HISMComponent->Mobility = EComponentMobility::Static;
	HISMComponent->SetGenerateOverlapEvents(false);
	HISMComponent->bUseDefaultCollision = true;
	RootComponent = HISMComponent;
}

UHierarchicalInstancedStaticMeshComponent* AHierarchicalInstancedActor::GetHISMComponent() const
{
	return HISMComponent;
}

int32 AHierarchicalInstancedActor::GetInstanceCount()
{
	return HISMComponent->GetInstanceCount();
}

#if WITH_EDITOR

void AHierarchicalInstancedActor::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);
}

void AHierarchicalInstancedActor::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	static const FName CullDistancesPropertyName = GET_MEMBER_NAME_CHECKED(FHierarchicalInstancedSettings, CullDistances);
	if (PropertyChangedEvent.PropertyChain.Num() > 0)
	{
		FProperty* MemberProperty = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue();
		if (MemberProperty != NULL)
		{
			FName PropertyName = PropertyChangedEvent.Property->GetFName();
			if (PropertyName == CullDistancesPropertyName && ReplacementActors.Num() > 0)
			{
				float CullDistance = GetPrimitiveMaxDrawDistances(ReplacementActors[0]->GetStaticMeshComponent());
				HISMComponent->SetCullDistances(CullDistance*0.8, CullDistance);
			}
		}
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void AHierarchicalInstancedActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	if (ReplacementActors.Num() > 0)
	{
		if (ReplacementActors[0] == nullptr || ReplacementActors[0]->GetLevel() != this->GetLevel())
		{
			ReplacementActors.Empty();
			return;
		}
	}
}

float AHierarchicalInstancedActor::GetPrimitiveMaxDrawDistances(UPrimitiveComponent* PrimitiveComponent)
{
	float CurrentError = FLT_MAX;
	float CurrentCullDistance = 0.f;
	const float PrimitiveSize = PrimitiveComponent->Bounds.SphereRadius * 2;
	if (InstancedSettings.CullType == EHICullType::Distance)
	{
		for (const FCullDistanceSizePair& CullDistancePair : InstancedSettings.CullDistances)
		{
			const float Error = FMath::Abs(PrimitiveSize - CullDistancePair.Size);
			if (Error < CurrentError)
			{
				CurrentError = Error;
				CurrentCullDistance = CullDistancePair.CullDistance;
			}
		}
	}
	else
	{
		static const float FOVRad = 90.0f * (float)PI / 360.0f;
		static const FMatrix ProjectionMatrix = FPerspectiveMatrix(FOVRad, 1920, 1080, 0.01f);
		CurrentCullDistance = ::ComputeBoundsDrawDistance(InstancedSettings.CullScreenSize, PrimitiveComponent->Bounds.SphereRadius, ProjectionMatrix);
	}
	return CurrentCullDistance;
}


void AHierarchicalInstancedActor::SetInstancedSettings(const FHierarchicalInstancedSettings& InSettings)
{
	InstancedSettings = InSettings;
}

bool AHierarchicalInstancedActor::AddReplacementActor(AStaticMeshActor * InActor)
{
	if ((InActor != nullptr) && !ReplacementActors.Contains(InActor))
	{
		ReplacementActors.Add(InActor);
		return true;
	}
	return false;
}

bool AHierarchicalInstancedActor::RemoveReplacementActor(AStaticMeshActor* InActor)
{
	if ((InActor != nullptr) && ReplacementActors.Contains(InActor))
	{
		ReplacementActors.Remove(InActor);
		return true;
	}
	return false;
}

bool AHierarchicalInstancedActor::HasAnyReplacementActors() const
{
	return (ReplacementActors.Num() != 0);
}

bool AHierarchicalInstancedActor::UpdateInstances()
{
	bool bSucceeded = false;
	if (ReplacementActors.Num() >= InstancedSettings.MinNumberOfActorsToInstanced && ReplacementActors[0])
	{
		TArray<AActor*> ActorsToBounds;
		FInstanceComponentsEntry ComponentEntry(ReplacementActors[0]->GetStaticMeshComponent());
		for (AStaticMeshActor* StaticMeshActor : ReplacementActors)
		{
			if (StaticMeshActor && !StaticMeshActor->IsPendingKill())
			{
				ActorsToBounds.Add(StaticMeshActor);
				ComponentEntry.Add(StaticMeshActor->GetStaticMeshComponent());
			}
		}

		if (ActorsToBounds.Num() > 0)
		{
			FVector Center;
			FVector Extent;
			UGameplayStatics::GetActorArrayBounds(ActorsToBounds, true, Center, Extent);
			this->SetActorLocation(Center);

			CustomPrimitiveDatas.Empty();
			HISMComponent->ClearInstances();
			HISMComponent->bAutoRebuildTreeOnInstanceChanges = false;
			HISMComponent->bHasPerInstanceHitProxies = true;
			// Make a new root if we dont have a root already
			this->SetRootComponent(HISMComponent);
			// Take 'instanced' ownership so it persists with this actor
			this->RemoveOwnedComponent(HISMComponent);
			HISMComponent->CreationMethod = EComponentCreationMethod::Instance;
			this->AddOwnedComponent(HISMComponent);
			HISMComponent->SetStaticMesh(ComponentEntry.StaticMesh);
			for (int32 MaterialIndex = 0; MaterialIndex < ComponentEntry.Materials.Num(); ++MaterialIndex)
			{
				HISMComponent->SetMaterial(MaterialIndex, ComponentEntry.Materials[MaterialIndex]);
			}
			HISMComponent->SetReverseCulling(ComponentEntry.bReverseCulling);
			HISMComponent->SetCollisionProfileName(ComponentEntry.CollisionProfileName);
			HISMComponent->SetCollisionEnabled(ComponentEntry.CollisionEnabled);
			HISMComponent->SetMobility(EComponentMobility::Static);
			if (UBodySetup* BodySetup = HISMComponent->GetBodySetup())
			{
				HISMComponent->bUseDefaultCollision = (BodySetup->DefaultInstance.GetCollisionProfileName() == HISMComponent->GetCollisionProfileName());
			}
			HISMComponent->NumCustomDataFloats = ComponentEntry.NumCustomDataFloats;
			for (UStaticMeshComponent* OriginalComponent : ComponentEntry.OriginalComponents)
			{
				FTransform NewInstanceTransform = OriginalComponent->GetComponentTransform();
				int32 InstanceIndex = HISMComponent->AddInstance(NewInstanceTransform.GetRelativeTransform(this->GetActorTransform()));
				if (ComponentEntry.NumCustomDataFloats > 0)
				{
					const FCustomPrimitiveData& CustomPrimitiveData = OriginalComponent->GetCustomPrimitiveData();
					check(CustomPrimitiveData.Data.Num() == ComponentEntry.NumCustomDataFloats);
					HISMComponent->SetCustomData(InstanceIndex, CustomPrimitiveData.Data);
					CustomPrimitiveDatas.Add(InstanceIndex, CustomPrimitiveData);
				}
			}
			HISMComponent->BuildTreeIfOutdated(true, false);
			float CullDistance = GetPrimitiveMaxDrawDistances(ReplacementActors[0]->GetStaticMeshComponent());
			HISMComponent->SetCullDistances(CullDistance*0.8, CullDistance);
			HISMComponent->MarkRenderStateDirty();
			// Now clean up our original actors
			for (AActor* ActorToCleanUp : ReplacementActors)
			{
				if (ActorToCleanUp)
				{
					if (InstancedSettings.MeshReplacementMethod == EHierarchicalInstancedReplacementMethod::RemoveOriginalActors)
					{
						ActorToCleanUp->Destroy();
					}
					else if (InstancedSettings.MeshReplacementMethod == EHierarchicalInstancedReplacementMethod::KeepOriginalActorsAsEditorOnly)
					{
						ActorToCleanUp->Modify();
						ActorToCleanUp->bIsEditorOnlyActor = true;
						ActorToCleanUp->SetHidden(true);
						ActorToCleanUp->bHiddenEd = true;
						ActorToCleanUp->SetIsTemporarilyHiddenInEditor(true);
						ActorToCleanUp->SetFolderPath(TEXT("HierarchicalInstanced_Backup"));
					}
				}
			}
		}
	}
	return bSucceeded;
}

void AHierarchicalInstancedActor::RevertActorsToLevel()
{
	if (ReplacementActors.Num() > 0 && InstancedSettings.MeshReplacementMethod == EHierarchicalInstancedReplacementMethod::KeepOriginalActorsAsEditorOnly)
	{
		for (AActor* ActorToRevert : ReplacementActors)
		{
			if (ActorToRevert)
			{
				ActorToRevert->Modify();
				ActorToRevert->bIsEditorOnlyActor = false;
				ActorToRevert->SetHidden(false);
				ActorToRevert->bHiddenEd = false;
				ActorToRevert->SetIsTemporarilyHiddenInEditor(false);
				ActorToRevert->SetFolderPath(TEXT(""));
			}
		}
	}
	else
	{
		ReplacementActors.Empty();
		for (int InstanceIndex = 0; InstanceIndex < HISMComponent->GetInstanceCount(); ++InstanceIndex)
		{
			FTransform OutTransform;
			if (HISMComponent->GetInstanceTransform(InstanceIndex, OutTransform, true))
			{
				FActorSpawnParameters ActorSpawnParameters;
				ActorSpawnParameters.OverrideLevel = this->GetLevel();
				AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), OutTransform, ActorSpawnParameters);
				UStaticMeshComponent* StaticMeshComponent = Actor->GetStaticMeshComponent();
				StaticMeshComponent->SetStaticMesh(HISMComponent->GetStaticMesh());
				TArray<UMaterialInterface*> Materials;
				HISMComponent->GetUsedMaterials(Materials);
				for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); ++MaterialIndex)
				{
					StaticMeshComponent->SetMaterial(MaterialIndex, Materials[MaterialIndex]);
				}
				StaticMeshComponent->SetReverseCulling(HISMComponent->bReverseCulling);
				StaticMeshComponent->SetCollisionProfileName(HISMComponent->GetCollisionProfileName());
				StaticMeshComponent->SetCollisionEnabled(HISMComponent->GetCollisionEnabled());
				StaticMeshComponent->SetMobility(HISMComponent->Mobility);
				if (UBodySetup* BodySetup = StaticMeshComponent->GetBodySetup())
				{
					StaticMeshComponent->bUseDefaultCollision = (BodySetup->DefaultInstance.GetCollisionProfileName() == StaticMeshComponent->GetCollisionProfileName());
				}
				float CullDistance = GetPrimitiveMaxDrawDistances(StaticMeshComponent);
				StaticMeshComponent->LDMaxDrawDistance = CullDistance;
				StaticMeshComponent->bAllowCullDistanceVolume = true;
				StaticMeshComponent->bNeverDistanceCull = false;

				if (CustomPrimitiveDatas.Contains(InstanceIndex))
				{
					FCustomPrimitiveData& CustomPrimitiveData = CustomPrimitiveDatas[InstanceIndex];
					for (int32 DataIndex = 0; DataIndex < CustomPrimitiveData.Data.Num(); ++DataIndex)
					{
						StaticMeshComponent->SetDefaultCustomPrimitiveDataFloat(DataIndex, CustomPrimitiveData.Data[DataIndex]);
					}
				}

				FName ActorName = MakeUniqueObjectName(this->GetOuter(), AStaticMeshActor::StaticClass(), *(HISMComponent->GetStaticMesh()->GetName()) );
				Actor->SetActorLabel(ActorName.ToString());
				Actor->InvalidateLightingCache();
				Actor->PostEditChange();
				Actor->MarkPackageDirty(); 
				AddReplacementActor(Actor);
				GetWorld()->UpdateCullDistanceVolumes(Actor);
			}
		}
	}
	HISMComponent->ClearInstances();
	ULevel::LevelDirtiedEvent.Broadcast();
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE