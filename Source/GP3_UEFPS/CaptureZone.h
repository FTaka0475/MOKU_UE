// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CaptureZone.generated.h"

class UBoxComponent;
class AGP3_UEFPSCharacter;

UCLASS()
class GP3_UEFPS_API ACaptureZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACaptureZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* ZoneBox;

    // 今このゾーン内にいるプレイヤー（Character）を保持
    UPROPERTY()
    TSet<TWeakObjectPtr<AGP3_UEFPSCharacter>> CharactersInZone;

    UFUNCTION()
    void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ZoneUpdate();

    FTimerHandle TickHandle;

    int DominatingTeamId = -1;
    int DominateCount = 0;

    static constexpr int DominateCountMax = 15;
};
