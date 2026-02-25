#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ClueData.generated.h"

USTRUCT(BlueprintType)
struct MULTIMEDIA_API FClueInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
	FText ClueDisplayName;
};