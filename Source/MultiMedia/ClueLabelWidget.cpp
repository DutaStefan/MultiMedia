#include "ClueLabelWidget.h"
#include "Components/TextBlock.h"

void UClueLabelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ClueText)
	{
		if (!ClueName.IsEmpty())
		{
			ClueText->SetText(ClueName);
		}
		else
		{
			// Fallback text so it's not invisible
			ClueText->SetText(FText::FromString("New Clue"));
		}
	}
}