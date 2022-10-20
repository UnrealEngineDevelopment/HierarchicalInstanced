/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedCommands.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedCommands"

void FHierarchicalInstancedCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "HierarchicalInstanced", "Bring up HierarchicalInstanced window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
