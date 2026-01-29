// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SessionSubsystem.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FpsPlayerController.generated.h"


class UResultOverlayWidget;

/**
 * 
 */
UCLASS()
class GP3_UEFPS_API AFpsPlayerController : public APlayerController
{
	GENERATED_BODY()

	
protected:
	virtual void Tick(float deltaSeconds) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleGameFinished(int winner);

	// Result表示用Widgetクラス
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ResultOverlayWidgetClass;

	// 実体（重複生成防止用）
	UPROPERTY()
	UResultOverlayWidget* ResultOverlayWidget = nullptr;

	USessionSubsystem* Session = nullptr;
};
