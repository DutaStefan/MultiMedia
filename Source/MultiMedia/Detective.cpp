// Fill out your copyright notice in the Description page of Project Settings.


#include "Detective.h"
#include "ClueBookWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"


// Sets default values
ADetective::ADetective()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADetective::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
		}
	}
}

// Called every frame
void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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