// Copyright Epic Games, Inc. All Rights Reserved.

#include "HierarchicalInstancedVolume.h"
#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMeshActor.h"
#include "HierarchicalInstancedBuilder.h"
#include "HierarchicalInstanced/HierarchicalInstancedActor.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"

AHierarchicalInstancedVolume::AHierarchicalInstancedVolume(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIncludeOverlappingActors(false)
{
    GetBrushComponent()->SetGenerateOverlapEvents(false);
    bNotForClientOrServer = true;
    bIsEditorOnlyActor = true;
    bColored = true;
    BrushColor.R = 255;
    BrushColor.G = 100;
    BrushColor.B = 255;
    BrushColor.A = 255;

#if WITH_EDITORONLY_DATA
    SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
    ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

    if (!IsRunningCommandlet())
    {
        // Structure to hold one-time initialization
        struct FConstructorStatics
        {
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> TargetIconSpawnObject;
            ConstructorHelpers::FObjectFinderOptional<UTexture2D> TargetIconObject;
            FName ID_HIVolume;
            FText NAME_HIVolume;
            FConstructorStatics()
                : TargetIconSpawnObject(TEXT("/Engine/EditorMaterials/T_1x1_Grid"))
                , TargetIconObject(TEXT("/Engine/EditorMaterials/T_1x1_Grid"))
                , ID_HIVolume(TEXT("HIVolume"))
                , NAME_HIVolume(FText::FromString(TEXT("HI Volume")) )
            {
            }
        };
        static FConstructorStatics ConstructorStatics;

        if (SpriteComponent)
        {
            SpriteComponent->Sprite = ConstructorStatics.TargetIconObject.Get();
            SpriteComponent->SetRelativeScale3D_Direct(FVector(0.35f, 0.35f, 0.35f));
            SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_HIVolume;
            SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_HIVolume;
            SpriteComponent->bIsScreenSizeScaled = true;

            SpriteComponent->SetupAttachment(GetBrushComponent());
        }

        if (ArrowComponent)
        {
            ArrowComponent->ArrowColor = FColor(150, 200, 255);

            ArrowComponent->ArrowSize = 0.5f;
            ArrowComponent->bTreatAsASprite = true;
            ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_HIVolume;
            ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_HIVolume;
            ArrowComponent->SetupAttachment(SpriteComponent);
            ArrowComponent->bIsScreenSizeScaled = true;

            // Counteract the scaled down parent so that the arrow is large enough to see.
            if (SpriteComponent)
            {
                ArrowComponent->SetRelativeScale3D((FVector::OneVector / SpriteComponent->GetRelativeScale3D()));
            }
        }
    }
#endif // WITH_EDITORONLY_DATA
}

void AHierarchicalInstancedVolume::BuildHIActor()
{
    if (HIActor)
    {
        HIActor->RevertActorsToLevel();
        HIActor->GetWorld()->Modify();
        HIActor->Modify();
        HIActor->GetWorld()->DestroyActor(HIActor);
        HIActor = nullptr;
    }
    UInstancedClusterSettings* InstancedClusterSettings = NewObject<UInstancedClusterSettings>();
    FHierarchicalInstancedBuilder Builder(InstancedClusterSettings, GetWorld());
    FBox HIVolumeBox = this->GetComponentsBoundingBox(true);
    TArray<AStaticMeshActor*> IncludedActors;
    ULevel* MyLevel = GetLevel();
    for (AActor* Actor : MyLevel->Actors)
    {
        if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (Builder.ShouldBuildToCluster(StaticMeshActor))
            {
                FBox ActorBox = StaticMeshActor->GetComponentsBoundingBox(true);
                if (HIVolumeBox.IsInside(ActorBox) || (this->bIncludeOverlappingActors && HIVolumeBox.Intersect(ActorBox)))
                {
                    IncludedActors.Add(StaticMeshActor);
                }
            }
        }
    }
    HIActor = Builder.SpawnHierarchicalInstancedActor(MyLevel, IncludedActors);
}




#if WITH_EDITORONLY_DATA
/** Returns SpriteComponent subobject **/
UBillboardComponent* AHierarchicalInstancedVolume::GetSpriteComponent() const { return SpriteComponent; }
/** Returns ArrowComponent subobject **/
UArrowComponent* AHierarchicalInstancedVolume::GetArrowComponent() const { return ArrowComponent; }
#endif