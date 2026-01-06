// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GP3PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GP3_UEFPS_API AGP3PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 TeamId = 0; // 0 or 1

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
