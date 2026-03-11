#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "ClueBookWidget.generated.h"

UCLASS()
class MULTIMEDIA_API UClueBookWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* BookSwitcher;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnShowClues;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnShowPhotos;

	UFUNCTION()
	void SwitchToClues();

	UFUNCTION()
	void SwitchToPhotos();

	UPROPERTY(meta = (BindWidget))
	class UWrapBox* PhotoGrid;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PhotoEntryClass;

	UFUNCTION(BlueprintImplementableEvent)
	void OnEntryCreated(UUserWidget* NewEntry);
};