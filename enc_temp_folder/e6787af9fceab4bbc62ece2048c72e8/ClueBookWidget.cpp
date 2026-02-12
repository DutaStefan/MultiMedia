#include "ClueBookWidget.h"
#include "ClueItemWidget.h"
#include "ClueLabelWidget.h"
#include "Blueprint/WidgetTree.h"

void UClueBookWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UClueBookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);

	TArray<UClueItemWidget*> Items;
	TArray<UClueLabelWidget*> Labels;

	for (UWidget* W : Widgets) {
		// Casting to our specific classes automatically handles the UUserWidget requirement
		if (UClueItemWidget* I = Cast<UClueItemWidget>(W)) Items.Add(I);
		if (UClueLabelWidget* L = Cast<UClueLabelWidget>(W)) Labels.Add(L);
	}

	for (UClueItemWidget* Item : Items) {
		for (UClueLabelWidget* Label : Labels) {
			if (Item->ClueIndex == Label->ClueIndex) {
				// The delegate signature now correctly matches (int32, bool)
				Item->OnClueSelected.RemoveDynamic(Label, &UClueLabelWidget::UpdateState);
				Item->OnClueSelected.AddDynamic(Label, &UClueLabelWidget::UpdateState);
			}
		}
	}
}