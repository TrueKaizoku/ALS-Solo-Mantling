// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "AlSMantleData.generated.h"

class UCurveVector;
class UAnimMontage;
class UAnimSequenceBase;
class UCurveFloat;
class UPrimitiveComponent;





USTRUCT(BlueprintType)
struct FALSComponentAndTransform
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "Character Struct Library")
		FTransform Transform;

	UPROPERTY(EditAnywhere, Category = "Character Struct Library")
		UPrimitiveComponent* Component = nullptr;
};

UENUM(BlueprintType)
enum class EALSMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

UENUM(BlueprintType)
enum class EALSMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

USTRUCT(BlueprintType)
struct FALSMantleAsset
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "Mantle System")
		UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		UCurveVector* PositionCorrectionCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		FVector StartingOffset;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float LowHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float LowPlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float LowStartPosition = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float HighHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float HighPlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float HighStartPosition = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSMantleParams
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "Mantle System")
		UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		UCurveVector* PositionCorrectionCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float StartingPosition = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float PlayRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		FVector StartingOffset;
};

USTRUCT(BlueprintType)
struct FALSMantleTraceSettings
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "Mantle System")
		float MaxLedgeHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float MinLedgeHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float ReachDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float ForwardTraceRadius = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Mantle System")
		float DownwardTraceRadius = 0.0f;
};


USTRUCT(BlueprintType)
struct FALSMovementAction
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		EALSMovementAction Action = EALSMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		bool LowMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		bool HighMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		bool Rolling_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
		bool GettingUp_ = false;

public:
	FALSMovementAction()
	{
	}

	FALSMovementAction(const EALSMovementAction InitialAction) { *this = InitialAction; }

	const bool& None() const { return None_; }
	const bool& LowMantle() const { return LowMantle_; }
	const bool& HighMantle() const { return HighMantle_; }
	const bool& Rolling() const { return Rolling_; }
	const bool& GettingUp() const { return GettingUp_; }

	operator EALSMovementAction() const { return Action; }

	void operator=(const EALSMovementAction NewAction)
	{
		Action = NewAction;
		None_ = Action == EALSMovementAction::None;
		LowMantle_ = Action == EALSMovementAction::LowMantle;
		HighMantle_ = Action == EALSMovementAction::HighMantle;
		Rolling_ = Action == EALSMovementAction::Rolling;
		GettingUp_ = Action == EALSMovementAction::GettingUp;
	}
};



