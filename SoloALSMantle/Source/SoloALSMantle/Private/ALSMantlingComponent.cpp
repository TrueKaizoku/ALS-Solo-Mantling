// Fill out your copyright notice in the Description page of Project Settings.


#include "ALSMantlingComponent.h"
#include "GameFramework/Character.h"
#include "Curves/CurveVector.h"
#include "ALSMantleLibrary.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


const FName NAME_MantleEnd(TEXT("MantleEnd"));
const FName NAME_MantleUpdate(TEXT("MantleUpdate"));
const FName NAME_MantleTimeline(TEXT("MantleTimeline"));

FName UALSMantlingComponent::NAME_IgnoreOnlyPawn(TEXT("IgnoreOnlyPawn"));


UALSMantlingComponent::UALSMantlingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MantleTimeline = CreateDefaultSubobject<UTimelineComponent>(NAME_MantleTimeline);
}

void UALSMantlingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter)
		{

			AddTickPrerequisiteActor(OwnerCharacter); // Always tick after owner, so we'll use updated values

			// Bindings
			FOnTimelineFloat TimelineUpdated;
			FOnTimelineEvent TimelineFinished;
			TimelineUpdated.BindUFunction(this, NAME_MantleUpdate);
			TimelineFinished.BindUFunction(this, NAME_MantleEnd);
			MantleTimeline->SetTimelineFinishedFunc(TimelineFinished);
			MantleTimeline->SetLooping(false);
			MantleTimeline->SetTimelineLengthMode(TL_TimelineLength);
			MantleTimeline->AddInterpFloat(MantleTimelineCurve, TimelineUpdated);

		}
	}
}


void UALSMantlingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (OwnerCharacter)
	{

		HasMovementInput = (OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() > 0.0f);
		if (OwnerCharacter->GetCharacterMovement()->MovementMode == MOVE_Falling)
		{
			// Perform a mantle check if falling while movement input is pressed.
			if (HasMovementInput)
			{
				MantleCheck(FallingTraceSettings);
			}
		}
	}
}

void UALSMantlingComponent::MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
	EALSMantleType MantleType)
{
	if (OwnerCharacter == nullptr || !IsValid(MantleLedgeWS.Component) || !IsValid(MantleTimeline))
	{
		return;
	}

	// Disable ticking during mantle
	SetComponentTickEnabledAsync(false);

	// Step 1: Get the Mantle Asset and use it to set the new Mantle Params.
	const FALSMantleAsset MantleAsset = GetMantleAsset(MantleType);
	check(MantleAsset.PositionCorrectionCurve)

		MantleParams.AnimMontage = MantleAsset.AnimMontage;
	MantleParams.PositionCorrectionCurve = MantleAsset.PositionCorrectionCurve;
	MantleParams.StartingOffset = MantleAsset.StartingOffset;
	MantleParams.StartingPosition = FMath::GetMappedRangeValueClamped({ MantleAsset.LowHeight, MantleAsset.HighHeight },
																	  {
																		  MantleAsset.LowStartPosition,
																		  MantleAsset.HighStartPosition
																	  },
		MantleHeight);
	MantleParams.PlayRate = FMath::GetMappedRangeValueClamped({ MantleAsset.LowHeight, MantleAsset.HighHeight },
		{ MantleAsset.LowPlayRate, MantleAsset.HighPlayRate },
		MantleHeight);

	// Step 2: Convert the world space target to the mantle component's local space for use in moving objects.
	MantleLedgeLS.Component = MantleLedgeWS.Component;
	MantleLedgeLS.Transform = MantleLedgeWS.Transform * MantleLedgeWS.Component->GetComponentToWorld().Inverse();

	// Step 3: Set the Mantle Target and calculate the Starting Offset
	// (offset amount between the actor and target transform).
	MantleTarget = MantleLedgeWS.Transform;
	MantleActualStartOffset = UALSMantleLibrary::TransfromSub(OwnerCharacter->GetActorTransform(), MantleTarget);

	// Step 4: Calculate the Animated Start Offset from the Target Location.
	// This would be the location the actual animation starts at relative to the Target Transform.
	FVector RotatedVector = MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y;
	RotatedVector.Z = MantleParams.StartingOffset.Z;
	const FTransform StartOffset(MantleTarget.Rotator(), MantleTarget.GetLocation() - RotatedVector,
		FVector::OneVector);
	MantleAnimatedStartOffset = UALSMantleLibrary::TransfromSub(StartOffset, MantleTarget);

	// Step 5: Clear the Character Movement Mode and set the Movement State to Mantling
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

	// Step 6: Configure the Mantle Timeline so that it is the same length as the
	// Lerp/Correction curve minus the starting position, and plays at the same speed as the animation.
	// Then start the timeline.
	float MinTime = 0.0f;
	float MaxTime = 0.0f;
	MantleParams.PositionCorrectionCurve->GetTimeRange(MinTime, MaxTime);
	MantleTimeline->SetTimelineLength(MaxTime - MantleParams.StartingPosition);
	MantleTimeline->SetPlayRate(MantleParams.PlayRate);
	MantleTimeline->PlayFromStart();

	// Step 7: Play the Anim Montaget if valid.
	if (IsValid(MantleParams.AnimMontage))
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(MantleParams.AnimMontage, MantleParams.PlayRate,
			EMontagePlayReturnType::MontageLength,
			MantleParams.StartingPosition, false);
	}
}

bool UALSMantlingComponent::MantleCheck(const FALSMantleTraceSettings& TraceSettings)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	// Step 1: Trace forward to find a wall / object the character cannot walk on.
	const FVector& TraceDirection = HasMovementInput
		? OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal()
		: OwnerCharacter->GetActorForwardVector();
	const FVector& CapsuleBaseLocation = UALSMantleLibrary::GetCapsuleBaseLocation(
		2.0f, OwnerCharacter->GetCapsuleComponent());
	FVector TraceStart = CapsuleBaseLocation + TraceDirection * -30.0f;
	TraceStart.Z += (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2.0f;
	const FVector TraceEnd = TraceStart + TraceDirection * TraceSettings.ReachDistance;
	const float HalfHeight = 1.0f + (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2.0f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(TraceSettings.ForwardTraceRadius, HalfHeight);
		const bool bHit = World->SweepSingleByProfile(HitResult, TraceStart, TraceEnd, FQuat::Identity, MantleObjectDetectionProfile,
			CapsuleCollisionShape, Params);

	}

	if (!HitResult.IsValidBlockingHit() || OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// Not a valid surface to mantle
		return false;
	}

	if (HitResult.GetComponent() != nullptr)
	{
		UPrimitiveComponent* PrimitiveComponent = HitResult.GetComponent();
		if (PrimitiveComponent && PrimitiveComponent->GetComponentVelocity().Size() > AcceptableVelocityWhileMantling)
		{
			// The surface to mantle moves too fast
			return false;
		}
	}

	const FVector InitialTraceImpactPoint = HitResult.ImpactPoint;
	const FVector InitialTraceNormal = HitResult.ImpactNormal;

	// Step 2: Trace downward from the first trace's Impact Point and determine if the hit location is walkable.
	FVector DownwardTraceEnd = InitialTraceImpactPoint;
	DownwardTraceEnd.Z = CapsuleBaseLocation.Z;
	DownwardTraceEnd += InitialTraceNormal * -15.0f;
	FVector DownwardTraceStart = DownwardTraceEnd;
	DownwardTraceStart.Z += TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.0f;

	{
		const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceSettings.DownwardTraceRadius);
		const bool bHit = World->SweepSingleByChannel(HitResult, DownwardTraceStart, DownwardTraceEnd, FQuat::Identity,
			WalkableSurfaceDetectionChannel, SphereCollisionShape,
			Params);

	}


	if (!OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// Not a valid surface to mantle
		return false;
	}

	const FVector DownTraceLocation(HitResult.Location.X, HitResult.Location.Y, HitResult.ImpactPoint.Z);
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	// Step 3: Check if the capsule has room to stand at the downward trace's location.
	// If so, set that location as the Target Transform and calculate the mantle height.
	const FVector& CapsuleLocationFBase = UALSMantleLibrary::GetCapsuleLocationFromBase(
		DownTraceLocation, 2.0f, OwnerCharacter->GetCapsuleComponent());
	const bool bCapsuleHasRoom = UALSMantleLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
		CapsuleLocationFBase, 0.0f,
		0.0f, EDrawDebugTrace::Type::None, bShowTraces);

	if (!bCapsuleHasRoom)
	{
		// Capsule doesn't have enough room to mantle
		return false;
	}

	const FTransform TargetTransform(
		(InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator(),
		CapsuleLocationFBase,
		FVector::OneVector);

	const float MantleHeight = (CapsuleLocationFBase - OwnerCharacter->GetActorLocation()).Z;

	// Step 4: Determine the Mantle Type by checking the movement mode and Mantle Height.
	EALSMantleType MantleType;
	if (OwnerCharacter->GetCharacterMovement()->MovementMode == (MOVE_Falling))
	{
		MantleType = EALSMantleType::FallingCatch;
	}
	else
	{
		MantleType = MantleHeight > 125.0f ? EALSMantleType::HighMantle : EALSMantleType::LowMantle;
	}

	// Step 5: If everything checks out, start the Mantle
	FALSComponentAndTransform MantleWS;
	MantleWS.Component = HitComponent;
	MantleWS.Transform = TargetTransform;
	MantleStart(MantleHeight, MantleWS, MantleType);
	Server_MantleStart(MantleHeight, MantleWS, MantleType);

	return true;
}

void UALSMantlingComponent::Server_MantleStart_Implementation(float MantleHeight,
	const FALSComponentAndTransform& MantleLedgeWS,
	EALSMantleType MantleType)
{
	Multicast_MantleStart(MantleHeight, MantleLedgeWS, MantleType);
}

void UALSMantlingComponent::Multicast_MantleStart_Implementation(float MantleHeight,
	const FALSComponentAndTransform& MantleLedgeWS,
	EALSMantleType MantleType)
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		MantleStart(MantleHeight, MantleLedgeWS, MantleType);
	}
}

// This function is called by "MantleTimeline" using BindUFunction in the AALSBaseCharacter::BeginPlay during the default settings initalization.
void UALSMantlingComponent::MantleUpdate(float BlendIn)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Step 1: Continually update the mantle target from the stored local transform to follow along with moving objects
	MantleTarget = UALSMantleLibrary::MantleComponentLocalToWorld(MantleLedgeLS);

	// Step 2: Update the Position and Correction Alphas using the Position/Correction curve set for each Mantle.
	const FVector CurveVec = MantleParams.PositionCorrectionCurve
		->GetVectorValue(
			MantleParams.StartingPosition + MantleTimeline->GetPlaybackPosition());
	const float PositionAlpha = CurveVec.X;
	const float XYCorrectionAlpha = CurveVec.Y;
	const float ZCorrectionAlpha = CurveVec.Z;

	// Step 3: Lerp multiple transforms together for independent control over the horizontal
	// and vertical blend to the animated start position, as well as the target position.

	// Blend into the animated horizontal and rotation offset using the Y value of the Position/Correction Curve.
	const FTransform TargetHzTransform(MantleAnimatedStartOffset.GetRotation(),
		{
			MantleAnimatedStartOffset.GetLocation().X,
			MantleAnimatedStartOffset.GetLocation().Y,
			MantleActualStartOffset.GetLocation().Z
		},
		FVector::OneVector);
	const FTransform& HzLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetHzTransform, XYCorrectionAlpha);

	// Blend into the animated vertical offset using the Z value of the Position/Correction Curve.
	const FTransform TargetVtTransform(MantleActualStartOffset.GetRotation(),
		{
			MantleActualStartOffset.GetLocation().X,
			MantleActualStartOffset.GetLocation().Y,
			MantleAnimatedStartOffset.GetLocation().Z
		},
		FVector::OneVector);
	const FTransform& VtLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetVtTransform, ZCorrectionAlpha);

	const FTransform ResultTransform(HzLerpResult.GetRotation(),
		{
			HzLerpResult.GetLocation().X, HzLerpResult.GetLocation().Y,
			VtLerpResult.GetLocation().Z
		},
		FVector::OneVector);

	// Blend from the currently blending transforms into the final mantle target using the X
	// value of the Position/Correction Curve.
	const FTransform& ResultLerp = UKismetMathLibrary::TLerp(
		UALSMantleLibrary::TransfromAdd(MantleTarget, ResultTransform), MantleTarget,
		PositionAlpha);

	// Initial Blend In (controlled in the timeline curve) to allow the actor to blend into the Position/Correction
	// curve at the midoint. This prevents pops when mantling an object lower than the animated mantle.
	const FTransform& LerpedTarget =
		UKismetMathLibrary::TLerp(UALSMantleLibrary::TransfromAdd(MantleTarget, MantleActualStartOffset), ResultLerp,
			BlendIn);

	// Step 4: Set the actors location and rotation to the Lerped Target.
	OwnerCharacter->SetActorLocationAndRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation().Rotator());
}

void UALSMantlingComponent::MantleEnd()
{
	// Set the Character Movement Mode to Walking
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	// Enable ticking back after mantle ends
	SetComponentTickEnabledAsync(true);
}

void UALSMantlingComponent::OnOwnerJumpInput()
{
	// Check if character is able to do one of the special mantling

	if (OwnerCharacter)
	{
		if (OwnerCharacter->GetCharacterMovement()->MovementMode == (MOVE_Walking) || OwnerCharacter->GetCharacterMovement()->MovementMode == (MOVE_NavWalking))
		{
			if (HasMovementInput)
			{
				MantleCheck(GroundedTraceSettings);
			}
		}
		else if (OwnerCharacter->GetCharacterMovement()->MovementMode == MOVE_Falling)
		{
			MantleCheck(FallingTraceSettings);
		}
	}
}

void UALSMantlingComponent::OnOwnerRagdollStateChanged(bool bRagdollState)
{
	// If owner is going into ragdoll state, stop mantling immediately
	if (bRagdollState)
	{
		MantleTimeline->Stop();
	}
}
