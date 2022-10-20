/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedOutliner.h"
#include "HierarchicalInstanced/HierarchicalInstancedSettings.h"
#include "HierarchicalInstancedEditor/HierarchicalInstancedBuilder.h"
#include "Modules/ModuleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PropertyEditorModule.h"
#include "EditorFontGlyphs.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "ScopedTransaction.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "HITreeWidgetItem.h"
#include "HIActorItem.h"
#include "StaticMeshActorItem.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedOutliner"

namespace HIOutliner
{
	SHierarchicalInstancedOutliner::SHierarchicalInstancedOutliner()
		: bNeedsRefresh(true)
		, SettingsView(nullptr)
		, MainContentPanel(nullptr)
		, CurrentWorld(nullptr)
		, InstancedClusterSettings(nullptr)
		, TreeView(nullptr)
		, HITreeRoot()
		, SelectedNodes()
		, SelectedHIActors()
		, TreeItemsMap()
		, PendingActions()
		, bArrangeHorizontally(false)
	{

	}

	SHierarchicalInstancedOutliner::~SHierarchicalInstancedOutliner()
	{
		DeregisterDelegates();
		if (InstancedClusterSettings)
		{
			InstancedClusterSettings->RemoveFromRoot();
		}
	}

	void SHierarchicalInstancedOutliner::Construct(const FArguments & InArgs)
	{
		CreateSettingsView();
		InstancedClusterSettings = NewObject<UInstancedClusterSettings>();
		InstancedClusterSettings->AddToRoot();
		SettingsView->SetObject(InstancedClusterSettings);

		/** Holds all widgets for the profiler window like menu bar, toolbar and tabs. */
		MainContentPanel = SNew(SVerticalBox);
		ChildSlot
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				MainContentPanel.ToSharedRef()
			]
		];

		MainContentPanel->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(1.0f)
			[
				CreateMainButtonWidgets()
			]
		];

		TSharedRef<SWidget> DetailsWidgets =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(1.0f)
		[
			SettingsView.ToSharedRef()
		];

		TSharedRef<SWidget> ClusterWidgets =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(1.0f)
		[
			CreateTreeviewWidget()
		];

		MainContentPanel->AddSlot()
		.FillHeight(1.0f)
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex(this, &SHierarchicalInstancedOutliner::GetSpitterWidgetIndex)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
				.Style(FEditorStyle::Get(), "ContentBrowser.Splitter")
				+ SSplitter::Slot()
				.Value(0.5)
				[
					ClusterWidgets
				]
				+ SSplitter::Slot()
				.Value(0.5)
				[
					DetailsWidgets
				]
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Vertical)
				.Style(FEditorStyle::Get(), "ContentBrowser.Splitter")
				+ SSplitter::Slot()
				.Value(0.5)
				[
					ClusterWidgets
				]
				+ SSplitter::Slot()
				.Value(0.5)
				[
					DetailsWidgets
				]
			]
		];

		RegisterDelegates();
	}

	void SHierarchicalInstancedOutliner::CreateSettingsView()
	{
		// initialize settings view
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.bAllowSearch = true;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.bLockable = false;
			DetailsViewArgs.bSearchInitialKeyFocus = true;
			DetailsViewArgs.bUpdatesFromSelection = false;
			DetailsViewArgs.NotifyHook = this;
			DetailsViewArgs.bShowOptions = true;
			DetailsViewArgs.bShowModifiedPropertiesOption = false;
			DetailsViewArgs.bAllowMultipleTopLevelObjects = true;
			DetailsViewArgs.bShowActorLabel = false;
			DetailsViewArgs.bCustomNameAreaLocation = true;
			DetailsViewArgs.bCustomFilterAreaLocation = true;
			DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
			DetailsViewArgs.bShowPropertyMatrixButton = false;
		}
		SettingsView = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);

		//FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		//FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::ENameAreaSettings::HideNameArea, false, nullptr, false, NAME_None);
		//TSharedRef<IDetailsView> DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
		//DetailsView->SetObject(InLocalizationTarget, true);
		//DetailsView->SetIsPropertyEditingEnabledDelegate(IsPropertyEditingEnabled);
		//DetailsView->OnFinishedChangingProperties().AddSP(this, &SLocalizationTargetEditor::OnFinishedChangingProperties);

		//ChildSlot
		//	[
		//		DetailsView
		//	];
	}

	TSharedRef<SWidget> SHierarchicalInstancedOutliner::CreateMainButtonWidgets()
	{
		return SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(FMargin(0.0f, 2.0f))
			[
				SNew(SWrapBox)
				.UseAllottedWidth(true)

				+ SWrapBox::Slot()
				.Padding(FMargin(2.0f))
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
					.HAlign(HAlign_Center)
					.OnClicked(this, &SHierarchicalInstancedOutliner::HandleBuildHIActors)
					.ToolTipText(LOCTEXT("BuildHIClustersToolTip", "Re-generates clusters and then instancing for each of the generated clusters in the level. This dirties the level."))
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Building)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew( STextBlock )
							.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
							.Text(LOCTEXT("BuildAll", "Build All"))
						]
					]
				]

				+ SWrapBox::Slot()
				.Padding(FMargin(2.0f))
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
					.HAlign(HAlign_Center)
					.OnClicked(this, &SHierarchicalInstancedOutliner::HandleRevertHIActors)
					.ToolTipText(LOCTEXT("RevertActorsToLevelToolTip", "Uninstancing actors to level. This dirties the level."))
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
							.Text(FEditorFontGlyphs::Undo)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4, 0, 0, 0)
						[
							SNew( STextBlock )
							.TextStyle( FEditorStyle::Get(), "ContentBrowser.TopBar.Font" )
							.Text(LOCTEXT("RevertAll", "Revert actors to level"))
						]
					]
				]
			];
	}

	TSharedRef<SWidget> SHierarchicalInstancedOutliner::CreateTreeviewWidget()
	{
		return SAssignNew(TreeView, SHITree)
			.ItemHeight(24.0f)
			.TreeItemsSource(&HITreeRoot)
			.OnGenerateRow(this, &SHierarchicalInstancedOutliner::OnOutlinerGenerateRow)
			.OnGetChildren(this, &SHierarchicalInstancedOutliner::OnOutlinerGetChildren)
			.OnSelectionChanged(this, &SHierarchicalInstancedOutliner::OnOutlinerSelectionChanged)
			.OnMouseButtonDoubleClick(this, &SHierarchicalInstancedOutliner::OnOutlinerDoubleClick)
			.OnContextMenuOpening(this, &SHierarchicalInstancedOutliner::OnOpenContextMenu)
			.OnExpansionChanged(this, &SHierarchicalInstancedOutliner::OnItemExpansionChanged)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("SceneActorName")
				.DefaultLabel(LOCTEXT("SceneActorName", "Scene Actor Name"))
				.FillWidth(0.3f)
				+ SHeaderRow::Column("InstanceCount")
				.DefaultLabel(LOCTEXT("InstanceCount", "Instance count"))
				.DefaultTooltip(LOCTEXT("InstanceCountToolTip", "Instance count"))
				.FillWidth(0.2f)
				+ SHeaderRow::Column("Level")
				.DefaultLabel(LOCTEXT("Level", "Level"))
				.DefaultTooltip(LOCTEXT("LevelToolTip", "Persistent Level of a HierarchicalInstancedActor"))
				.FillWidth(0.2f)
			);
	}

	bool SHierarchicalInstancedOutliner::UpdateCurrentWorld()
	{
		CurrentWorld = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				CurrentWorld = Context.World();
				break;
			}
			else if (Context.WorldType == EWorldType::Editor)
			{
				CurrentWorld = Context.World();
			}
		}
		return (CurrentWorld.IsValid());
	}

	void SHierarchicalInstancedOutliner::RegisterDelegates()
	{
		FEditorDelegates::MapChange.AddSP(this, &SHierarchicalInstancedOutliner::OnMapChange);
		FEditorDelegates::NewCurrentLevel.AddSP(this, &SHierarchicalInstancedOutliner::OnNewCurrentLevel);
		FEditorDelegates::OnMapOpened.AddSP(this, &SHierarchicalInstancedOutliner::OnMapLoaded);
		FWorldDelegates::LevelAddedToWorld.AddSP(this, &SHierarchicalInstancedOutliner::OnLevelAdded);
		FWorldDelegates::LevelRemovedFromWorld.AddSP(this, &SHierarchicalInstancedOutliner::OnLevelRemoved);
	}

	void SHierarchicalInstancedOutliner::DeregisterDelegates()
	{
		FEditorDelegates::MapChange.RemoveAll(this);
		FEditorDelegates::NewCurrentLevel.RemoveAll(this);
		FEditorDelegates::OnMapOpened.RemoveAll(this);
		FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
		FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
	}

	FReply SHierarchicalInstancedOutliner::HandleBuildHIActors()
	{
		UpdateCurrentWorld();
		FHierarchicalInstancedBuilder HierarchicalInstancedBuilder(InstancedClusterSettings, CurrentWorld.Get());
		HierarchicalInstancedBuilder.Build();
		bNeedsRefresh = true;
		return FReply::Handled();
	}

	FReply SHierarchicalInstancedOutliner::HandleRevertHIActors()
	{
		UpdateCurrentWorld();
		FHierarchicalInstancedBuilder HierarchicalInstancedBuilder(InstancedClusterSettings, CurrentWorld.Get());
		HierarchicalInstancedBuilder.RevertActorsToLevel();
		bNeedsRefresh = true;
		return FReply::Handled();
	}

	void SHierarchicalInstancedOutliner::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
	{
		if (bNeedsRefresh)
		{
			FullRefresh();
		}
		bool bChangeMade = false;
		// Only deal with 256 at a time
		const int32 End = FMath::Min(PendingActions.Num(), 512);
		for (int32 Index = 0; Index < End; ++Index)
		{
			auto& PendingAction = PendingActions[Index];
			switch (PendingAction.Type)
			{
			case FOutlinerAction::AddItem:
				bChangeMade |= AddItemToTree(PendingAction.Item, PendingAction.ParentItem);
				break;
			case FOutlinerAction::RemoveItem:
				RemoveItemFromTree(PendingAction.Item);
				bChangeMade = true;
				break;
			default:
				check(false);
				break;
			}
		}
		PendingActions.RemoveAt(0, End);

		if (bChangeMade)
		{
			// Restore expansion states
			TreeView->RequestTreeRefresh();
		}

		bArrangeHorizontally = AllottedGeometry.Size.X > AllottedGeometry.Size.Y;
	}

	void SHierarchicalInstancedOutliner::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, class FEditPropertyChain* PropertyThatChanged)
	{

	}

	TSharedRef<ITableRow> SHierarchicalInstancedOutliner::OnOutlinerGenerateRow(FTreeItemPtr InTreeItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		TSharedRef<ITableRow> Widget = SNew(SHITreeWidgetItem, OwnerTable)
			.TreeItemToVisualize(InTreeItem)
			.Outliner(this)
			.World(CurrentWorld.Get());

		return Widget;
	}

	void SHierarchicalInstancedOutliner::OnOutlinerGetChildren(FTreeItemPtr InParent, TArray<FTreeItemPtr>& OutChildren)
	{
		for (auto& WeakChild : InParent->GetChildren())
		{
			auto Child = WeakChild.Pin();
			// Should never have bogus entries in this list
			check(Child.IsValid());
			OutChildren.Add(Child);
		}
	}

	void SHierarchicalInstancedOutliner::OnOutlinerSelectionChanged(FTreeItemPtr TreeItem, ESelectInfo::Type SelectionInfo)
	{
		if (SelectionInfo == ESelectInfo::Direct)
		{
			return;
		}

		TArray<FTreeItemPtr> NewSelectedNodes = TreeView->GetSelectedItems();
		// Make sure that we do not actually change selection when the users selects a HLOD level node
		if (NewSelectedNodes.ContainsByPredicate([](FTreeItemPtr Item) -> bool { return Item.IsValid() && Item->GetTreeItemType() != ITreeItem::Invalid;  }))
		{
			EmptySelection();

			// Loop over previously retrieve lsit of selected nodes
			StartSelection();

			bool bChanged = false;

			for (const FTreeItemPtr& SelectedItem : NewSelectedNodes)
			{
				if (SelectedItem.IsValid())
				{
					ITreeItem::TreeItemType Type = SelectedItem->GetTreeItemType();
					switch (Type)
					{
					case ITreeItem::HierarchicalInstancedActor:
					{
						FHIActorItem* ActorItem = (FHIActorItem*)(SelectedItem.Get());
						SelectActorInViewport(ActorItem->HIActor.Get(), 0);
						bChanged = true;
						break;
					}

					case ITreeItem::StaticMeshActor:
					{
						FStaticMeshActorItem* StaticMeshActorItem = (FStaticMeshActorItem*)(SelectedItem.Get());
						SelectActorInViewport(StaticMeshActorItem->StaticMeshActor.Get(), 0);
						bChanged = true;
						break;
					}
					}
				}
			}
			EndSelection(bChanged);
		}
		SelectedNodes = TreeView->GetSelectedItems();
	}

	void SHierarchicalInstancedOutliner::OnOutlinerDoubleClick(FTreeItemPtr TreeItem)
	{
		ITreeItem::TreeItemType Type = TreeItem->GetTreeItemType();
		const bool bActiveViewportOnly = false;

		switch (Type)
		{
		case ITreeItem::HierarchicalInstancedActor:
		{
			FHIActorItem* ActorItem = (FHIActorItem*)(TreeItem.Get());
			SelectActorInViewport(ActorItem->HIActor.Get(), 0);
			GEditor->MoveViewportCamerasToActor(*ActorItem->HIActor.Get(), bActiveViewportOnly);
			break;
		}
		case ITreeItem::StaticMeshActor:
		{
			FStaticMeshActorItem* StaticMeshActorItem = (FStaticMeshActorItem*)(TreeItem.Get());
			SelectActorInViewport(StaticMeshActorItem->StaticMeshActor.Get(), 0);
			GEditor->MoveViewportCamerasToActor(*StaticMeshActorItem->StaticMeshActor.Get(), bActiveViewportOnly);
			break;
		}
		}
	}

	TSharedPtr<SWidget> SHierarchicalInstancedOutliner::OnOpenContextMenu()
	{
		if (!CurrentWorld.IsValid())
		{
			return nullptr;
		}

		// Multi-selection support, check if all selected items are of the same type, if so return the appropriate context menu
		auto SelectedItems = TreeView->GetSelectedItems();
		ITreeItem::TreeItemType Type = ITreeItem::Invalid;
		bool bSameType = true;
		for (int32 SelectedIndex = 0; SelectedIndex < SelectedItems.Num(); ++SelectedIndex)
		{
			if (SelectedIndex == 0)
			{
				Type = SelectedItems[SelectedIndex]->GetTreeItemType();
			}
			else
			{
				// Not all of the same types
				if (SelectedItems[SelectedIndex]->GetTreeItemType() != Type)
				{
					bSameType = false;
					break;
				}
			}
		}

		if (SelectedItems.Num() && bSameType)
		{
			UToolMenus* ToolMenus = UToolMenus::Get();
			static const FName MenuName = "HierarchicalLODOutliner.HLODOutlinerContextMenu";
			if (!ToolMenus->IsMenuRegistered(MenuName))
			{
				ToolMenus->RegisterMenu(MenuName);
			}

			// Build up the menu for a selection
			FToolMenuContext Context;
			UToolMenu* Menu = ToolMenus->GenerateMenu(MenuName, Context);
			TreeView->GetSelectedItems()[0]->GenerateContextMenu(Menu, *this);
			return ToolMenus->GenerateWidget(Menu);
		}

		return TSharedPtr<SWidget>();
	}

	void SHierarchicalInstancedOutliner::OnItemExpansionChanged(FTreeItemPtr TreeItem, bool bIsExpanded)
	{
		TreeItem->bIsExpanded = bIsExpanded;
		// Expand any children that are also expanded
		for (const auto& WeakChild : TreeItem->GetChildren())
		{
			auto Child = WeakChild.Pin();
			if (Child->bIsExpanded)
			{
				TreeView->SetItemExpansion(Child, true);
			}
		}
	}

	void SHierarchicalInstancedOutliner::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
	{
		bNeedsRefresh = true;
	}

	void SHierarchicalInstancedOutliner::OnLevelRemoved(ULevel* InLevel, UWorld* InWorld)
	{
		bNeedsRefresh = true;
	}

	void SHierarchicalInstancedOutliner::OnMapChange(uint32 MapFlags)
	{
		CurrentWorld = nullptr;
		bNeedsRefresh = true;
	}

	void SHierarchicalInstancedOutliner::OnNewCurrentLevel()
	{
		CurrentWorld = nullptr;
		bNeedsRefresh = true;
	}

	void SHierarchicalInstancedOutliner::OnMapLoaded(const FString& Filename, bool bAsTemplate)
	{
		CurrentWorld = nullptr;
		bNeedsRefresh = true;
	}

	int32 SHierarchicalInstancedOutliner::GetSpitterWidgetIndex() const
	{
		// Split vertically or horizontally based on dimensions
		return bArrangeHorizontally ? 0 : 1;
	}

	void SHierarchicalInstancedOutliner::RebuildInstances()
	{
		if (CurrentWorld.IsValid())
		{
			const FScopedTransaction Transaction(LOCTEXT("UndoAction_RebuildInstances", "Rebuild Instances"));

			// This call came from a context menu
			auto SelectedItems = TreeView->GetSelectedItems();

			// Loop over all selected items (context menu can't be called with multiple items selected that aren't of the same types)
			for (const auto& SelectedItem : SelectedItems)
			{
				FHIActorItem* ActorItem = (FHIActorItem*)(SelectedItem.Get());
				if (ActorItem && ActorItem->HIActor.IsValid())
				{
					ActorItem->HIActor->UpdateInstances();
				}
			}
		}
	}

	void SHierarchicalInstancedOutliner::RevertActorsToLevel()
	{
		if (CurrentWorld.IsValid())
		{
			const FScopedTransaction Transaction(LOCTEXT("UndoAction_RevertActorsToLevel", "Revert Actors To Level"));

			// This call came from a context menu
			auto SelectedItems = TreeView->GetSelectedItems();

			// Loop over all selected items (context menu can't be called with multiple items selected that aren't of the same types)
			for (const auto& SelectedItem : SelectedItems)
			{
				FHIActorItem* ActorItem = (FHIActorItem*)(SelectedItem.Get());
				if (ActorItem && ActorItem->HIActor.IsValid())
				{
					ActorItem->HIActor->RevertActorsToLevel();
				}
			}
		}
	}

	void SHierarchicalInstancedOutliner::MoveStaticMeshActorToLevel()
	{
		if (CurrentWorld.IsValid())
		{
			const FScopedTransaction Transaction(LOCTEXT("UndoAction_MoveStaticMeshActorToLevel", "Move StaticMeshActor To Level"));

			// This call came from a context menu
			auto SelectedItems = TreeView->GetSelectedItems();

			// Loop over all selected items (context menu can't be called with multiple items selected that aren't of the same types)
			for (auto SelectedItem : SelectedItems)
			{
				FStaticMeshActorItem* ActorItem = (FStaticMeshActorItem*)(SelectedItem.Get());
				if (ActorItem && ActorItem->StaticMeshActor.IsValid())
				{
					PendingActions.Emplace(FOutlinerAction::RemoveItem, SelectedItem);
					auto Parent = ActorItem->GetParent();

					FHIActorItem* HIActorItem = (FHIActorItem*)(Parent.Get());
					if (HIActorItem)
					{
						HIActorItem->HIActor->RemoveReplacementActor(ActorItem->StaticMeshActor.Get());
						HIActorItem->HIActor->UpdateInstances();
					}
					ActorItem->StaticMeshActor->Modify();
					ActorItem->StaticMeshActor->bIsEditorOnlyActor = false;
					ActorItem->StaticMeshActor->SetHidden(false);
					ActorItem->StaticMeshActor->bHiddenEd = false;
					ActorItem->StaticMeshActor->SetIsTemporarilyHiddenInEditor(false);
					ActorItem->StaticMeshActor->SetFolderPath(TEXT(""));
				}
			}
		}
	}

	void SHierarchicalInstancedOutliner::DeleteStaticMeshActor()
	{
		if (CurrentWorld.IsValid())
		{
			const FScopedTransaction Transaction(LOCTEXT("UndoAction_DeleteStaticMeshActor", "Delete StaticMeshActor"));
			// This call came from a context menu
			auto SelectedItems = TreeView->GetSelectedItems();
			// Loop over all selected items (context menu can't be called with multiple items selected that aren't of the same types)
			for (auto SelectedItem : SelectedItems)
			{
				FStaticMeshActorItem* ActorItem = (FStaticMeshActorItem*)(SelectedItem.Get());
				if (ActorItem && ActorItem->StaticMeshActor.IsValid())
				{
					PendingActions.Emplace(FOutlinerAction::RemoveItem, SelectedItem);
					auto Parent = ActorItem->GetParent();
					FHIActorItem* HIActorItem = (FHIActorItem*)(Parent.Get());
					if (HIActorItem)
					{
						HIActorItem->HIActor->RemoveReplacementActor(ActorItem->StaticMeshActor.Get());
						HIActorItem->HIActor->UpdateInstances();
					}
					ActorItem->StaticMeshActor->Destroy();
				}
			}
		}
	}

	const bool SHierarchicalInstancedOutliner::AddItemToTree(FTreeItemPtr InItem, FTreeItemPtr InParentItem)
	{
		const auto ItemID = InItem->GetID();

		TreeItemsMap.Add(ItemID, InItem);

		if (InParentItem.Get())
		{
			InParentItem->AddChild(InItem->AsShared());
		}

		return true;
	}

	void SHierarchicalInstancedOutliner::RemoveItemFromTree(FTreeItemPtr InItem)
	{
		const int32 NumRemoved = TreeItemsMap.Remove(InItem->GetID());

		if (!NumRemoved)
		{
			return;
		}

		auto ParentItem = InItem->GetParent();
		if (ParentItem.IsValid())
		{
			ParentItem->RemoveChild(InItem->AsShared());
		}
	}

	void SHierarchicalInstancedOutliner::FullRefresh()
	{
		ResetCachedData();
		UpdateCurrentWorld();
		// Loop over all the levels in the current world
		for (ULevel* Level : CurrentWorld->GetLevels())
		{
			// Only handling visible levels (this is to allow filtering the HLOD outliner per level, should change when adding new sortable-column)
			if (Level->bIsVisible)
			{
				for (AActor* Actor : Level->Actors)
				{
					// Only handling LODActors
					if (AHierarchicalInstancedActor* HIActor = Cast<AHierarchicalInstancedActor>(Actor))
					{
						FTreeItemPtr Item = MakeShareable(new FHIActorItem(HIActor));
						HITreeRoot.Add(Item);

						for (AStaticMeshActor* ChildActor : HIActor->ReplacementActors)
						{
							FTreeItemRef ChildItem = MakeShareable(new FStaticMeshActorItem(ChildActor));
							PendingActions.Emplace(FOutlinerAction::AddItem, ChildItem, Item);
						}
					}
				}
			}
		}
		// Request treeview UI item to refresh
		TreeView->RequestTreeRefresh();
		bNeedsRefresh = false;
	}

	void SHierarchicalInstancedOutliner::ResetCachedData()
	{
		HITreeRoot.Reset();
		PendingActions.Reset();
		TreeItemsMap.Reset();
	}

	void SHierarchicalInstancedOutliner::StartSelection()
	{
		GEditor->GetSelectedActors()->BeginBatchSelectOperation();
	}

	void SHierarchicalInstancedOutliner::EmptySelection()
	{
		GEditor->SelectNone(false, true, true);
		DestroySelectionActors();
	}

	void SHierarchicalInstancedOutliner::DestroySelectionActors()
	{
		SelectedHIActors.Empty();
	}

	void SHierarchicalInstancedOutliner::SelectActorInViewport(AActor* Actor, const uint32 SelectionDepth)
	{
		GEditor->SelectActor(Actor, true, false, true);

		if (Actor->IsA<AHierarchicalInstancedActor>() && SelectionDepth == 0)
		{
			SelectedHIActors.AddUnique(Actor);
		}
	}

	void SHierarchicalInstancedOutliner::EndSelection(const bool bChange)
	{
		// Commit selection changes
		GEditor->GetSelectedActors()->EndBatchSelectOperation();
		if (bChange)
		{
			// Fire selection changed event
			GEditor->NoteSelectionChange();
		}
	}

} // namespace HIOutliner
#undef LOCTEXT_NAMESPACE

