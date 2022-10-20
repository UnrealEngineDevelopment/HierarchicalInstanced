/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedVolumeCustomization.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "HierarchicalInstancedVolume.h"
#include "HierarchicalInstanced/HierarchicalInstancedSettings.h"

TSharedRef<IDetailCustomization> FHIVolumeCustomization::MakeInstance()
{
	return MakeShareable(new FHIVolumeCustomization);
}

void FHIVolumeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//Edits a category. If it doesn't exist it creates a new one
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("HI Volume");
	CustomCategory.AddCustomRow(FText::GetEmpty())
	.WholeRowContent()
	.HAlign(HAlign_Left)
	[
		SNew(SBox)
		.MaxDesiredWidth(300.f)
		[
            SNew(SButton).VAlign(VAlign_Fill)
            .OnClicked(this, &FHIVolumeCustomization::BuildHIActor_OnClicked)
            .Content()
            [
                SNew(STextBlock).Text(FText::FromString("BuildHIActor"))
            ]
		]
	];

	//Store the currently selected objects from the viewport to the SelectedObjects array.
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
}

FReply FHIVolumeCustomization::BuildHIActor_OnClicked()
{
    for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
    {
        if (AHierarchicalInstancedVolume* HIVolume = Cast<AHierarchicalInstancedVolume>(Object.Get()))
        {
            if (!HIVolume->IsPendingKill())
            {
                HIVolume->BuildHIActor();
            }
        }
    }
    return FReply::Handled();
}
