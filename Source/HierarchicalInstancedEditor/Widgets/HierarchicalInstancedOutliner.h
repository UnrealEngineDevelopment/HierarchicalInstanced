/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Misc/NotifyHook.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"
#include "TreeItemID.h"
#include "ITreeItem.h"

class FHierarchicalInstancedBuilder;
class UInstancedClusterSettings;
namespace HIOutliner
{
	using SHITree = STreeView<FTreeItemPtr>;

	/**
	* Outliner action class used for making changing to the Outliner's treeview
	*/
	struct FOutlinerAction
	{
		enum ActionType
		{
			AddItem,
			RemoveItem,
		};

		FOutlinerAction(ActionType InType, FTreeItemPtr InItem) : Type(InType), Item(InItem) {};
		FOutlinerAction(ActionType InType, FTreeItemPtr InItem, FTreeItemPtr InParentItem) : Type(InType), Item(InItem), ParentItem(InParentItem) {};

		ActionType Type;
		FTreeItemPtr Item;
		FTreeItemPtr ParentItem;
	};

	class SHierarchicalInstancedOutliner : public SCompoundWidget, public FNotifyHook
	{
		friend struct FHIActorItem;
		friend struct FStaticMeshActorItem;
	public:
		/** Default constructor. */
		SHierarchicalInstancedOutliner();
		/** Virtual destructor. */
		virtual ~SHierarchicalInstancedOutliner();

		SLATE_BEGIN_ARGS(SHierarchicalInstancedOutliner) {}
		SLATE_END_ARGS()

		/**
		* Constructs this widget.
		*/
		void Construct(const FArguments& InArgs);

		void CreateSettingsView();
		TSharedRef<SWidget> CreateMainButtonWidgets();
		TSharedRef<SWidget> CreateTreeviewWidget();

		bool UpdateCurrentWorld();

		FReply HandleBuildHIActors();
		FReply HandleRevertHIActors();
		//~ Begin SCompoundWidget Interface
		virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
		//~ End SCompoundWidget Interface
	public:
		// FNotifyHook interface
		virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, class FEditPropertyChain* PropertyThatChanged) override;
	protected:
		/** Tree view callbacks */

		/**
		* Generates a Tree view row for the given Tree view Node
		*
		* @param InReflectorNode - Node to generate a row for
		* @param OwnerTable - Owning table of the InReflectorNode
		* @return TSharedRef<ITableRow>
		*/
		TSharedRef<ITableRow> OnOutlinerGenerateRow(FTreeItemPtr InTreeItem, const TSharedRef<STableViewBase>& OwnerTable);

		/**
		* Treeview callback for retrieving the children of a specific TreeItem
		*
		* @param InParent - Parent item to return the children from
		* @param OutChildren - InOut array for the children
		*/
		void OnOutlinerGetChildren(FTreeItemPtr InParent, TArray<FTreeItemPtr>& OutChildren);

		/**
		* Handles the event fired when selection with the HLOD Tree view changes
		*
		* @param TreeItem - Selected node(s)
		* @param SelectionInfo - Type of selection change
		*/
		void OnOutlinerSelectionChanged(FTreeItemPtr TreeItem, ESelectInfo::Type SelectionInfo);

		/**
		* Handles double click events from the HLOD Tree view
		*
		* @param TreeItem - Node which was double-clicked
		*/
		void OnOutlinerDoubleClick(FTreeItemPtr TreeItem);

		/** Open a context menu for this scene outliner */
		TSharedPtr<SWidget> OnOpenContextMenu();

		/**
		* Handles item expansion events from the HLOD tree view
		*
		* @param TreeItem - Item which expansion state was changed
		* @param bIsExpanded - New expansion state
		*/
		void OnItemExpansionChanged(FTreeItemPtr TreeItem, bool bIsExpanded);

		/** End of Tree view callbacks */
	protected:
		/** Registers all the callback delegates required for keeping the treeview sync */
		void RegisterDelegates();

		/** De-registers all the callback delegates required for keeping the treeview sync */
		void DeregisterDelegates();

		/** Called by the engine when a level is added to the world. */
		void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);

		/** Called by the engine when a level is removed from the world. */
		void OnLevelRemoved(ULevel* InLevel, UWorld* InWorld);

		/** Called when the map has changed*/
		void OnMapChange(uint32 MapFlags);

		/** Called when the current level has changed */
		void OnNewCurrentLevel();

		/** Called when a new map is being loaded */
		void OnMapLoaded(const FString&  Filename, bool bAsTemplate);
		/** Handle splitting horizontally or vertically based on dimensions */
		int32 GetSpitterWidgetIndex() const;

		// ~Begin menu actions
		void RebuildInstances();
		void RevertActorsToLevel();
		void MoveStaticMeshActorToLevel();
		void DeleteStaticMeshActor();
		// ~End menu actions

	private:
		/**
		* Adds a new Treeview item
		*
		* @param InItem - Item to add
		* @param InParentItem - Optional parent item to add it to
		* @return const bool
		*/
		const bool AddItemToTree(FTreeItemPtr InItem, FTreeItemPtr InParentItem);

		/**
		* Removes a TreeView item
		*
		* @param InItem - Item to remove
		*/
		void RemoveItemFromTree(FTreeItemPtr InItem);

		/** Tells the scene outliner that it should do a full refresh, which will clear the entire tree and rebuild it from scratch. */
		void FullRefresh();

		/** Clears and resets all arrays and maps containing cached/temporary data */
		void ResetCachedData();

		/** Starts the Editor selection batch */
		void StartSelection();

		/** Empties the current Editor selection */
		void EmptySelection();

		/** Destroys the created selection actors */
		void DestroySelectionActors();

		/**
		* Selects an Actor in the Editor viewport
		*
		* @param Actor - AActor to select inside the viewport
		* @param SelectionDepth - (recursive)
		*/
		void SelectActorInViewport(AActor* Actor, const uint32 SelectionDepth);
		/** Ends the Editor selection batch, bChange determines whether or not there was an actual change and call NoteSelectionChange */
		void EndSelection(const bool bChange);
	private:
		/** Whether or not we need to do a refresh of the Tree view*/
		bool bNeedsRefresh;
		/** Property viewing widget */
		TSharedPtr<IDetailsView>   SettingsView;

		/** Content panel widget */
		TSharedPtr<SVerticalBox> MainContentPanel;

		/** Owning world HIActors are created for */
		TWeakObjectPtr<UWorld> CurrentWorld;
		UInstancedClusterSettings* InstancedClusterSettings;

		/** HI Treeview widget*/
		TSharedPtr<SHITree> TreeView;
		/** Tree view nodes */
		TArray<FTreeItemPtr> HITreeRoot;
		/** Currently selected Tree view nodes*/
		TArray<FTreeItemPtr> SelectedNodes;
		/** Array of currently selected LODActors */
		TArray<AActor*> SelectedHIActors;
		/** Map containing all the nodes with their corresponding keys */
		TMultiMap<FTreeItemID, FTreeItemPtr> TreeItemsMap;
		/** Array of pending OutlinerActions */
		TArray<FOutlinerAction> PendingActions;
		/** Whether to arrange the main UI horizontally or vertically */
		bool bArrangeHorizontally;
	};
};
