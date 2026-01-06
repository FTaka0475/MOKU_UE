// Fill out your copyright notice in the Description page of Project Settings.


#include "GP3PlayerState.h"
#include "Net/UnrealNetwork.h"

void AGP3PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGP3PlayerState, TeamId);
}
