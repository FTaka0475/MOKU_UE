// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameStartGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameFinished, int, V);

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

	void Server_GameStarted();

	void Server_GameEnd();

	UFUNCTION()
	void OnRep_GameFinish();

	bool isGameStarted() const { return bGameStarted; }
	bool isGameFinished() const { return bFinished; }

	void ResetPosition();

	void OnDominate(const FString& zoneName, int teamId);

	// UIがBPで受け取りやすいようにイベントも用意
	UPROPERTY(BlueprintAssignable)
	FOnGameFinished OnGameFinished;

protected:
	UPROPERTY(Replicated)
	bool bGameStarted;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime;

	UPROPERTY(ReplicatedUsing = OnRep_GameFinish, BlueprintReadOnly)
	int Winner = -2;

	bool bFinished;

	FTimerHandle CheckTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle GameCountTimerHandle;

	static constexpr int GameCountMax = 20;
	std::map<FString, int> DominatedTeamMap;

};
