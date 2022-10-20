/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Engine/Level.h"
#include "Widgets/SWidget.h"
#include "TreeItemID.h"
#include "ITreeItem.h"
#include "Engine/StaticMeshActor.h"

class UToolMenu;

namespace HIOutliner
{
	struct FStaticMeshActorItem : ITreeItem
	{
		/** Represented StaticMeshActor */
		mutable TWeakObjectPtr<AStaticMeshActor> StaticMeshActor;
		/** TreeItem's ID */
		mutable FTreeItemID ID;

		explicit FStaticMeshActorItem(AStaticMeshActor* InStaticMeshActor);

		//~ Begin ITreeItem Interface.
		virtual bool CanInteract() const override;
		virtual FString GetDisplayString() const override;
		virtual FTreeItemID GetID() override;
		virtual void GenerateContextMenu(UToolMenu* Menu, SHierarchicalInstancedOutliner& Outliner) override;
		//~ End ITreeItem Interface.
	};
};
