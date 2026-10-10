#include "VaccineDartProjectile.h"
#include "VaccineTarget.h"
#include "VaccinationComponent.h"
#include "ArsenalEffects.h"
#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"

AVaccineDartProjectile::AVaccineDartProjectile()
{
	// quiet - this is a precision tool, not something meant to draw the horde
	NoiseLoudness = 1.0f;
	NoiseRange = 500.0f;
	NoiseTag = FName("VaccineDart");

	bRicochetOnImpact = true;
}

void AVaccineDartProjectile::OnProjectileImpact(const FHitResult& Hit)
{
	Super::OnProjectileImpact(Hit);

	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return;
	}

	if (UVaccinationComponent* Vaccination = HitActor->FindComponentByClass<UVaccinationComponent>())
	{
		const bool bAlreadyCured = Vaccination->IsVaccinated();
		if (Vaccination->ApplyDose(GetInstigator()))
		{
			PlayCureFeedback(HitActor);
		}
		else if (!bAlreadyCured)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DoseSound, Hit.ImpactPoint);
			ArsenalEffects::SpawnEffectAtLocation(this, DoseEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}
	}

	if (HitActor->Implements<UVaccineTarget>())
	{
		IVaccineTarget::Execute_ApplyVaccine(HitActor, GetInstigator());
	}
}

void AVaccineDartProjectile::PlayCureFeedback(AActor* CuredActor)
{
	UGameplayStatics::PlaySoundAtLocation(this, CureSound, CuredActor->GetActorLocation());
	ArsenalEffects::SpawnEffectAttached(CureEffect, CuredActor->GetRootComponent());

	if (CuredOverlayMaterial)
	{
		TArray<UMeshComponent*> Meshes;
		CuredActor->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			Mesh->SetOverlayMaterial(CuredOverlayMaterial);
		}
	}
}
