// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureZone.h"
#include "Components/BoxComponent.h"
#include "GP3_UEFPSCharacter.h"

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
    // Pawn ‚È‚ç Character ‚ÉŒÀ’è
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
    int32 Count = 0;
    for (auto It = CharactersInZone.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
            continue;
        }
        Count++;
    }
        
    if (Count > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Zone %s : players=%d"), *GetName(), Count);
    }
}


