// Copyright Epic Games, Inc. All Rights Reserved.

#include "GP3_UEFPSCharacter.h"
#include "GP3_UEFPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GP3_UEFPSGameMode.h"
#include "GP3_UEFPSWeaponComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AGP3_UEFPSCharacter

AGP3_UEFPSCharacter::AGP3_UEFPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

//////////////////////////////////////////////////////////////////////////// Input

void AGP3_UEFPSCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AGP3_UEFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGP3_UEFPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGP3_UEFPSCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AGP3_UEFPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AGP3_UEFPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGP3_UEFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGP3_UEFPSCharacter, Health);
}

float AGP3_UEFPSCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// ここはサーバーでだけ呼ばれる想定（ApplyDamage したのがサーバーなので）
	const float OldHealth = Health;
	Health = FMath::Max(0.0f, Health - DamageAmount);

	auto str = FString::Printf(TEXT("%s took %f damage -> %f"), *GetName(), DamageAmount, Health);
	UKismetSystemLibrary::PrintString(this, str, true, true, FColor::Red, 4.f, TEXT("None"));

	if (Health <= 0.0f && OldHealth > 0.0f)
	{
		UKismetSystemLibrary::PrintString(this, TEXT("Die!"), true, true, FColor::Red, 4.f, TEXT("None"));
		Die();
	}

	return DamageAmount;
}


void AGP3_UEFPSCharacter::SetCurrentWeapon(UGP3_UEFPSWeaponComponent* w)
{
	if (!HasAuthority())
	{
		return;
	}
	CurrentWeapon = w;
}


void AGP3_UEFPSCharacter::HandleRespawn(AController* DeadController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (DeadController == nullptr)
	{
		Destroy();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	// GameModeを取得
	AGP3_UEFPSGameMode* GM = World->GetAuthGameMode<AGP3_UEFPSGameMode>();
	if (GM)
	{
		// PlayerStart から新しいキャラを spawn + Possess してくれる
		GM->RestartPlayer(DeadController);
	}

	// 古い死体を削除
	Destroy();
}


void AGP3_UEFPSCharacter::OnRep_Health()
{
	// クライアント側：UI更新やヒット演出など
	UKismetSystemLibrary::PrintString(
		this,
		FString::Printf(TEXT("Health = %.1f"), Health),
		true, false, FColor::Red, 1.0f);
}

void AGP3_UEFPSCharacter::Die()
{
	// 死亡処理（ラグドール、Respawn など）
	// 念のためサーバーだけ
	if (!HasAuthority())
	{
		return;
	}

	if (Health > 0.0f)
	{
		return;
	}

	// まず武器を落とす／リセット
	if (CurrentWeapon)
	{
		CurrentWeapon->DropWeapon();
		CurrentWeapon = nullptr;
	}

	// 移動停止
	if (auto* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	// 当たり判定オフ（撃たれたり当たったりしないように）
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 武器発射などの入力を止めたい場合は Controller のInputを無効にしたりも可
	if (AController* PC = Controller)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	// Controllerを後で RestartPlayer に渡したいので退避
	AController* DeadController = Controller;

	// コントローラとこのPawnの関連を切る（RestartPlayerが新しいPawnを持てるように）
	DetachFromControllerPendingDestroy();

	// タイマーセット
	if (UWorld* World = GetWorld())
	{
		FTimerDelegate RespawnDelegate;
		RespawnDelegate.BindUObject(this, &AGP3_UEFPSCharacter::HandleRespawn, DeadController);

		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			RespawnDelegate,
			RespawnDelay,
			false  // ループしない
		);
	}

	// ここで死体アニメやラグドールにしたければこのタイミングで
	// Mesh1P->SetSimulatePhysics(true); など
}
