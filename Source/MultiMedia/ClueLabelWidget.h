#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClueData.h"
#include "ClueLabelWidget.generated.h"

UCLASS()
class MULTIMEDIA_API UClueLabelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 ClueIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	class UDataTable* ClueDataTable;

	UFUNCTION()
	void UpdateState(int32 ReceivedIndex, bool bInTurnOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Clue Logic")
	void OnVisualStateChanged(bool bInTurnOn);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ClueText;

	virtual void NativePreConstruct() override;
};