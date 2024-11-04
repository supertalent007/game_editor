#include "TruGameObjectWidget.h"

#include "ContextMenuWidget.h"
#include "EditorUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"  // Include for EditableTextBox
#include "Components/Border.h"
#include "truworld/GameObjects/TruGameObject.h"
#include "Engine/World.h"
#include "truworld/Editor/EditorPlayerController.h"

void UTruGameObjectWidget::Setup(ATruGameObject* InGameObject, UEditorUI* EditorUI)
{
	GameObject = InGameObject;
	this->Parent = EditorUI;

	int max_characters = 40;
	if (ObjectNameText && GameObject)
	{
		FString Name = GameObject->GetName();
		if (Name.Len() > max_characters)
		{
			Name = Name.Left(max_characters) + TEXT("...");
		}
		ObjectNameText->SetText(FText::FromString(Name));
		ObjectNameEditableText->SetText(FText::FromString(Name));
	}

	UpdateBorderColor();

}

void UTruGameObjectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UTruGameObjectWidget::OnButtonClicked);
	}

	if (ObjectNameEditableText)
	{
		ObjectNameEditableText->OnTextCommitted.AddDynamic(this, &UTruGameObjectWidget::OnNameTextCommitted);
		ObjectNameEditableText->SetVisibility(ESlateVisibility::Collapsed); 
	}
}

void UTruGameObjectWidget::ToggleEditMode(bool bEnableEditMode)
{
	bIsEditMode = bEnableEditMode;
	if (ObjectNameText && ObjectNameEditableText)
	{
		ObjectNameText->SetVisibility(bIsEditMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		ObjectNameEditableText->SetVisibility(bIsEditMode ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		if (bIsEditMode)
		{
			ObjectNameEditableText->SetKeyboardFocus();
		}
	}
}

void UTruGameObjectWidget::OnElementClicked(const FString& internal_name)
{
	if(internal_name == "rename")
		ToggleEditMode(true);
}

void UTruGameObjectWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (Border && !IsSelected())
	{
		Border->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.4f, 1.0f));
	}
}

void UTruGameObjectWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	UpdateBorderColor();
}

void UTruGameObjectWidget::OnButtonClicked()
{
	if(AEditorPlayerController* Controller = Cast<AEditorPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		Controller->SetSelected(GameObject);
	}
	UpdateBorderColor();
}

bool UTruGameObjectWidget::IsSelected() const
{
	AEditorPlayerController* PlayerController = Cast<AEditorPlayerController>(GetWorld()->GetFirstPlayerController());
	return PlayerController && PlayerController->GetSelectedObject() == GameObject;
}

void UTruGameObjectWidget::UpdateBorderColor()
{
	if (!Border)
	{
		return;
	}

	if (IsSelected())
	{
		Border->SetBrushColor(FLinearColor(0.3f, 0.5f, 1.0f, 1.0f));
	}
	else
	{
		Border->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.3f, 1.0f));
	}
}

void UTruGameObjectWidget::OnNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		if (GameObject)
		{
			GameObject->Rename(*Text.ToString());
		}

		// Update the text displayed in the non-edit mode view
		if (ObjectNameText)
		{
			ObjectNameText->SetText(Text);
		}

		// Exit edit mode after committing
		ToggleEditMode(false);
	}
}

FReply UTruGameObjectWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (!bIsEditMode)
		{
			Parent->GetContextWindow()->ShowMenuAtMousePosition(this);
			//ToggleEditMode(true);
		}
        
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
