#include "ClueLabelWidget.h"
#include "Components/TextBlock.h"

void UClueLabelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (ClueText && ClueDataTable)
	{
		FName RowName = FName(*FString::FromInt(ClueIndex));
		FClueInfo* Data = ClueDataTable->FindRow<FClueInfo>(RowName, TEXT("LabelLookup"));
		if (Data) {
			ClueText->SetText(Data->ClueDisplayName);
		}
		else {
			ClueText->SetText(FText::AsNumber(ClueIndex));
		}
	}
}

void UClueLabelWidget::UpdateState(int32 ReceivedIndex, bool bInTurnOn)
{
	if (ReceivedIndex == ClueIndex)
	{
		// This triggers the red event node in your Blueprint
		OnVisualStateChanged(bInTurnOn);
	}
}