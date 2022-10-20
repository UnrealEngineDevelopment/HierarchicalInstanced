/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class HIERARCHICALINSTANCED_API FHierarchicalInstancedModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
