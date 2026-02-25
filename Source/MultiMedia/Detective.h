#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Detective.generated.h"

class UCameraComponent;
class UInputAction;
class UClueBookWidget;

UCLASS()
class MULTIMEDIA_API ADetective : public ACharacter
{
	GENERATED_BODY()

public:
	ADetective();

protected:
	virtual void BeginPlay() override;

	/** The internal gameplay camera - Hidden from defaults to avoid conflicts */
	UPROPERTY()
	UCameraComponent* FPSCamera;

	/** The cinematic intro camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* IntroCamera;

	// --- EDITABLE INTRO SETTINGS ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	float IntroDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	FVector IntroStartOffset = FVector(-400.f, 0.f, 150.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	float InterpSpeed = 1.2f;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ClueBookWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleBookAction;

	UPROPERTY()
	UClueBookWidget* ClueBookInstance;

	bool bIsDoingIntro = true;
	float IntroTimer = 0.0f;

	void ToggleBook();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};