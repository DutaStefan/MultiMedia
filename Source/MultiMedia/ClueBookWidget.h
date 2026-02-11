#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClueBookWidget.generated.h"

UCLASS()
class MULTIMEDIA_API UClueBookWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// This runs when the widget is created
	virtual void NativeConstruct() override;
};