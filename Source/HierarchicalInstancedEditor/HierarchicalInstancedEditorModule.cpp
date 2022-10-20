/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedEditorModule.h"
#include "Engine/StaticMeshActor.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/HierarchicalInstancedOutliner.h"
#include "HierarchicalInstancedStyle.h"
#include "HierarchicalInstancedCommands.h"
#include "HierarchicalInstanced/HierarchicalInstancedActor.h"
#include "HierarchicalInstancedActorCustomization.h"
#include "HierarchicalInstancedVolume.h"
#include "HierarchicalInstancedVolumeCustomization.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedEditorModule"

static const FName HierarchicalInstancedTabName("HierarchicalInstanced");

void FHierarchicalInstancedEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	RegisterCustomClassLayout(AHierarchicalInstancedActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FHIActorCustomization::MakeInstance));
    RegisterCustomClassLayout(AHierarchicalInstancedVolume::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FHIVolumeCustomization::MakeInstance));
    PropertyModule.NotifyCustomizationModuleChanged();

	FHierarchicalInstancedStyle::Initialize();
	FHierarchicalInstancedStyle::ReloadTextures();
	FHierarchicalInstancedCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FHierarchicalInstancedCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FHierarchicalInstancedEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FHierarchicalInstancedEditorModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		//TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		//ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FHierarchicalInstancedEditorModule::AddToolbarExtension));
		//LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HierarchicalInstancedTabName, FOnSpawnTab::CreateRaw(this, &FHierarchicalInstancedEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FHierarchicalInstancedTabTitle", "HierarchicalInstanced"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FHierarchicalInstancedEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// Unregister all classes customized by name
		for (auto It = RegisteredClassNames.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomClassLayout(*It);
			}
		}

		// Unregister all structures
		for (auto It = RegisteredPropertyTypes.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomPropertyTypeLayout(*It);
			}
		}

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	FHierarchicalInstancedStyle::Shutdown();
	FHierarchicalInstancedCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(HierarchicalInstancedTabName);
}


void FHierarchicalInstancedEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(HierarchicalInstancedTabName);
}

void FHierarchicalInstancedEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FHierarchicalInstancedCommands::Get().OpenPluginWindow);
}

void FHierarchicalInstancedEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FHierarchicalInstancedCommands::Get().OpenPluginWindow);
}

TSharedRef<class SDockTab> FHierarchicalInstancedEditorModule::OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(HIOutliner::SHierarchicalInstancedOutliner)
		];
}

void FHierarchicalInstancedEditorModule::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);
	RegisteredClassNames.Add(ClassName);
	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

void FHierarchicalInstancedEditorModule::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);
	RegisteredPropertyTypes.Add(PropertyTypeName);
	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FHierarchicalInstancedEditorModule, HierarchicalInstancedEditor)
