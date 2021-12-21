// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALSMantleData.h"
#include "GameplayTagContainer.h"
#include "Curves/CurveVector.h"
#include "ALSMantlingComponent.generated.h"

class UTimelineComponent;

UCLASS(Blueprintable, BlueprintType)
class SOLOALSMANTLE_API UALSMantlingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UALSMantlingComponent();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		bool MantleCheck(const FALSMantleTraceSettings& TraceSettings);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		void MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
			EALSMantleType MantleType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		void MantleUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		void MantleEnd();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		void OnOwnerJumpInput();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
		void OnOwnerRagdollStateChanged(bool bRagdollState);

	/** Implement on BP to get correct mantle parameter set according to character state */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Mantle System")
		FALSMantleAsset GetMantleAsset(EALSMantleType MantleType);

	UPROPERTY(BlueprintReadWrite, Category = "ALS|Mantle System")
		bool  bShowTraces = false;

	UPROPERTY(BlueprintReadWrite, Category = "ALS|Mantle System")
		bool HasMovementInput = false;


protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the game starts
	virtual void BeginPlay() override;

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "ALS|Mantle System")
		void Server_MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
			EALSMantleType MantleType);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "ALS|Mantle System")
		void Multicast_MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
			EALSMantleType MantleType);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Mantle System")
		UTimelineComponent* MantleTimeline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		FALSMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		FALSMantleTraceSettings AutomaticTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		FALSMantleTraceSettings FallingTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		UCurveFloat* MantleTimelineCurve;

	static FName NAME_IgnoreOnlyPawn;
	/** Profile to use to detect objects we allow mantling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		FName MantleObjectDetectionProfile = NAME_IgnoreOnlyPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		TEnumAsByte<ECollisionChannel> WalkableSurfaceDetectionChannel = ECC_Visibility;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		FALSMantleParams MantleParams;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		FALSComponentAndTransform MantleLedgeLS;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		FTransform MantleTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		FTransform MantleActualStartOffset = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		FTransform MantleAnimatedStartOffset = FTransform::Identity;

	/** If a dynamic object has a velocity bigger than this value, do not start mantle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
		float AcceptableVelocityWhileMantling = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
		ACharacter* OwnerCharacter;

};
