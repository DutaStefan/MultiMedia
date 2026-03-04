#include "ClueLabelWidget.h"
#include "Components/TextBlock.h"
#include "ClueData.h"

void UClueLabelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ClueDataTable && ClueText)
	{
		FName RowName = FName(*FString::FromInt(ClueIndex));

		static const FString ContextString(TEXT("Clue UI Context"));
		FClueInfo* FoundRow = ClueDataTable->FindRow<FClueInfo>(RowName, ContextString);

		if (FoundRow)
		{
			ClueText->SetText(FoundRow->ClueDisplayName);
		}
		else
		{
			ClueText->SetText(FText::FromString("Clue Row Not Found"));
		}
	}
}

void UClueLabelWidget::UpdateState(int32 ReceivedIndex, bool bInTurnOn)
{
	if (ReceivedIndex == ClueIndex)
	{
		OnVisualStateChanged(bInTurnOn);
	}
}