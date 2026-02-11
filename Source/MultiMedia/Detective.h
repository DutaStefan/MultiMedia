// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" 
#include "Detective.generated.h"

class UInputAction;
class UInputMappingContext;
class UClueBookWidget;

UCLASS()
class MULTIMEDIA_API ADetective : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADetective();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleBookAction;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ClueBookWidgetClass;

	/** The internal instance of the book widget */
	UPROPERTY()
	UClueBookWidget* ClueBookInstance;

	/** The function that runs when you press E */
	void ToggleBook();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
