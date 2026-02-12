#include "ClueItemWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UClueItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (HitButton)
	{
		HitButton->OnClicked.AddUniqueDynamic(this, &UClueItemWidget::OnClueClicked);
	}
}

void UClueItemWidget::OnClueClicked()
{
	// Logic: Flip the local state, then tell the label exactly what that state is
	bIsActive = !bIsActive;

	OnClueSelected.Broadcast(ClueIndex, bIsActive);

	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}