// Copyright Epic Games, Inc. All Rights Reserved.


#include "GP3_UEFPSWeaponComponent.h"
#include "GP3_UEFPSCharacter.h"
#include "GP3_UEFPSProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GP3_UEFPSPickUpComponent.h"

// Sets default values for this component's properties
UGP3_UEFPSWeaponComponent::UGP3_UEFPSWeaponComponent()
{
	SetIsReplicatedByDefault(true);

	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}


void UGP3_UEFPSWeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	if (!GetOwner()->HasAuthority())
	{
		// クライアント側ならサーバーRPCを呼ぶ
		ServerFire();
//		UKismetSystemLibrary::PrintString(this, TEXT("ClientFire!"), true, true, FColor::Red, 4.f, TEXT("None"));
		return;
	}
	// サーバー側は直接 Spawn
	// （Listen サーバーなら「自分がホストのクライアント」もここを通る）
	ServerFire();
}

bool UGP3_UEFPSWeaponComponent::AttachWeapon(AGP3_UEFPSCharacter* TargetCharacter)
{
	if (Character != nullptr)
	{
		return false;
	}

	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->FindComponentByClass<UGP3_UEFPSWeaponComponent>() != nullptr)
	{
		return false;
	}
	if (Character->GetCurrentWeapon() != nullptr)
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// 現在の武器を伝える
	if (Character->HasAuthority())
	{
		Character->SetCurrentWeapon(this);
	}

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			if (!bInputBound)
			{
				// Fire
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UGP3_UEFPSWeaponComponent::Fire);
				bInputBound = true;
				auto str = FString::Printf(TEXT("BindAction"));
				UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Orange, 4.f, TEXT("None"));
			}
		}
	}

	return true;
}

void UGP3_UEFPSWeaponComponent::DropWeapon()
{
	if (!Character)
	{
		return;
	}

	// キャラの手から外す
	DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// キャラとの関連を切る
	Character = nullptr;

	// ここからは「地面に落ちている武器」としての設定
	if (AActor* OwnerActor = GetOwner()) // これが BP_Pickup_Rifle
	{
		auto str = FString::Printf(TEXT("DropWeapon Owner"));
		UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Blue, 4.f, TEXT("None"));

		OwnerActor->SetOwner(nullptr);

		// 拾えるようにコリジョンONに戻す
		OwnerActor->SetActorEnableCollision(true);

		// 物理で落としたければ
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
		{
			Prim->SetSimulatePhysics(true);
		}

		if (UGP3_UEFPSPickUpComponent* PickupComp =
			OwnerActor->FindComponentByClass<UGP3_UEFPSPickUpComponent>())
		{
			PickupComp->ReactivatePickup();
		}
	}
}

void UGP3_UEFPSWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ensure we have a character owner
	if (Character != nullptr)
	{
		// remove the input mapping context from the Player Controller
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FireMappingContext);
			}
		}
	}

	// maintain the EndPlay call chain
	Super::EndPlay(EndPlayReason);
}

void UGP3_UEFPSWeaponComponent::ServerFire_Implementation()
{
//	UKismetSystemLibrary::PrintString(this, TEXT("ServerFire!"), true, true, FColor::Red, 4.f, TEXT("None"));

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// Spawn the projectile at the muzzle
			World->SpawnActor<AGP3_UEFPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

}
