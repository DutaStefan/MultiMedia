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

	IntroCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IntroCamera"));
	IntroCamera->SetupAttachment(RootComponent);
	IntroCamera->SetAutoActivate(true);
}

void ADetective::BeginPlay()
{
	Super::BeginPlay();

	ActiveGameplayCamera = FindComponentByClass<UCameraComponent>();

	TArray<UCameraComponent*> Components;
	GetComponents<UCameraComponent>(Components);
	for (UCameraComponent* Cam : Components)
	{
		if (Cam != IntroCamera)
		{
			ActiveGameplayCamera = Cam;
			break;
		}
	}

	if (ActiveGameplayCamera)
	{
		ActiveGameplayCamera->SetActive(false);
	}

	CurrentState = ECinematicState::DetectiveZoom;
	CurrentTargetIndex = -1;
	StateTimer = 0.0f;
	FoundNPCs.Empty();

	for (FName Tag : TargetNPCTags)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, OutActors);
		if (OutActors.Num() > 0) FoundNPCs.Add(OutActors[0]);
	}

	if (IntroCamera)
	{
		IntroCamera->SetRelativeLocation(IntroStartOffset);
		IntroCamera->bUsePawnControlRotation = false;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true); // Stop mouse during intro
	}
}

void ADetective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ECinematicState::Finished) return;

	StateTimer += DeltaTime;
	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	if (ActiveGameplayCamera)
	{
		TargetLocation = ActiveGameplayCamera->GetComponentLocation();
		TargetRotation = ActiveGameplayCamera->GetComponentRotation();
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
		if (FoundNPCs.IsValidIndex(CurrentTargetIndex))
		{
			AActor* NPC = FoundNPCs[CurrentTargetIndex];
			FVector EyesLocation = NPC->GetActorLocation() + FVector(0, 0, 65.f);

			if (USkeletalMeshComponent* NPCMesh = NPC->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (NPCMesh->DoesSocketExist(TEXT("head")))
					EyesLocation = NPCMesh->GetSocketLocation(TEXT("head"));
			}

			TargetLocation = EyesLocation + (NPC->GetActorForwardVector() * 150.f);
			TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, EyesLocation);

			if (StateTimer >= TimePerCharacter)
			{
				CurrentTargetIndex++;
				StateTimer = 0.0f;
				if (!FoundNPCs.IsValidIndex(CurrentTargetIndex)) CurrentState = ECinematicState::ReturningHome;
			}
		}
		break;

	case ECinematicState::ReturningHome:
		if (FVector::Dist(IntroCamera->GetComponentLocation(), TargetLocation) < 10.0f || StateTimer > 5.0f)
		{
			FinishIntro();
			return;
		}
		break;

	default:
		break;
	}

	FVector NewLoc = FMath::VInterpTo(IntroCamera->GetComponentLocation(), TargetLocation, DeltaTime, InterpSpeed);
	FRotator NewRot = FMath::RInterpTo(IntroCamera->GetComponentRotation(), TargetRotation, DeltaTime, InterpSpeed);
	IntroCamera->SetWorldLocationAndRotation(NewLoc, NewRot);
}

void ADetective::FinishIntro()
{
	CurrentState = ECinematicState::Finished;

	if (IntroCamera) IntroCamera->SetActive(false);

	if (ActiveGameplayCamera)
	{
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


				PC->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}