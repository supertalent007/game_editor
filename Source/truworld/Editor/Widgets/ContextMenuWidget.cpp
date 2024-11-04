#include "ContextMenuWidget.h"

#include "ContextMenuWidgetItem.h"
#include "Components/VerticalBox.h"
#include "OptionWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

UContextMenuWidget::UContextMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildMenu();

	// Initially hide the context menu
	SetVisibility(ESlateVisibility::Collapsed);
}

void UContextMenuWidget::SetOptionsMap(const TMap<FString, FString>& NewOptionsMap)
{
	OptionsMap = NewOptionsMap;
	BuildMenu();
}

void UContextMenuWidget::BuildMenu()
{
	if (!OptionsBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("OptionsBox is not bound!"));
		return;
	}

	OptionsBox->ClearChildren();

	// Check if the OptionWidgetClass is set
	if (!OptionWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("OptionWidgetClass is not set in the ContextMenuWidget!"));
		return;
	}

	for (const TPair<FString, FString>& OptionPair : OptionsMap)
	{
		UOptionWidget* OptionWidget = CreateWidget<UOptionWidget>(this, OptionWidgetClass);
		if (OptionWidget)
		{
			OptionWidget->OptionKey = OptionPair.Key;
			if (OptionWidget->OptionText)
			{
				OptionWidget->OptionText->SetText(FText::FromString(OptionPair.Value));
			}

			OptionWidget->OnOptionClicked.AddDynamic(this, &UContextMenuWidget::OnOptionClicked);

			OptionsBox->AddChild(OptionWidget);
		}
	}
}

void UContextMenuWidget::OnOptionClicked(const FString& OptionKey)
{
	if(Item)
		((IContextMenuWidgetItem*)Item)->OnElementClicked(OptionKey);
	
	SetVisibility(ESlateVisibility::Collapsed);
	OnOptionSelected.Broadcast(OptionKey);
}

void UContextMenuWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	// Hide the context menu when the mouse leaves
	SetVisibility(ESlateVisibility::Collapsed);
}

void UContextMenuWidget::ShowMenuAtMousePosition(class IContextMenuWidgetItem* item)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) return;

	Item = (UObject*)item;
	float MouseX, MouseY;
	UWidgetLayoutLibrary::GetMousePositionScaledByDPI(PlayerController, MouseX, MouseY);
	if(UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(Slot))
	{
		slot->SetPosition({MouseX, MouseY});
	}

	SetVisibility(ESlateVisibility::Visible);
}
