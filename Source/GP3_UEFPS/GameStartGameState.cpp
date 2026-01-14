// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStartGameState.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "LobbyGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"


void AGameStartGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGameStartGameState, RemainingTime);
    DOREPLIFETIME(AGameStartGameState, bGameStarted);
}


void AGameStartGameState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority()) // サーバーのみで実行
    {
        GetWorld()->GetTimerManager().SetTimer(CheckTimerHandle, this,
            &AGameStartGameState::CheckPlayerCount, 0.5f, true);
    }
}


void AGameStartGameState::CheckPlayerCount()
{
    if (bGameStarted)
        return;

    const int32 NumPlayers = PlayerArray.Num();

    if (NumPlayers >= ALobbyGameMode::MaxPlayers)
    {
        StartCountdown(3);
        GetWorld()->GetTimerManager().ClearTimer(CheckTimerHandle);
    }
}


void AGameStartGameState::StartCountdown(int32 InSeconds)
{
    if (!HasAuthority())
        return;

    RemainingTime = InSeconds;
    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this,
        &AGameStartGameState::TickCountdown, 1.0f, true);

    ResetPosition();
}


void AGameStartGameState::ResetPosition()
{
    TArray<AActor*> StartPoints;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), StartPoints);

    for (int32 i = 0; i < PlayerArray.Num(); ++i)
    {
        if (!StartPoints.IsValidIndex(i))
            break;

        APlayerState* PS = PlayerArray[i];
        APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
        if (!PC) continue;

        APawn* Pawn = PC->GetPawn();
        if (!Pawn) continue;

        Pawn->SetActorLocation(StartPoints[i]->GetActorLocation());
        Pawn->SetActorRotation(StartPoints[i]->GetActorRotation());
    }
}


void AGameStartGameState::TickCountdown()
{
    if (--RemainingTime <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
        bGameStarted = true;
        OnRep_GameStarted(); // 明示呼び出し（サーバー側）

        // ゲームのカウントタイマーをセット
        RemainingTime = GameCountMax;
        GetWorld()->GetTimerManager().SetTimer(GameCountTimerHandle, this,
            &AGameStartGameState::TickGameCount, 1.0f, true);
    }

    OnRep_RemainingTime(); // 全クライアントに更新を通知
}


void AGameStartGameState::TickGameCount()
{
    if (--RemainingTime <= 0)
    {
        // ゲーム終了
        GetWorld()->GetTimerManager().ClearTimer(GameCountTimerHandle);
        OnRep_GameEnd(); // 明示呼び出し（サーバー側）
    }

    OnRep_RemainingTime(); // 全クライアントに更新を通知
}

void AGameStartGameState::OnDominate(const FString& zoneName, int teamId)
{
    DominatedTeamMap[zoneName] = teamId;
}

void AGameStartGameState::OnRep_RemainingTime()
{
    auto str = FString::Printf(TEXT("RemainingTime = %d"), RemainingTime);
    UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Orange, 6.f, TEXT("None"));
}


void AGameStartGameState::OnRep_GameStarted()
{
    UKismetSystemLibrary::PrintString(this, TEXT("Game Start!!"), true, true, FColor::Yellow, 6.f, TEXT("None"));
}


void AGameStartGameState::OnRep_GameEnd()
{
    UKismetSystemLibrary::PrintString(this, TEXT("Game End!!"), true, true, FColor::Yellow, 6.f, TEXT("None"));

    for (auto& pair : DominatedTeamMap)
    {
        FString str = FString::Printf(TEXT("Zone:%s dominate - %d"), *pair.first, pair.second);
        UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Green, 6.f, TEXT("None"));
    }
}
