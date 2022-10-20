/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"

namespace HIOutliner
{
	class SHierarchicalInstancedOutliner;
	struct FTreeItemID;
	using FTreeItemPtr = TSharedPtr<struct ITreeItem>;
	using FTreeItemRef = TSharedRef<struct ITreeItem>;

	struct ITreeItem : TSharedFromThis < ITreeItem >
	{
		enum TreeItemType
		{
			Invalid,
			HierarchicalInstancedActor,
			StaticMeshActor
		};

		friend SHierarchicalInstancedOutliner;

	protected:
		/** Default constructor */
		ITreeItem() : Parent(nullptr), Type(Invalid), bIsExpanded(false) {}
		virtual ~ITreeItem() {}

		/** This item's parent, if any. */
		mutable TWeakPtr<ITreeItem> Parent;

		/** Array of children contained underneath this item */
		mutable TArray<TWeakPtr<ITreeItem>> Children;

		/** Item type enum */
		mutable TreeItemType Type;

	public:
		/** Get this item's parent. Can be nullptr. */
		FTreeItemPtr GetParent() const
		{
			return Parent.Pin();
		}

		/** Add a child to this item */
		void AddChild(FTreeItemRef Child)
		{
			Child->Parent = AsShared();
			Children.Add(MoveTemp(Child));
		}

		/** Remove a child from this item */
		void RemoveChild(const FTreeItemRef& Child)
		{
			if (Children.Remove(Child))
			{
				Child->Parent = nullptr;
			}
		}

		/** Returns the TreeItem's type */
		const TreeItemType GetTreeItemType()
		{
			return Type;
		}

		/** Get this item's children, if any. Although we store as weak pointers, they are guaranteed to be valid. */
		FORCEINLINE const TArray<TWeakPtr<ITreeItem>>& GetChildren() const
		{
			return Children;
		}

		/** Flag whether or not this item is expanded in the treeview */
		bool bIsExpanded;

	public:		
		/** Get the raw string to display for this tree item - used for sorting */
		virtual FString GetDisplayString() const = 0;

		/** Check whether it should be possible to interact with this tree item */
		virtual bool CanInteract() const = 0;

		/** Called when this item is expanded or collapsed */
		virtual void OnExpansionChanged() {};

		/** Returns this TreeItem's ID */
		virtual FTreeItemID GetID() = 0;

		/** Generate a context menu for this item. Only called if *only* this item is selected. */
		virtual void GenerateContextMenu(UToolMenu* Menu, SHierarchicalInstancedOutliner& Outliner) {}

		/** Returns the Tint used for displaying this item in the Treeview*/
		virtual FSlateColor GetTint() const{return FLinearColor(1.0f, 1.0f, 1.0f);}
	};
};
