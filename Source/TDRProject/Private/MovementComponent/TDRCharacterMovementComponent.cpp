// Fill out your copyright notice in the Description page of Project Settings.

#include "..\..\Public\MovementComponent\TDRCharacterMovementComponent.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"
#include "TimerManager.h"

UTDRCharacterMovementComponent::UTDRCharacterMovementComponent(const class FObjectInitializer& ObjectInitialiazer) : Super(ObjectInitialiazer)
{
	DodgeStrength = 10000.f;
	WallWalkingStrength = 1000.f;
}

void UTDRCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bWalkingSideWays)
	{
		Server_SetLocation();
	}
}

bool UTDRCharacterMovementComponent::Server_SetLocation_Validate()
{
	return true;
}

void UTDRCharacterMovementComponent::Server_SetLocation_Implementation()
{
	/*if (PawnOwner->Role == ROLE_Authority)
	{
		FVector a = PawnOwner->GetActorLocation();
		a.Z = ZLocation;
		PawnOwner->SetActorLocation(a);
	}*/
	
}

void UTDRCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);
	if (!CharacterOwner)
	{
		return;
	}

	if (bWalkingUp && PawnOwner->GetVelocity().Z < 0)
	{
		FQuat QuatRotation = FQuat(FRotator(-85, 0, 0));
		PawnOwner->AddActorLocalRotation(QuatRotation, true, 0, ETeleportType::None);
		bWalkingUp = false;
	}

	if (bWalkingSideWays && bEndSideWalk)
	{
		/*if (PawnOwner->Role == ROLE_Authority)
		{
			FVector DodgeVelocity = (MoveDirection * 1000);
			Launch(DodgeVelocity);
		}*/

		PawnOwner->AddActorLocalRotation(PosToReturn, true, 0, ETeleportType::None);
		bEndSideWalk = false;
		bWalkingSideWays = false;
	}

	if (bWalkup)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			Server_MoveDirection(FVector(0, 0, PawnOwner->GetActorUpVector().Z));
		}
		FQuat QuatRotation = FQuat(FRotator(85, 0, 0));
		PawnOwner->AddActorLocalRotation(QuatRotation, true, 0, ETeleportType::None);

		if (PawnOwner->Role == ROLE_Authority)
		{
			FVector DodgeVelocity = (MoveDirection * WallWalkingStrength);
			Launch(DodgeVelocity);
		}
		bWalkup = false;
		bWalkingUp = true;
	}

	if (bWalkSideWays)
	{
		//ZLocation = PawnOwner->GetActorLocation().Z + 200;
		ATDRCharacterBase* CharacterMovement = Cast<ATDRCharacterBase>(PawnOwner);

		if (PawnOwner->IsLocallyControlled())
		{
			Server_MoveDirection(FVector(PawnOwner->GetActorForwardVector().X, PawnOwner->GetActorForwardVector().Y, 0).GetSafeNormal());
		}
		
		if (CharacterMovement->bLeftArmTouchingWall)
		{
			FQuat QuatRotation = FQuat(FRotator(0, 0, -85));
			PosToReturn = FQuat(FRotator(0, 0, 85));
			PawnOwner->AddActorLocalRotation(QuatRotation, true, 0, ETeleportType::None);			
		}
		else
		{
			FQuat QuatRotation = FQuat(FRotator(0, 0, 85));
			PosToReturn = FQuat(FRotator(0, 0, -85));
			PawnOwner->AddActorLocalRotation(QuatRotation, true, 0, ETeleportType::None);
		}

		if (PawnOwner->Role == ROLE_Authority)
		{
			FVector DodgeVelocity = (MoveDirection * WallWalkingStrength);
			StopActiveMovement();
			StopMovementImmediately();
			Launch(DodgeVelocity);
		}
		PawnOwner->GetActorLocation();
		bWalkSideWays = false;
		bWalkingSideWays = true;
		FTimerHandle TimerHandle;
		PawnOwner->GetWorldTimerManager().SetTimer(TimerHandle, this, &UTDRCharacterMovementComponent::EndSideWalk, 2.f, false);
	}

	if (bWantsToDodge)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			Server_MoveDirection(PawnOwner->GetLastMovementInputVector());
		}
		if (PawnOwner->Role == ROLE_Authority)
		{
			FVector DodgeVelocity = (MoveDirection * DodgeStrength);
			Launch(DodgeVelocity);
		}
		bWantsToDodge = false;
	}
	/*if (PawnOwner->Role == ROLE_Authority)
	{
		if (bWantsToDodge)
		{
			if (PawnOwner->IsLocallyControlled())
			{
				Server_MoveDirection(PawnOwner->GetLastMovementInputVector());
			}
			MoveDirection.Normalize();
			FVector DodgeVelocity;
			DodgeVelocity = MoveDirection * DodgeStrength;
			DodgeVelocity.Z = 0.0f;
			Launch(DodgeVelocity);
			bWantsToDodge = false;

		}

		if (bReturnToNormal)
		{
			FQuat QuatRotation = FQuat(FRotator(-85, 0, 0));
			PawnOwner->AddActorLocalRotation(QuatRotation);
			bReturnToNormal = false;
		}
	}*/

}

void UTDRCharacterMovementComponent::EndSideWalk()
{
	bEndSideWalk = true;
}

void UTDRCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client side
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToDodge = (Flags&FSavedMove_Character::FLAG_Custom_1) != 0;
	bWalkup = (Flags&FSavedMove_Character::FLAG_Custom_2) != 0;
	bWalkSideWays = (Flags&FSavedMove_Character::FLAG_Custom_3) != 0;
}

class FNetworkPredictionData_Client* UTDRCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);
	check(PawnOwner->Role < ROLE_Authority);

	if (!ClientPredictionData)
	{
		UTDRCharacterMovementComponent* MutableThis = const_cast<UTDRCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UTDRCharacterMovementComponent::FSavedMove_My::Clear()
{
	Super::Clear();
	bSavedWAntsToDodge = false;
	SavedMoveDirection = FVector::ZeroVector;
	bSavedWalkUp = false;
	bSavedWalkSideWays = false;
}

uint8 UTDRCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWAntsToDodge)
	{
		Result |= FLAG_Custom_1;
	}
	if (bSavedWalkUp)
	{
		Result |= FLAG_Custom_2;
	}

	if (bSavedWalkSideWays)
	{
		Result |= FLAG_Custom_3;
	}

	return Result;
}

bool UTDRCharacterMovementComponent::FSavedMove_My::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{

	if (bSavedWAntsToDodge != ((FSavedMove_My*)&NewMove)->bSavedWAntsToDodge)
	{
		return false;
	}

	if (SavedMoveDirection != ((FSavedMove_My*)&NewMove)->SavedMoveDirection)
	{
		return false;
	}

	if (bSavedWalkUp != ((FSavedMove_My*)&NewMove)->bSavedWalkUp)
	{
		return false;
	}

	if (bSavedWalkSideWays != ((FSavedMove_My*)&NewMove)->bSavedWalkSideWays)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UTDRCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	UTDRCharacterMovementComponent* CharacterMovement = Cast<UTDRCharacterMovementComponent>(Character->GetCharacterMovement());

	if (CharacterMovement)
	{
		bSavedWAntsToDodge = CharacterMovement->bWantsToDodge;
		SavedMoveDirection = CharacterMovement->MoveDirection;
		bSavedWalkUp = CharacterMovement->bWalkup;
		bSavedWalkSideWays = CharacterMovement->bWalkSideWays;
	}
}

void UTDRCharacterMovementComponent::FSavedMove_My::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UTDRCharacterMovementComponent* CharacterMovement = Cast<UTDRCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->MoveDirection = SavedMoveDirection;
	}
}

UTDRCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr UTDRCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_My());
}

bool UTDRCharacterMovementComponent::Server_MoveDirection_Validate(const FVector& MoveDir)
{
	return true;
}

void UTDRCharacterMovementComponent::Server_MoveDirection_Implementation(const FVector& MoveDir)
{
	MoveDirection = MoveDir;
}

//Trigger dodge
void UTDRCharacterMovementComponent::Dodge()
{
	bWantsToDodge = true;
}

void UTDRCharacterMovementComponent::Walkup()
{
	bWalkup = true;
}

void UTDRCharacterMovementComponent::SideWalk()
{
	bWalkSideWays = true;
}

