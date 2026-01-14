// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameStartGameState.generated.h"

/**
 * 
 */
UCLASS()
class GP3_UEFPS_API AGameStartGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void CheckPlayerCount();

	void StartCountdown(int32 InSeconds);

	void TickCountdown();

	void TickGameCount();

	UFUNCTION()
	void OnRep_RemainingTime();

	void OnRep_GameStarted();

	void OnRep_GameEnd();

	bool isGameStarted() const { return bGameStarted; }

	void ResetPosition();

	void OnDominate(const FString& zoneName, int teamId);

protected:
	UPROPERTY(Replicated)
	bool bGameStarted;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime;

	FTimerHandle CheckTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle GameCountTimerHandle;

	static constexpr int GameCountMax = 60;
	std::map<FString, int> DominatedTeamMap;
};
