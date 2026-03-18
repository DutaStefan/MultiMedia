#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClueBookWidget.h"
#include "Blueprint/UserWidget.h"
#include <InputAction.h>
#include "Detective.generated.h"


class UCameraComponent;

UENUM(BlueprintType)
enum class ECinematicState : uint8
{
	DetectiveZoom,
	NPCZoom,
	ReturningHome,
	Finished
};

UCLASS()
class MULTIMEDIA_API ADetective : public ACharacter
{
	GENERATED_BODY()

public:
	ADetective();

	// The instance editable boolean you can toggle in Blueprints to start the cutscene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bShouldStartIntro = false;

	// --- NEW PAUSE CONTROLS ---
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PauseCutscene();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void UnpauseCutscene();

protected:
	virtual void BeginPlay() override;

	// Internal tracker so we only start the intro once
	bool bHasIntroStarted = false;

	// Tracks if the cinematic is currently frozen
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	bool bIsCutscenePaused = false;

	// The function that actually kicks off the cutscene
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void StartIntro();

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ActiveGameplayCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* IntroCamera;

	// --- CINEMATIC SETTINGS ---
	// Key: The Actor Tag (e.g. Suspect1), Value: The Name to display (e.g. The Barman)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TMap<FName, FString> NPCNameMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	float TimePerCharacter = 3.0f;

	// How long the camera stares at the Detective before flying to the first NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	float DetectiveHoldDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	FVector IntroStartOffset = FVector(400.f, 0.f, 150.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	FRotator IntroStartRotation = FRotator(0.f, 180.f, 0.f);

	// Acts as a multiplier for the linear units-per-second speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Intro")
	float InterpSpeed = 1.5f;

	// UI Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Photo")
	void OnPhotoTaken(FName NPCID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Photo")
	void OnOpenPhotoUI(FName NPCID);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo")
	FName LastPhotographedID;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cinematic")
	void OnShowNPCName(const FString& NewName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cinematic")
	void OnClearNPCName();

	UFUNCTION(BlueprintCallable, Category = "Input")
	FText GetSkipKeyName();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ClueBookWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleBookAction;

	UPROPERTY()
	UClueBookWidget* ClueBookInstance;

	void ToggleBook();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SkipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Photo")
	UInputAction* TakePhotoAction;

	UFUNCTION(BlueprintImplementableEvent)
	void OnShowSkipWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHideSkipWidget();

	void SkipCutscene();
	void TakePhoto();
	void OpenPhoto();

private:
	ECinematicState CurrentState = ECinematicState::DetectiveZoom;

	UPROPERTY()
	TArray<AActor*> FoundNPCs;

	int32 CurrentTargetIndex = -1;
	int32 LastTriggeredIndex = -2;
	float StateTimer = 0.0f;
	bool bIsPausingOnNPC = false;

	void FinishIntro();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo")
	TArray<FName> CapturedNPCIDs;
};