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
	bIsActive = !bIsActive;

	UE_LOG(LogTemp, Warning, TEXT("The button has been clicked"));

	OnClueSelected.Broadcast(ClueIndex, bIsActive);

	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}