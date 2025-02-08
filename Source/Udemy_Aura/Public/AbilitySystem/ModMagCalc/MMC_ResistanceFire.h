// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_ResistanceFire.generated.h"

/**
 * 
 */
UCLASS()
class UDEMY_AURA_API UMMC_ResistanceFire : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_ResistanceFire();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:

	FGameplayEffectAttributeCaptureDefinition ResilienceDef;
	
};
