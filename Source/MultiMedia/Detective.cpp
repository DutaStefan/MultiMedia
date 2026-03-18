#include "Detective.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ClueBookWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "InputMappingContext.h"
#include <Misc/OutputDeviceNull.h>

ADetective::ADetective()
{
	PrimaryActorTick.bCanEverTick = true;

	IntroCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IntroCamera"));
	IntroCamera->SetupAttachment(RootComponent);
	IntroCamera->SetAutoActivate(false);

	// Ensure the camera can never drift from player mouse movements
	IntroCamera->bUsePawnControlRotation = false;
}

void ADetective::BeginPlay()
{
	Super::BeginPlay();

	// 1. Identify the cameras
	ActiveGameplayCamera = nullptr;
	TArray<UCameraComponent*> Cams;
	GetComponents<UCameraComponent>(Cams);
	for (UCameraComponent* C : Cams)
	{
		if (C != IntroCamera)
		{
			ActiveGameplayCamera = C;
			break;
		}
	}

	// 2. Force the correct camera states immediately on spawn
	if (IntroCamera)
	{
		IntroCamera->SetActive(false);
		IntroCamera->bUsePawnControlRotation = false;
		IntroCamera->bConstrainAspectRatio = true;
		IntroCamera->AspectRatio = 2.39f;
	}

	if (ActiveGameplayCamera)
	{
		ActiveGameplayCamera->SetActive(true);
	}

	// 3. Setup the NPC Targets for later
	FoundNPCs.Empty();
	TArray<FName> MapKeys;
	NPCNameMap.GetKeys(MapKeys);

	for (FName Tag : MapKeys)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, OutActors);
		if (OutActors.Num() > 0) FoundNPCs.Add(OutActors[0]);
	}
}

void ADetective::StartIntro()
{
	bHasIntroStarted = true;

	// Calculate the exact world space coordinates
	FVector StartLoc = GetActorTransform().TransformPosition(IntroStartOffset);
	FRotator StartRot = GetActorTransform().TransformRotation(IntroStartRotation.Quaternion()).Rotator();

	if (IntroCamera)
	{
		// Detach and enforce absolute world coordinates so the FP character can't drag it
		IntroCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		IntroCamera->SetUsingAbsoluteLocation(true);
		IntroCamera->SetUsingAbsoluteRotation(true);
		IntroCamera->SetWorldLocationAndRotation(StartLoc, StartRot);
		IntroCamera->SetActive(true);
	}

	// Swap cameras over to the cinematic view
	if (ActiveGameplayCamera) ActiveGameplayCamera->SetActive(false);

	// Lock player controls
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	OnShowSkipWidget();
}

void ADetective::PauseCutscene()
{
	bIsCutscenePaused = true;
}

void ADetective::UnpauseCutscene()
{
	bIsCutscenePaused = false;
}

void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Wait for the instance editable boolean to become true via Blueprints
	if (!bHasIntroStarted)
	{
		if (bShouldStartIntro)
		{
			StartIntro();
		}
		else
		{
			// Exit tick early; do not process cinematic math until triggered
			return;
		}
	}

	if (CurrentState == ECinematicState::Finished) return;

	// --- FREEZE EVERYTHING IF PAUSED ---
	if (bIsCutscenePaused) return;

	StateTimer += DeltaTime;

	// Default Fallback (Player Position)
	FVector TargetLocation = (ActiveGameplayCamera) ? ActiveGameplayCamera->GetComponentLocation() : GetActorLocation();
	FRotator TargetRotation = (ActiveGameplayCamera) ? ActiveGameplayCamera->GetComponentRotation() : GetActorRotation();

	// Smooth Letterbox Exit
	if (CurrentState == ECinematicState::ReturningHome)
	{
		IntroCamera->AspectRatio = FMath::FInterpTo(IntroCamera->AspectRatio, 1.77f, DeltaTime, 1.0f);
		if (IntroCamera->AspectRatio >= 1.76f) IntroCamera->bConstrainAspectRatio = false;
	}

	switch (CurrentState)
	{
	case ECinematicState::DetectiveZoom:
	{
		// TargetLocation defaults to the Detective's head from the fallback above, so it will physically zoom in.
		// However, we OVERRIDE the TargetRotation to force the camera to stare at the Detective, preventing it from spinning away!
		TargetRotation = UKismetMathLibrary::FindLookAtRotation(IntroCamera->GetComponentLocation(), TargetLocation);

		if (StateTimer >= DetectiveHoldDuration)
		{
			CurrentTargetIndex = 0;
			StateTimer = 0.0f;
			CurrentState = (FoundNPCs.Num() > 0) ? ECinematicState::NPCZoom : ECinematicState::ReturningHome;
		}
		break;
	}

	case ECinematicState::NPCZoom:
	{
		if (FoundNPCs.IsValidIndex(CurrentTargetIndex))
		{
			AActor* NPC = FoundNPCs[CurrentTargetIndex];
			if (!NPC) { CurrentTargetIndex++; break; }

			// 1. Setup Camera Targets
			FVector Eyes = NPC->GetActorLocation() + FVector(0, 0, 70.f);
			if (USkeletalMeshComponent* MeshComp = NPC->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (MeshComp->DoesSocketExist(TEXT("head"))) Eyes = MeshComp->GetSocketLocation(TEXT("head"));
			}
			TargetLocation = Eyes + (NPC->GetActorForwardVector() * 140.f) + (NPC->GetActorRightVector() * 40.f);
			TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, Eyes);

			// 2. Check Distance to see if we have "Arrived"
			float DistToTarget = FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation);
			float RotToTarget = FVector::Dist(IntroCamera->GetComponentRotation().Vector(), TargetRotation.Vector());

			if (DistToTarget < 15.0f && !bIsPausingOnNPC)
			{
				// --- ARRIVAL MOMENT ---
				bIsPausingOnNPC = true;
				StateTimer = 0.5f; // Reset timer to 0 so we get the FULL duration

				FString DisplayName = "Unknown Suspect";
				for (FName T : NPC->Tags) {
					if (NPCNameMap.Contains(T)) {
						DisplayName = NPCNameMap[T];
						break;
					}
				}
				OnShowNPCName(DisplayName); // Show the UI
			}

			// 3. Only count the timer if we are in the Pause phase
			if (bIsPausingOnNPC)
			{
				StateTimer += DeltaTime;

				if (StateTimer >= TimePerCharacter)
				{
					OnClearNPCName();   // Clear UI
					bIsPausingOnNPC = false; // Reset for next traveler
					CurrentTargetIndex++;
					StateTimer = 0.0f;

					if (!FoundNPCs.IsValidIndex(CurrentTargetIndex))
					{
						CurrentState = ECinematicState::ReturningHome;
					}
				}
			}
		}
		break;
	}

	case ECinematicState::ReturningHome:
		// Target the exact world position of the First Person Camera so it lines up perfectly
		if (ActiveGameplayCamera)
		{
			TargetLocation = ActiveGameplayCamera->GetComponentLocation();
			TargetRotation = ActiveGameplayCamera->GetComponentRotation();
		}

		if (FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation) < 15.0f || StateTimer > 5.0f)
		{
			FinishIntro();
			return;
		}
		break;
	}

	// Final Interp - Swapped to Constant Speed!
	// We multiply InterpSpeed to turn it into "Units Per Second" and "Degrees Per Second"
	float LinearLocSpeed = InterpSpeed * 150.0f;
	float LinearRotSpeed = InterpSpeed * 60.0f;

	IntroCamera->SetWorldLocation(FMath::VInterpConstantTo(IntroCamera->GetComponentLocation(), TargetLocation, DeltaTime, LinearLocSpeed));
	IntroCamera->SetWorldRotation(FMath::RInterpConstantTo(IntroCamera->GetComponentRotation(), TargetRotation, DeltaTime, LinearRotSpeed));
}

void ADetective::FinishIntro()
{
	OnHideSkipWidget();
	OnClearNPCName();

	CurrentState = ECinematicState::Finished;

	if (IntroCamera)
	{
		IntroCamera->SetActive(false);
		// Clean up our absolute locks and re-attach the camera to the character
		IntroCamera->SetUsingAbsoluteLocation(false);
		IntroCamera->SetUsingAbsoluteRotation(false);
		IntroCamera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	if (ActiveGameplayCamera)
	{
		ActiveGameplayCamera->SetActive(true);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}
}

// Called to bind functionality to input
void ADetective::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ToggleBookAction)
		{
			EnhancedInputComponent->BindAction(ToggleBookAction, ETriggerEvent::Started, this, &ADetective::ToggleBook);
		}

		if (SkipAction)
		{
			EnhancedInputComponent->BindAction(SkipAction, ETriggerEvent::Started, this, &ADetective::SkipCutscene);
		}

		if (TakePhotoAction)
		{
			EnhancedInputComponent->BindAction(TakePhotoAction, ETriggerEvent::Started, this, &ADetective::TakePhoto);
		}
	}
}

FText ADetective::GetSkipKeyName()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (SkipAction)
			{
				// Get all keys mapped to IA_Skip
				TArray<FKey> BoundKeys = Subsystem->QueryKeysMappedToAction(SkipAction);
				if (BoundKeys.Num() > 0)
				{
					// Return the first key (e.g., "Space Bar" or "X")
					return BoundKeys[0].GetDisplayName();
				}
			}
		}
	}
	return FText::FromString("None");
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
				PC->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ADetective::SkipCutscene()
{
	if (CurrentState == ECinematicState::Finished) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Cutscene Skipped!"));

	OnClearNPCName();
	OnHideSkipWidget();
	FinishIntro();
}

void ADetective::TakePhoto()
{
	if (CurrentState != ECinematicState::Finished) return;

	FVector Start = ActiveGameplayCamera->GetComponentLocation();
	FVector End = Start + (ActiveGameplayCamera->GetForwardVector() * 500.0f); // 5 meters range

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			// Check if the hit actor has a tag we recognize from our NPCNameMap
			for (auto& Elem : NPCNameMap)
			{
				if (HitActor->ActorHasTag(Elem.Key))
				{
					CapturedNPCIDs.AddUnique(Elem.Key);

					LastPhotographedID = Elem.Key;
					OnPhotoTaken(LastPhotographedID);

					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
						FString::Printf(TEXT("Photo taken of: %s"), *Elem.Value));
					return;
				}
			}
		}
	}
}

void ADetective::OpenPhoto()
{
	if (LastPhotographedID.IsNone())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("No photo taken yet!"));
		return;
	}

	// Call the Blueprint event to show the UI
	OnOpenPhotoUI(LastPhotographedID);
}