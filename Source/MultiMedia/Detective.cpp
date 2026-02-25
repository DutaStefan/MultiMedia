#include "Detective.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ClueBookWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>

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

	for (FName Tag : TargetNPCTags)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, OutActors);

		if (OutActors.Num() > 0)
		{
			FoundNPCs.Add(OutActors[0]); // Add the first actor found with that tag
		}
	}

	if (IntroCamera)
	{
		IntroCamera->SetRelativeLocation(IntroStartOffset);
		IntroCamera->bUsePawnControlRotation = false; // We control rotation manually now
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		PC->SetIgnoreMoveInput(true);
}

void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDoingIntro || !IntroCamera || !FPSCamera) return;

	StateTimer += DeltaTime;

	FVector TargetLocation;
	FRotator TargetRotation;

	if (CurrentTargetIndex == -1)
	{
		// STAGE 1: Zooming into Detective
		TargetLocation = FPSCamera->GetComponentLocation();
		TargetRotation = FPSCamera->GetComponentRotation();

		if (StateTimer >= 3.0f) // Initial zoom duration
		{
			CurrentTargetIndex = 0;
			StateTimer = 0.0f;
		}
	}
	else if (FoundNPCs.IsValidIndex(CurrentTargetIndex))
	{
		AActor* CurrentNPC = FoundNPCs[CurrentTargetIndex];
		if (CurrentNPC)
		{
			// We get the Actor's eyes/head height dynamically
			float HalfHeight = CurrentNPC->GetSimpleCollisionHalfHeight();
			FVector HeadLevelOffset = FVector(0, 0, HalfHeight * 0.8f);

			// Position camera 120 units in front of the NPC's face
			FVector NPCLocation = CurrentNPC->GetActorLocation();
			FVector Forward = CurrentNPC->GetActorForwardVector();

			TargetLocation = NPCLocation + (Forward * 120.0f) + HeadLevelOffset;

			// Make the camera look directly at the head level
			TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, NPCLocation + HeadLevelOffset);
		}

		if (StateTimer >= TimePerCharacter)
		{
			CurrentTargetIndex++;
			StateTimer = 0.0f;
		}
	}
	else
	{
		// STAGE 3: Return to Detective and Finish
		TargetLocation = FPSCamera->GetComponentLocation();
		TargetRotation = FPSCamera->GetComponentRotation();

		if (FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation) < 10.0f)
		{
			FinishIntro();
		}
	}

	// Smooth Movement and Rotation
	FVector NewLoc = FMath::VInterpTo(IntroCamera->GetComponentLocation(), TargetLocation, DeltaTime, TravelSpeed);
	FRotator NewRot = FMath::RInterpTo(IntroCamera->GetComponentRotation(), TargetRotation, DeltaTime, TravelSpeed);

	IntroCamera->SetWorldLocationAndRotation(NewLoc, NewRot);
}

void ADetective::FinishIntro()
{
	IntroCamera->SetActive(false);
	FPSCamera->SetActive(true);
	bIsDoingIntro = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetControlRotation(FPSCamera->GetComponentRotation());
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