/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/

#include "StaticMeshActorItem.h"
#include "ToolMenus.h"
#include "HIActorItem.h"

#define LOCTEXT_NAMESPACE "StaticMeshActorItem"

HIOutliner::FStaticMeshActorItem::FStaticMeshActorItem(AStaticMeshActor* InStaticMeshActor)
	: StaticMeshActor(InStaticMeshActor)
	, ID(InStaticMeshActor)
{
	Type = ITreeItem::StaticMeshActor;
}

bool HIOutliner::FStaticMeshActorItem::CanInteract() const
{
	return true;
}

FString HIOutliner::FStaticMeshActorItem::GetDisplayString() const
{
	if (StaticMeshActor.IsValid())
	{
		return StaticMeshActor->GetFName().GetPlainNameString();
	}
	else
	{
		return FString("");
	}	
}

HIOutliner::FTreeItemID HIOutliner::FStaticMeshActorItem::GetID()
{
	return ID;
}

void HIOutliner::FStaticMeshActorItem::GenerateContextMenu(UToolMenu * Menu, SHierarchicalInstancedOutliner& Outliner)
{
	FToolMenuSection& Section = Menu->AddSection("Section");
	Section.AddMenuEntry("MoveToLevel", LOCTEXT("MoveToLevel", "Move To Level"), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(&Outliner, &SHierarchicalInstancedOutliner::MoveStaticMeshActorToLevel)));
	Section.AddMenuEntry("Delete", LOCTEXT("Delete", "Delete"), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(&Outliner, &SHierarchicalInstancedOutliner::DeleteStaticMeshActor)));
}

#undef LOCTEXT_NAMESPACE
