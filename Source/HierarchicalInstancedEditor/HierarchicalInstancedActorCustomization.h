/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class HIERARCHICALINSTANCEDEDITOR_API FHIActorCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	FReply RefreshInstances_OnClicked();
	FReply RevertActorsToLevel_OnClicked();
private:
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
};