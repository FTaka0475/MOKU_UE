// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionTest.h"

#include "SessionSubsystem.h"

// Sets default values
ASessionTest::ASessionTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASessionTest::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* Sub = GetGameInstance()->GetSubsystem<USessionSubsystem>())
	{
		Sub->CreateLanSession(3);
	}
}

// Called every frame
void ASessionTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

