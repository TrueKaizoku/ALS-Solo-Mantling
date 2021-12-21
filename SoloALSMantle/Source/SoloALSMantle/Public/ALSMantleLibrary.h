


#pragma once

#include "ALSMantleData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CoreMinimal.h"
#include "ALSMantleLibrary.generated.h"

class UCapsuleComponent;


UCLASS()
class SOLOALSMANTLE_API UALSMantleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static FTransform MantleComponentLocalToWorld(const FALSComponentAndTransform& CompAndTransform);

	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static FTransform TransfromSub(const FTransform& T1, const FTransform& T2)
	{
		return FTransform(T1.GetRotation().Rotator() - T2.GetRotation().Rotator(),
			T1.GetLocation() - T2.GetLocation(), T1.GetScale3D() - T2.GetScale3D());
	}

	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static FTransform TransfromAdd(const FTransform& T1, const FTransform& T2)
	{
		return FTransform(T1.GetRotation().Rotator() + T2.GetRotation().Rotator(),
			T1.GetLocation() + T2.GetLocation(), T1.GetScale3D() + T2.GetScale3D());
	}

	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static FVector GetCapsuleBaseLocation(float ZOffset, UCapsuleComponent* Capsule);

	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static FVector GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset, UCapsuleComponent* Capsule);

	UFUNCTION(BlueprintCallable, Category = "ALS|Math Utils")
		static bool CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset,
			float RadiusOffset, EDrawDebugTrace::Type DebugType = EDrawDebugTrace::Type::None, bool DrawDebugTrace = false);
};