#include "ClueItemWidget.h"
#include "Components/Button.h"

void UClueItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Default color Red
	if (HitButton)
	{
		HitButton->SetBackgroundColor(FLinearColor::Red);
	}
}

void UClueItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HitButton)
	{
		HitButton->OnClicked.AddDynamic(this, &UClueItemWidget::OnClueClicked);
	}
}

void UClueItemWidget::OnClueClicked()
{
	bIsSelected = !bIsSelected;

	if (HitButton)
	{
		FLinearColor NewColor = bIsSelected ? FLinearColor::Green : FLinearColor::Red;
		HitButton->SetBackgroundColor(NewColor);
	}

	if (OnClueSelected.IsBound())
	{
		OnClueSelected.Broadcast(bIsSelected);
	}
}