#include "Detective.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ClueBookWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ADetective::ADetective()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Internal FPS Camera
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(RootComponent);
	FPSCamera->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
	FPSCamera->bUsePawnControlRotation = true;
	FPSCamera->SetAutoActivate(false);

	// 2. Intro Camera
	IntroCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IntroCamera"));
	IntroCamera->SetupAttachment(RootComponent);
	IntroCamera->SetAutoActivate(true);
}

void ADetective::BeginPlay()
{
	Super::BeginPlay();

	if (bIsDoingIntro && IntroCamera)
	{
		IntroCamera->SetRelativeLocation(IntroStartOffset);

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetIgnoreMoveInput(true);
		}
	}
}

void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDoingIntro && IntroCamera && FPSCamera)
	{
		IntroTimer += DeltaTime;

		FVector CurrentLoc = IntroCamera->GetRelativeLocation();
		FVector TargetLoc = FPSCamera->GetRelativeLocation();

		// Smoothly move using the editable InterpSpeed
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, InterpSpeed);
		IntroCamera->SetRelativeLocation(NewLoc);

		// Stop intro when close or time expires
		if (FVector::Dist(NewLoc, TargetLoc) < 5.0f || IntroTimer >= IntroDuration)
		{
			IntroCamera->SetActive(false);
			FPSCamera->SetActive(true);
			bIsDoingIntro = false;

			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->SetIgnoreMoveInput(false);
			}
		}
	}
}

// Called to bind functionality to input
void ADetective::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// This connects the "ToggleBookAction" variable to the "ToggleBook" function
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Make sure ToggleBookAction is selected in the Blueprint, otherwise this crashes!
		if (ToggleBookAction)
		{
			EnhancedInputComponent->BindAction(ToggleBookAction, ETriggerEvent::Started, this, &ADetective::ToggleBook);
		}
	}
}

void ADetective::ToggleBook()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Input Received!"));

	if (!ClueBookWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClueBookWidgetClass is not set in BP_Detective!"));
		return;
	}

	if (!ClueBookInstance)
	{
		ClueBookInstance = CreateWidget<UClueBookWidget>(GetWorld(), ClueBookWidgetClass);
	}

	if (ClueBookInstance)
	{
		if (ClueBookInstance->IsInViewport())
		{
			// --- CLOSE BOOK ---
			ClueBookInstance->RemoveFromParent();

			// Hide mouse cursor and lock to game
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
		else
		{
			// --- OPEN BOOK ---
			ClueBookInstance->AddToViewport();

			// Show mouse cursor and allow clicking
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = true;

				// This allows both UI clicks and Game movement (optional)
				// Use FInputModeUIOnly() if you want the player to stop moving while reading.
				PC->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}