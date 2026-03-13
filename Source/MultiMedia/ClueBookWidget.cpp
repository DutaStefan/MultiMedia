#include "ClueBookWidget.h"
#include "ClueItemWidget.h"
#include "ClueLabelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/WrapBox.h"
#include "Detective.h"

void UClueBookWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BtnShowClues)
		BtnShowClues->OnClicked.AddDynamic(this, &UClueBookWidget::SwitchToClues);

	if (BtnShowPhotos)
		BtnShowPhotos->OnClicked.AddDynamic(this, &UClueBookWidget::SwitchToPhotos);
}

void UClueBookWidget::SwitchToClues()
{
	if (BookSwitcher) BookSwitcher->SetActiveWidgetIndex(0);
}

void UClueBookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SwitchToClues();

	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);

	TArray<UClueItemWidget*> Items;
	TArray<UClueLabelWidget*> Labels;

	for (UWidget* W : Widgets) {
		if (UClueItemWidget* I = Cast<UClueItemWidget>(W)) Items.Add(I);
		if (UClueLabelWidget* L = Cast<UClueLabelWidget>(W)) Labels.Add(L);
	}

	for (UClueItemWidget* Item : Items) {
		for (UClueLabelWidget* Label : Labels) {
			if (Item->ClueIndex == Label->ClueIndex) {
				Item->OnClueSelected.RemoveDynamic(Label, &UClueLabelWidget::UpdateState);
				Item->OnClueSelected.AddDynamic(Label, &UClueLabelWidget::UpdateState);
			}
		}
	}
}	

void UClueBookWidget::SwitchToPhotos()
{
	if (BookSwitcher) BookSwitcher->SetActiveWidgetIndex(1);
	if (!PhotoGrid || !PhotoEntryClass) return;

	PhotoGrid->ClearChildren();

	// Get the Detective character to see what they've photographed
	if (ADetective* Det = Cast<ADetective>(GetOwningPlayerPawn()))
	{
		for (FName ID : Det->CapturedNPCIDs)
		{
			UUserWidget* NewEntry = CreateWidget<UUserWidget>(this, PhotoEntryClass);
			if (NewEntry)
			{
				FProperty* Prop = NewEntry->GetClass()->FindPropertyByName(TEXT("NPCID"));
				if (Prop)
				{
					FName* ValuePtr = Prop->ContainerPtrToValuePtr<FName>(NewEntry);
					if (ValuePtr)
					{
						*ValuePtr = ID;
					}
				}

				PhotoGrid->AddChildToWrapBox(NewEntry);
				// This triggers the event in your WBP_ClueBook Blueprint
				OnEntryCreated(NewEntry);
			}

		}
	}
}