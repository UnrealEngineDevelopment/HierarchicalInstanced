/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "HierarchicalInstancedStyle.h"

class FHierarchicalInstancedCommands : public TCommands<FHierarchicalInstancedCommands>
{
public:

	FHierarchicalInstancedCommands()
		: TCommands<FHierarchicalInstancedCommands>(TEXT("HierarchicalInstanced"), NSLOCTEXT("Contexts", "HierarchicalInstanced", "HierarchicalInstanced Plugin"), NAME_None, FHierarchicalInstancedStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};