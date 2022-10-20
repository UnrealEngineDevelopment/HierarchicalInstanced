/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedActorCustomization.h"
#include "HierarchicalInstanced/HierarchicalInstancedActor.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
TSharedRef<IDetailCustomization> FHIActorCustomization::MakeInstance()
{
	return MakeShareable(new FHIActorCustomization);
}

void FHIActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//Edits a category. If it doesn't exist it creates a new one
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Instancing");
	CustomCategory.AddCustomRow(FText::GetEmpty())
	.WholeRowContent()
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.MaxDesiredWidth(300.f)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2.0f)
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton).VAlign(VAlign_Fill)
				.OnClicked(this, &FHIActorCustomization::RefreshInstances_OnClicked) //Binding the OnClick function we want to execute when this object is clicked
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("RefreshInstances"))
				]
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton).VAlign(VAlign_Fill)
				.OnClicked(this, &FHIActorCustomization::RevertActorsToLevel_OnClicked) //Binding the OnClick function we want to execute when this object is clicked
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("RevertActorsToLevel"))
				]
			]
		]
	];

	//Store the currently selected objects from the viewport to the SelectedObjects array.
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
}

FReply FHIActorCustomization::RefreshInstances_OnClicked()
{
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		if (AHierarchicalInstancedActor* Actor = Cast<AHierarchicalInstancedActor>(Object.Get()))
		{
			if (!Actor->IsPendingKill())
			{
				Actor->UpdateInstances();
			}
		}
	}
	return FReply::Handled();
}

FReply FHIActorCustomization::RevertActorsToLevel_OnClicked()
{
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		if (AHierarchicalInstancedActor* Actor = Cast<AHierarchicalInstancedActor>(Object.Get()))
		{
			if (!Actor->IsPendingKill())
			{
				Actor->RevertActorsToLevel();
			}
		}
	}
	return FReply::Handled();
}