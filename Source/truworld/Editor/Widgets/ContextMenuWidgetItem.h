// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ContextMenuWidgetItem.generated.h"

UINTERFACE()
class UContextMenuWidgetItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRUWORLD_API IContextMenuWidgetItem
{
	GENERATED_BODY()
public:
	virtual void OnElementClicked(const FString& internal_name) {};
};
