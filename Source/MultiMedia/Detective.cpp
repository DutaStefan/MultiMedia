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
	IntroCamera->SetAutoActivate(true);
}

void ADetective::BeginPlay()
{
	Super::BeginPlay();

	ActiveGameplayCamera = FindComponentByClass<UCameraComponent>();
	TArray<UCameraComponent*> Cams;
	GetComponents<UCameraComponent>(Cams);
	for (UCameraComponent* C : Cams) { if (C != IntroCamera) { ActiveGameplayCamera = C; break; } }

	if (ActiveGameplayCamera) ActiveGameplayCamera->SetActive(false);

	FoundNPCs.Empty();
	TArray<FName> MapKeys;
	NPCNameMap.GetKeys(MapKeys);

	for (FName Tag : MapKeys)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, OutActors);
		if (OutActors.Num() > 0) FoundNPCs.Add(OutActors[0]);
	}

	if (IntroCamera)
	{
		IntroCamera->SetRelativeLocation(IntroStartOffset);
		IntroCamera->bConstrainAspectRatio = true;
		IntroCamera->AspectRatio = 2.39f;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}
	OnShowSkipWidget();
}

void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentState == ECinematicState::Finished) return;

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
		if (StateTimer >= IntroDuration)
		{
			CurrentTargetIndex = 0;
			StateTimer = 0.0f;
			CurrentState = (FoundNPCs.Num() > 0) ? ECinematicState::NPCZoom : ECinematicState::ReturningHome;
		}
		break;

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
		if (FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation) < 15.0f || StateTimer > 5.0f)
		{
			FinishIntro();
			return;
		}
		break;
		}

		// Final Interp
		float Dist = FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation);
		float Speed = FMath::Clamp(Dist / 100.0f, 0.7f, 1.2f) * InterpSpeed;
		IntroCamera->SetWorldLocation(FMath::VInterpTo(IntroCamera->GetComponentLocation(), TargetLocation, DeltaTime, Speed));
		IntroCamera->SetWorldRotation(FMath::RInterpTo(IntroCamera->GetComponentRotation(), TargetRotation, DeltaTime, InterpSpeed));
	}

void ADetective::FinishIntro()
{

	OnHideSkipWidget();
	OnClearNPCName();

	CurrentState = ECinematicState::Finished;
	if (IntroCamera) IntroCamera->SetActive(false);
	if (ActiveGameplayCamera) {
		ActiveGameplayCamera->SetActive(true);
		ActiveGameplayCamera->bUsePawnControlRotation = true;
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
		PC->SetControlRotation(ActiveGameplayCamera->GetComponentRotation());
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