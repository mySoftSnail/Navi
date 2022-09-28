// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Navi/Interfaces/HitInterface.h"
#include "Explosive.generated.h"

class UParticleSystem;
class USoundCue;
class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class NAVI_API AExplosive : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:
	FORCEINLINE void SetDamage(float InDamage) { Damage = InDamage; }
	
public:	
	AExplosive();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ExplodeParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ExplosiveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage = 1.f;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Actor, AController* Controller) override;

	UFUNCTION(BlueprintCallable)
	void Explode();
};
