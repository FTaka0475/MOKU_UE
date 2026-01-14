// Copyright Epic Games, Inc. All Rights Reserved.

#include "GP3_UEFPSGameMode.h"
#include "GP3_UEFPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GP3PlayerState.h"

AGP3_UEFPSGameMode::AGP3_UEFPSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = AGP3PlayerState::StaticClass();
}

void AGP3_UEFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// サーバー以外では処理しない
	if (!HasAuthority()) return;

	// スタンドアロン（ネットワークなし）なら無視
	if (GetNetMode() == NM_Standalone) return;

	AGP3PlayerState* PS = NewPlayer->GetPlayerState<AGP3PlayerState>();
	if (!PS) return;

	PS->TeamId = nextTeamId;
	nextTeamId++;

	UE_LOG(LogTemp, Log, TEXT("TeamId=%d"), PS->TeamId);
}
