// Fill out your copyright notice in the Description page of Project Settings.


#include "FpsPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameStartGameState.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ResultOverlayWidget.h"



void AFpsPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AGameStartGameState* GS = GetWorld()->GetGameState<AGameStartGameState>();
    if(!GS)
        return;

    // ===== GameState の通知に Bind =====
    GS->OnGameFinished.AddDynamic(this, &AFpsPlayerController::HandleGameFinished);
}


void AFpsPlayerController::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

    AGameStartGameState* GS = GetWorld()->GetGameState<AGameStartGameState>();
    if (!GS)
        return;

    const bool bStarted = GS->isGameStarted() && !GS->isGameFinished();

    // カウントダウン中は一切動かさない
    SetIgnoreMoveInput(!bStarted);
    SetIgnoreLookInput(!bStarted);
}


void AFpsPlayerController::HandleGameFinished(int winner)
{
    FString str = FString::Printf(TEXT("HandleGameFinished winner - %d"), winner);
    UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Purple, 8.f, TEXT("None"));
    ResultOverlayWidget = CreateWidget<UResultOverlayWidget>(
        this,
        ResultOverlayWidgetClass
    );

    if(ResultOverlayWidget != nullptr)
    {
        ResultOverlayWidget->AddToViewport();
        bShowMouseCursor = true;
        SetInputMode(FInputModeUIOnly());
        FString name = FString::Printf(TEXT("%d"), winner);
        name = Session->GetUserName();
        ResultOverlayWidget->SetWinner(name);
    }
}
