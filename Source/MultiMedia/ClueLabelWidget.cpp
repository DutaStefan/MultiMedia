#include "ClueLabelWidget.h"
#include "Components/TextBlock.h"

void UClueLabelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UClueLabelWidget::UpdateState(int32 ReceivedIndex, bool bInTurnOn)
{
	if (ReceivedIndex == ClueIndex)
	{
		OnVisualStateChanged(bInTurnOn);
	}
}