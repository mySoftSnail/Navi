// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navi/Enemy/Enemy.h"

AExplosive::AExplosive() 
{
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());

}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();
}

void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* _Actor, AController* Controller)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, GetActorLocation(), FRotator(0.f), true);
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (auto _Actor : OverlappingActors)
	{
		if (Cast<AEnemy>(_Actor))
		{
			UGameplayStatics::ApplyDamage(_Actor, Damage, Controller, _Actor, UDamageType::StaticClass());
		}
	}

	Destroy();
}

void AExplosive::Explode()
{
	BulletHit_Implementation(FHitResult(), this, UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

