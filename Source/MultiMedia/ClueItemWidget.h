#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClueItemWidget.generated.h"

// Delegate declared BEFORE the class
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClueSelected, int32, SelectedIndex, bool, bInTurnOn);

UCLASS()
class MULTIMEDIA_API UClueItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnClueSelected OnClueSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 ClueIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundBase* ClickSound;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* HitButton;

	// This stores the current state to send to the label
	bool bIsActive = false;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClueClicked();
};