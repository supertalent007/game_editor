#pragma once

#include "CoreMinimal.h"
#include "ContextMenuWidgetItem.h"
#include "Blueprint/UserWidget.h"
#include "TruGameObjectWidget.generated.h"

UCLASS()
class TRUWORLD_API UTruGameObjectWidget : public UUserWidget, public IContextMenuWidgetItem
{
	GENERATED_BODY()

public:
	void Setup(class ATruGameObject* InGameObject, class UEditorUI* EditorUI);
	void UpdateBorderColor();
	void ToggleEditMode(bool bEnableEditMode);
	
	void OnElementClicked(const FString& internal_name) override;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UPROPERTY() TObjectPtr<UEditorUI> Parent;
	UPROPERTY(meta = (BindWidget)) class UButton* Button;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* ObjectNameText;
	UPROPERTY(meta = (BindWidget)) class UEditableTextBox* ObjectNameEditableText;
	UPROPERTY(meta = (BindWidget)) class UBorder* Border;

private:
	UFUNCTION()
	void OnButtonClicked();
	UFUNCTION()
	void OnNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	// Add this declaration in the class header file
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	bool IsSelected() const;
	
	UPROPERTY()
	ATruGameObject* GameObject;

	bool bIsEditMode = false;  // New flag to track edit mode
};
