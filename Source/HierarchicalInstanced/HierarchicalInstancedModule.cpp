/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#include "HierarchicalInstancedModule.h"

#define LOCTEXT_NAMESPACE "HierarchicalInstancedModule"

void FHierarchicalInstancedModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FHierarchicalInstancedModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHierarchicalInstancedModule, HierarchicalInstanced)