#include "OptionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UOptionWidget::HandleButtonClicked);
	}
}

void UOptionWidget::HandleButtonClicked()
{
	OnOptionClicked.Broadcast(OptionKey);
}
