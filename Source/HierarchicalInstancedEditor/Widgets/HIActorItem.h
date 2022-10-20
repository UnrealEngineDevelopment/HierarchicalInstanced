/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"
#include "TreeItemID.h"
#include "UnrealClient.h"
#include "HierarchicalInstancedEditor/Widgets/ITreeItem.h"
#include "HierarchicalInstanced/HierarchicalInstancedActor.h"

class UToolMenu;

namespace HIOutliner
{
	struct FHIActorItem : ITreeItem
	{
		mutable TWeakObjectPtr<AHierarchicalInstancedActor> HIActor;
		mutable FTreeItemID ID;

		explicit FHIActorItem(AHierarchicalInstancedActor* InHIActor);

		//~ Begin ITreeItem Interface.
		virtual bool CanInteract() const override;
		virtual FString GetDisplayString() const override;
		virtual FSlateColor GetTint() const override;
		virtual FTreeItemID GetID() override;
		virtual void GenerateContextMenu(UToolMenu* Menu, class SHierarchicalInstancedOutliner& Outliner) override;
		//~ End ITreeItem Interface.

		FText GetInstanceCountAsText() const;

		FText GetLevelAsText() const;
	};
};
