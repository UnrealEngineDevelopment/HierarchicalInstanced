// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Volume.h"
#include "HierarchicalInstanced/HierarchicalInstancedSettings.h"
#include "HierarchicalInstancedVolume.generated.h"

/** An invisible volume used to manually define/create a HLOD cluster. */
UCLASS(MinimalAPI, hidecategories = (Navigation, Collision, Advanced, Attachment))
class AHierarchicalInstancedVolume : public AVolume
{
    GENERATED_UCLASS_BODY()
public:
    /** When set this volume will incorporate actors which bounds overlap with the volume, otherwise only actors which are completely inside of the volume are incorporated */
    UPROPERTY(EditAnywhere, Category = "HI Volume")
    bool bIncludeOverlappingActors;

    UPROPERTY(EditAnywhere, Category = "HI Volume")
    FHierarchicalInstancedSettings InstancingSettings;

    UPROPERTY(VisibleAnywhere, Category = "HI Volume")
    class AHierarchicalInstancedActor* HIActor;

    void BuildHIActor();

#if WITH_EDITORONLY_DATA
private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Display, meta = (AllowPrivateAccess = "true"))
        class UBillboardComponent* SpriteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Display, meta = (AllowPrivateAccess = "true"))
        class UArrowComponent* ArrowComponent;

public:
    /** Returns SpriteComponent subobject **/
    class UBillboardComponent* GetSpriteComponent() const;
    /** Returns ArrowComponent subobject **/
    class UArrowComponent* GetArrowComponent() const;
#endif
};
