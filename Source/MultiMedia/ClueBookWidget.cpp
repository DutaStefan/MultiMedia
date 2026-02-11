// Fill out your copyright notice in the Description page of Project Settings.


#include "ClueBookWidget.h"
#include "ClueItemWidget.h"
#include "ClueLabelWidget.h"
#include "Blueprint/WidgetTree.h"

void UClueBookWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<UClueItemWidget*> Items;
    TArray<UClueLabelWidget*> Labels;

    // Use WidgetTree to find every child
    WidgetTree->ForEachWidget([&](UWidget* Widget) {
        if (UClueItemWidget* Item = Cast<UClueItemWidget>(Widget)) {
            Items.Add(Item);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Found Item: %s"), *Item->ClueName.ToString()));
        }
        if (UClueLabelWidget* Label = Cast<UClueLabelWidget>(Widget)) {
            Labels.Add(Label);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Found Label: %s"), *Label->ClueName.ToString()));
        }
        });

    // Match them
    for (UClueItemWidget* Item : Items) {
        for (UClueLabelWidget* Label : Labels) {
            if (Item->ClueName.EqualTo(Label->ClueName)) {
                Item->OnClueSelected.AddDynamic(Label, &UClueLabelWidget::ToggleLine);
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SUCCESS: Linked Pair '1'!"));
            }
        }
    }
}

void UClueItemWidget::OnClueClicked()
{
    bIsSelected = !bIsSelected;

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Button Clicked!"));

    if (OnClueSelected.IsBound()) {
        OnClueSelected.Broadcast(bIsSelected);
    }
    else {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Error: Nothing is listening to this button!"));
    }
}

