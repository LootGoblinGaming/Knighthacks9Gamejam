#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChecklistWidget.generated.h"

class UTexture2D;
class UImage;
class UCanvasPanel;

/** One line on the checklist paper: which player bool marks it collected, and where its strike goes */
USTRUCT(BlueprintType)
struct FChecklistLine
{
	GENERATED_BODY()

	/** Name of the bool variable on the player Blueprint that is true once this item is collected (e.g. HasWater) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist")
	FName CollectedFlag;

	/** Strike start/end and vertical centre, as fractions (0-1) of the paper image's width/height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist", meta = (ClampMin = 0, ClampMax = 1))
	float Left = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist", meta = (ClampMin = 0, ClampMax = 1))
	float Right = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist", meta = (ClampMin = 0, ClampMax = 1))
	float CenterY = 0.5f;
};

/**
 * Hand-drawn survival checklist HUD. Shows the paper texture in a screen corner and draws a strike through
 * each line as the player collects that item (read from the player Blueprint's Has* bools, so the pickups
 * need no changes). Defaults match the art in /Game/UI/T_Checklist and /Game/UI/T_ChecklistStrike.
 */
UCLASS()
class KNIGHTHACKS9JAM_API UChecklistWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChecklistWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist")
	TObjectPtr<UTexture2D> PaperTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist")
	TObjectPtr<UTexture2D> StrikeTexture;

	/** On-screen width of the paper in pixels; height follows the texture's proportions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist", meta = (ClampMin = 50))
	float PaperWidth = 230.0f;

	/** Distance from the top-left corner of the screen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist")
	FVector2D ScreenOffset = FVector2D(24.0f, 24.0f);

	/** Strike thickness as a fraction of the paper height */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist", meta = (ClampMin = 0.005, ClampMax = 0.2))
	float StrikeHeight = 0.05f;

	/** Seconds for a strike to be drawn across its line */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist", meta = (ClampMin = 0.01, Units = "s"))
	float StrikeDrawTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checklist")
	TArray<FChecklistLine> Lines;

protected:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	TArray<TObjectPtr<UImage>> Strikes;

	/** 0 = not collected, rising to 1 as the strike is drawn */
	TArray<float> StrikeProgress;

	bool IsCollected(const FName& Flag) const;
};
