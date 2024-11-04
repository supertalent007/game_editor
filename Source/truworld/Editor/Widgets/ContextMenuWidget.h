#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionSelected, const FString&, OptionName);

/**
 * Context Menu Widget
 */
UCLASS()
class TRUWORLD_API UContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()
    
public:
	UContextMenuWidget(const FObjectInitializer& ObjectInitializer);

	// Map of internal option names to display names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	TMap<FString, FString> OptionsMap;

	// Delegate called when an option is selected
	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnOptionSelected OnOptionSelected;

	// Widget class for individual options
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	TSubclassOf<class UOptionWidget> OptionWidgetClass;

	// Function to set the options map
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void SetOptionsMap(const TMap<FString, FString>& NewOptionsMap);

	// Show the context menu at the mouse's current position
	void ShowMenuAtMousePosition(class IContextMenuWidgetItem* item);
protected:
	virtual void NativeConstruct() override;
	
	// Function to build the menu options
	void BuildMenu();

	UPROPERTY() UObject* Item;
	
	// The vertical box that contains the options
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* OptionsBox;

	// Handle mouse leave event
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void OnOptionClicked(const FString& OptionKey);
};
