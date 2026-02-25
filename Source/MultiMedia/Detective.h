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

	// ---CINEMATIC CONTROLS ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TArray<FName> TargetNPCTags;

	UPROPERTY()
	TArray<AActor*> FoundNPCs;

	/** How long the camera stays on each NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	float TimePerCharacter = 2.5f;

	/** How fast the camera travels between people */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	float TravelSpeed = 2.0f;

	/** If true, NPCs will try to rotate their heads toward the camera during their zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bNPCsShouldFaceCamera = true;

	UPROPERTY(EditAnywhere, Category = "Camera Intro")
	FVector IntroStartOffset = FVector(-400.f, 0.f, 150.f);

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ClueBookWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleBookAction;

	UPROPERTY()
	UClueBookWidget* ClueBookInstance;

	bool bIsDoingIntro = true;
	int32 CurrentTargetIndex = -1; // -1 is the initial Detective zoom
	float StateTimer = 0.0f;

	void ToggleBook();
	void FinishIntro();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};