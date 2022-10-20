/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Layout/Geometry.h"
#include "Input/DragAndDrop.h"
#include "Input/Reply.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "HierarchicalInstancedEditor/Widgets/ITreeItem.h"

namespace HIOutliner
{
	class SHierarchicalInstancedOutliner;

	/**
	* Widget that visualizes the contents of a FReflectorNode.
	*/
	class SHITreeWidgetItem : public SMultiColumnTableRow < FTreeItemPtr >
	{
	public:

		SLATE_BEGIN_ARGS(SHITreeWidgetItem)
			: _TreeItemToVisualize(),
			_Outliner(),
			_World()
		{ }

		SLATE_ARGUMENT(FTreeItemPtr, TreeItemToVisualize)
		SLATE_ARGUMENT(SHierarchicalInstancedOutliner*, Outliner)
		SLATE_ARGUMENT(UWorld*, World)
		SLATE_END_ARGS()

	public:
		/**
		* Construct child widgets that comprise this widget.
		*
		* @param InArgs Declaration from which to construct this widget.
		*/
		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwner);

	public:
		// SMultiColumnTableRow overrides
		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

		/** Returns World set through the HLODOutliner parent widget */
		UWorld* GetWorld() const { return World; }
	protected:
		/** Returns the display string of the node */
		FText GetItemDisplayString() const;

		/** Returns the tint of the node */
		FSlateColor GetTint() const;
		
		//~ Begin STableRow Interface.
		virtual void OnDragEnter(FGeometry const& MyGeometry, FDragDropEvent const& DragDropEvent) override;
		virtual void OnDragLeave(FDragDropEvent const& DragDropEvent) override;
		virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
		virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
		//~ End STableRow Interface.

	private:
		/** The info about the widget that we are visualizing. */
		ITreeItem* TreeItem;

		/** Cached Display String as FText */
		FText CachedItemName;

		/** WeakPtr to the Treeview widget */
		TWeakPtr< STableViewBase > WeakTableViewBase;

		/** Pointer to the owning SHLODOutliner */
		SHierarchicalInstancedOutliner* Outliner;
		
		/** Pointer to the currently represented world (unused atm) */
		UWorld* World;
	};
};
