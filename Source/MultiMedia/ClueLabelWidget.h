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

	UFUNCTION()
	void UpdateState(int32 ReceivedIndex, bool bInTurnOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Clue Logic")
	void OnVisualStateChanged(bool bInTurnOn);


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ClueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue Data")
	class UDataTable* ClueDataTable;


	virtual void NativePreConstruct() override;
};