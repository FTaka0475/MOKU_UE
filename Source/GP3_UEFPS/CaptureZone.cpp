// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureZone.h"
#include "Components/BoxComponent.h"
#include "GP3_UEFPSCharacter.h"
#include "LobbyGameMode.h"
#include "GP3PlayerState.h"
#include "GameStartGameState.h"
#include <algorithm>

// Sets default values
ACaptureZone::ACaptureZone()
{
    bReplicates = true;

    ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
    SetRootComponent(ZoneBox);

    ZoneBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ZoneBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    ZoneBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ZoneBox->SetGenerateOverlapEvents(true);

    ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &ACaptureZone::OnZoneBeginOverlap);
    ZoneBox->OnComponentEndOverlap.AddDynamic(this, &ACaptureZone::OnZoneEndOverlap);
}

void ACaptureZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Pawn なら Character に限定
    AGP3_UEFPSCharacter* Ch = Cast<AGP3_UEFPSCharacter>(OtherActor);
    if (!Ch) return;

    UE_LOG(LogTemp, Warning, TEXT("OnZoneBeginOverlap(%s)"), *Ch->GetName());
    CharactersInZone.Add(Ch);
}

void ACaptureZone::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AGP3_UEFPSCharacter* Ch = Cast<AGP3_UEFPSCharacter>(OtherActor);
    if (!Ch) return;

    UE_LOG(LogTemp, Warning, TEXT("OnZoneEndOverlap(%s)"), *Ch->GetName());
    CharactersInZone.Remove(Ch);
}

// Called when the game starts or when spawned
void ACaptureZone::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(TickHandle, this, &ACaptureZone::ZoneUpdate, 0.2f, true);
    }

}

void ACaptureZone::ZoneUpdate()
{
    std::vector<int> counts; // チームごとのゾーンに入っている人数
    counts.insert(counts.begin(), ALobbyGameMode::MaxPlayers, 0);

    // ゾーンに含まれるプレイヤーを全てループ
    for (auto It = CharactersInZone.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
            continue;
        }

        auto ps = It->Get()->GetPlayerState<AGP3PlayerState>();
        if (ps != nullptr)
        {
            // PlayerStateから取得できるTeamIdに応じたcountsを加算
            counts[ps->TeamId]++;
        }
    }

    // 数えた人数が最大の要素を指すcountsのイテレータを取得
    auto itMax = std::ranges::max_element(counts);

    // 人数が最大のチームの数が単独で存在している場合だけ true
    auto isDominant = std::ranges::count(counts, *itMax) == 1;

    if (isDominant)
    {
        int teamId = itMax - counts.begin();
        if (DominatingTeamId == teamId)
        {
            DominateCount++;
            if (DominateCount == DominateCountMax)
            {
                // 占領が完了した
                AGameStartGameState* GS = GetWorld()->GetGameState<AGameStartGameState>();
                if (!GS) return;

                // 占領したことをゲームステートに通知
                GS->OnDominate(GetName(), DominatingTeamId);
            }
        }
        else
        {
            DominatingTeamId = teamId;
            DominateCount = 0;
        }
        UE_LOG(LogTemp, Log, TEXT("DominatingTeamId =%d DominateCount=%d"), DominatingTeamId, DominateCount);
    }
    else
    {
        if (DominateCount > 0)
        {
            DominateCount--;
            UE_LOG(LogTemp, Log, TEXT("DominatingTeamId =%d DominateCount=%d"), DominatingTeamId, DominateCount);
        }
    }
}


