// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Udemy_Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			SocketTag);
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		
		
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Projectile);
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(Projectile);
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);
		
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

		for (TTuple<FGameplayTag, FScalableFloat> Pair : DamageTypes)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}

		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}

FString UAuraProjectileSpell::GetDescription(int32 Level)
{
	const float Damage = DamageTypes[FAuraGameplayTags::Get().Damage_Fire].GetValueAtLevel(Level);
	FString FormattedDamage = FString::Printf(TEXT("%.1f"), Damage);
	if (Level == 1)
	{
		return FString::Printf(TEXT("<Title>FIRE BOLT</>\n\n<Default>Launches a bolt of fire, exploding on impact and dealing: </><Damage>%s</><Default> fire damage with a chance to burn</>\n\n<Small>Level: </><Level>%i</>"),
			*FormattedDamage, Level);
	}
	else
	{
		return FString::Printf(TEXT("<Title>FIRE BOLT </>\n \n<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%s</><Default> fire damage with a chance to burn</>\n \n<Small>Level: </><Level>%i</>"),
			FMath::Min(Level, NumProjectiles), *FormattedDamage, Level);	
	}
}

FString UAuraProjectileSpell::GetNextLevelDescription(int32 Level)
{
	const float Damage = DamageTypes[FAuraGameplayTags::Get().Damage_Fire].GetValueAtLevel(Level);
	FString FormattedDamage = FString::Printf(TEXT("%.1f"), Damage);
		
	return FString::Printf(TEXT("<Title>NEXT LEVEL</>\n \n<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%s</><Default> fire damage with a chance to burn</>\n \n<Small>Level: </><Level>%i</>"),
				FMath::Min(Level, NumProjectiles), *FormattedDamage, Level);	
	
}
