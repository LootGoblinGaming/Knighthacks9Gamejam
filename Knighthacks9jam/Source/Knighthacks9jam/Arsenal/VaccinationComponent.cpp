#include "VaccinationComponent.h"
#include "GameFramework/Character.h"
#include "MovementModifierComponent.h"
#include "AIController.h"

UVaccinationComponent::UVaccinationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UVaccinationComponent::ApplyDose(AActor* VaccineInstigator)
{
	if (bVaccinated)
	{
		return false;
	}

	DosesReceived++;
	OnDoseReceived.Broadcast(VaccineInstigator, DosesReceived, DosesRequired);

	if (DosesReceived < DosesRequired)
	{
		return false;
	}
	bVaccinated = true;

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UMovementModifierComponent* Modifier = UMovementModifierComponent::FindOrAdd(Character))
		{
			Modifier->LockSpeed(CuredWalkSpeed);
		}

		// cancel the current chase; the speed lock above keeps any later MoveTo from going anywhere
		if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
		{
			AIController->StopMovement();
		}
	}

	OnVaccinated.Broadcast(VaccineInstigator);
	return true;
}
