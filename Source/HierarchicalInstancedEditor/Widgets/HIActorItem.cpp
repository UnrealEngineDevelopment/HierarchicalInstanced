/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HIActorItem.h"
#include "ToolMenus.h"
#include "Engine/Level.h"
#include "HierarchicalInstancedOutliner.h"

#define LOCTEXT_NAMESPACE "HIActorItem"

HIOutliner::FHIActorItem::FHIActorItem(AHierarchicalInstancedActor* InHIActor)
	: HIActor(InHIActor)
	, ID(InHIActor)
{
	Type = ITreeItem::HierarchicalInstancedActor;
}


bool HIOutliner::FHIActorItem::CanInteract() const
{
	return true;
}


FString HIOutliner::FHIActorItem::GetDisplayString() const
{
	if (AHierarchicalInstancedActor* ActorPtr = HIActor.Get())
	{
		return ActorPtr->GetName();
	}
	return FString();
}

HIOutliner::FTreeItemID HIOutliner::FHIActorItem::GetID()
{
	return ID;
}

void HIOutliner::FHIActorItem::GenerateContextMenu(UToolMenu * Menu, SHierarchicalInstancedOutliner& Outliner)
{
	FToolMenuSection& Section = Menu->AddSection("Section");
	Section.AddMenuEntry("RefreshInstances", LOCTEXT("RefreshInstances", "Refresh Instances"), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(&Outliner, &SHierarchicalInstancedOutliner::RebuildInstances)));
	Section.AddMenuEntry("RevertActorsToLevel", LOCTEXT("RevertActorsToLevel", "Revert Actors To Level"), FText(), FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(&Outliner, &SHierarchicalInstancedOutliner::RevertActorsToLevel)));
}

FSlateColor HIOutliner::FHIActorItem::GetTint() const
{
	AHierarchicalInstancedActor* HIActorPtr = HIActor.Get();
	if (HIActorPtr)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f);
	}
	return FLinearColor(1.0f, 1.0f, 1.0f);
}

FText HIOutliner::FHIActorItem::GetInstanceCountAsText() const
{
	if (HIActor.IsValid())
	{
		return FText::FromString(FString::FromInt(HIActor->GetInstanceCount()));
	}
	else
	{
		return FText::FromString(TEXT("Not available"));
	}
}

FText HIOutliner::FHIActorItem::GetLevelAsText() const
{
	if (HIActor.IsValid())
	{
		return FText::FromString(HIActor->GetLevel()->GetOuter()->GetName());
	}
	else
	{
		return FText::FromString(TEXT("Invalid"));
	}
}

#undef LOCTEXT_NAMESPACE
