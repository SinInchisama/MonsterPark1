// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayState.h"
#include "Net/UnrealNetwork.h"

APlayState::APlayState() {
    CurrentRound = 0;
    RemainingTime = 0;
    ReplicatedMatchState = EMatchState::Waiting;
}

void APlayState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayState, CurrentRound);
    DOREPLIFETIME(APlayState, RemainingTime);
    DOREPLIFETIME(APlayState, ReplicatedMatchState);
}

void APlayState::OnRep_CurrentRound()
{
    OnRoundChanged.Broadcast(CurrentRound);
}

void APlayState::OnRep_RemainingTime() {
    OnTimerChanged.Broadcast(RemainingTime);
}

void APlayState::OnRep_MatchState() {

}