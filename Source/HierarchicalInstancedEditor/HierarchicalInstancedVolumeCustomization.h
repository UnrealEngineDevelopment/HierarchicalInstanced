/****************************************************************************
  Copyright (c) 2019 libo All Rights Reserved.

  losemymind.libo@gmail.com

****************************************************************************/
#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class HIERARCHICALINSTANCEDEDITOR_API FHIVolumeCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	FReply BuildHIActor_OnClicked();
public:

private:
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
};

