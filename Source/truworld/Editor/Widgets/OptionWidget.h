#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionClicked, const FString&, OptionKey);

/**
 * Option Widget for the context menu
 */
UCLASS()
class TRUWORLD_API UOptionWidget : public UUserWidget
{
	GENERATED_BODY()
    
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* OptionButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* OptionText;

	// Internal name of the option
	FString OptionKey;

	// Delegate to notify when the option is clicked
	UPROPERTY(BlueprintAssignable, Category = "Option")
	FOnOptionClicked OnOptionClicked;

	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleButtonClicked();
};
