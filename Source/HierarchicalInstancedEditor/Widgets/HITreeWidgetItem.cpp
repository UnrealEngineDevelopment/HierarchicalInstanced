/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HITreeWidgetItem.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STreeView.h"
#include "EditorStyleSet.h"
#include "DragAndDrop/ActorDragDropGraphEdOp.h"
#include "HIActorItem.h"

#define LOCTEXT_NAMESPACE "HITreeWidgetItem"

namespace HIOutliner
{
	static FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, TWeakPtr<STableViewBase> Table)
	{
		auto TablePtr = Table.Pin();
		if (TablePtr.IsValid() && MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		{
			auto TreeView = (STreeView< FTreeItemPtr >*)TablePtr.Get();
			TSharedPtr<FDragDropOperation> Operation = MakeShareable(new FDragDropOperation());
			if (Operation.IsValid())
			{
				return FReply::Handled().BeginDragDrop(Operation.ToSharedRef());
			}
		}
		return FReply::Unhandled();
	}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void SHITreeWidgetItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwner)
	{
		this->TreeItem = InArgs._TreeItemToVisualize.Get();
		this->Outliner = InArgs._Outliner;
		this->World = InArgs._World;
		check(TreeItem);

		WeakTableViewBase = InOwner;

		auto Args = FSuperRowType::FArguments()
			.Padding(1)
			.OnDragDetected_Static(HIOutliner::OnDragDetected, TWeakPtr<STableViewBase>(InOwner));

		SMultiColumnTableRow< FTreeItemPtr >::Construct(Args, InOwner);
	}


	TSharedRef<SWidget> SHITreeWidgetItem::GenerateWidgetForColumn(const FName& ColumnName)
	{
		if (ColumnName == TEXT("SceneActorName"))
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SExpanderArrow, SharedThis(this))
				]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SHITreeWidgetItem::GetItemDisplayString)
					.ColorAndOpacity(this, &SHITreeWidgetItem::GetTint)
				];
		}		
		else if (ColumnName == TEXT("InstanceCount") && TreeItem->GetTreeItemType() == ITreeItem::HierarchicalInstancedActor)
		{
			FHIActorItem* Item = static_cast<FHIActorItem*>(TreeItem);
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Item, &FHIActorItem::GetInstanceCountAsText)
					.ColorAndOpacity(this, &SHITreeWidgetItem::GetTint)
				];
		}
		else if (ColumnName == TEXT("Level") && TreeItem->GetTreeItemType() == ITreeItem::HierarchicalInstancedActor)
		{
			FHIActorItem* Item = static_cast<FHIActorItem*>(TreeItem);

			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Item, &FHIActorItem::GetLevelAsText)
					.ColorAndOpacity(this, &SHITreeWidgetItem::GetTint)
				];
		}
		else
		{
			return SNullWidget::NullWidget;
		}
	}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

	FText SHITreeWidgetItem::GetItemDisplayString() const
	{
		return FText::FromString(TreeItem->GetDisplayString());
	}

	FSlateColor SHITreeWidgetItem::GetTint() const
	{
		return TreeItem->GetTint();
	}

	void SHITreeWidgetItem::OnDragEnter(FGeometry const& MyGeometry, FDragDropEvent const& DragDropEvent)
	{
		if (TreeItem)
		{
		}
	}

	void SHITreeWidgetItem::OnDragLeave(FDragDropEvent const& DragDropEvent)
	{

	}

	FReply SHITreeWidgetItem::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
	{
		return FReply::Handled();
	}

	FReply SHITreeWidgetItem::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
	{
		if (TreeItem)
		{
		}
		return FReply::Unhandled();
	}	
};

#undef LOCTEXT_NAMESPACE
