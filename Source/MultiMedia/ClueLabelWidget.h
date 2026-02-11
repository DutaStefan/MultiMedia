#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClueLabelWidget.generated.h"

class UTextBlock;

UCLASS()
class MULTIMEDIA_API UClueLabelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue Config")
	FText ClueName;

	UFUNCTION(BlueprintImplementableEvent, Category = "Clue Logic")
	void ToggleLine(bool bTurnOn);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ClueText;

	virtual void NativePreConstruct() override;
};