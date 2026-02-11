#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClueItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClueSelected, bool, bIsSelected);

class UButton;

UCLASS()
class MULTIMEDIA_API UClueItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue Config")
	FText ClueName;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnClueSelected OnClueSelected;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* HitButton;

	bool bIsSelected = false;

	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	UFUNCTION()
	void OnClueClicked();
};