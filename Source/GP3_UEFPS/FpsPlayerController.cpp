// Fill out your copyright notice in the Description page of Project Settings.


#include "FpsPlayerController.h"
#include "GameStartGameState.h"
#include "Blueprint/UserWidget.h"


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
    ResultOverlayWidget = CreateWidget<UUserWidget>(
        this,
        ResultOverlayWidgetClass
    );

    ResultOverlayWidget->AddToViewport();

    bShowMouseCursor = true;
    SetInputMode(FInputModeUIOnly());
}
