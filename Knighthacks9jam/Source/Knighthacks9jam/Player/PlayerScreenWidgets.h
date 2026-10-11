#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScreenWidgets.generated.h"

class UPlayerScreensComponent;
class UImage;
class UTexture2D;
class UVerticalBox;
class UButton;
class UTextBlock;

/**
 * Placeholder screens built in code, so pause/game over/damage feedback work with no widget assets.
 * A Blueprint subclass that designs its own layout keeps it - the code layout is only built when the
 * widget has no root yet. Custom replacement widgets can call UPlayerScreensComponent's functions instead.
 */

/** Full-screen, click-through damage tint. Shows BloodTexture if set, otherwise a flat red wash. */
UCLASS()
class KNIGHTHACKS9JAM_API UDamageOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetBloodTexture(UTexture2D* BloodTexture);

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY()
	TObjectPtr<UImage> OverlayImage;
};

/** Dimmed full-screen menu: a title and a column of buttons that drive UPlayerScreensComponent */
UCLASS(Abstract)
class KNIGHTHACKS9JAM_API UPlayerMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetScreensComponent(UPlayerScreensComponent* InScreensComponent) { ScreensComponent = InScreensComponent; }

protected:
	virtual bool Initialize() override;

	/** Subclasses add their title and buttons here */
	virtual void BuildMenu(UVerticalBox* Column) {}

	void AddTitle(UVerticalBox* Column, const FText& Text, const FLinearColor& Color);
	UButton* AddButton(UVerticalBox* Column, const FText& Label);

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	TWeakObjectPtr<UPlayerScreensComponent> ScreensComponent;
};

UCLASS()
class KNIGHTHACKS9JAM_API UPauseMenuWidget : public UPlayerMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildMenu(UVerticalBox* Column) override;
};

UCLASS()
class KNIGHTHACKS9JAM_API UGameOverWidget : public UPlayerMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildMenu(UVerticalBox* Column) override;
};

UCLASS()
class KNIGHTHACKS9JAM_API UVictoryWidget : public UPlayerMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void BuildMenu(UVerticalBox* Column) override;
};

/** Click-through "+" crosshair at the centre of the screen, where hitscan weapons hit */
UCLASS()
class KNIGHTHACKS9JAM_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Length of each crosshair line */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (ClampMin = 2))
	float Size = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (ClampMin = 1))
	float Thickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair")
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.85f);

protected:
	virtual bool Initialize() override;

private:
	void AddLine(class UOverlay* Overlay, float Width, float Height);
};

/**
 * BotW-style stamina wheel: a ring around the screen centre that empties as stamina drains, turns red when
 * exhausted, and fades away once full. Drawn in code, so it needs no textures or materials.
 */
UCLASS()
class KNIGHTHACKS9JAM_API UStaminaWheelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Ring radius in pixels; keep it larger than the crosshair */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina Wheel", meta = (ClampMin = 4))
	float Radius = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina Wheel", meta = (ClampMin = 1))
	float Thickness = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina Wheel")
	FLinearColor FillColor = FLinearColor(0.35f, 1.0f, 0.35f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina Wheel")
	FLinearColor ExhaustedColor = FLinearColor(1.0f, 0.25f, 0.2f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina Wheel")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.45f);

	void SetStamina(float InFraction, bool bInExhausted);

protected:
	virtual bool Initialize() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	float Fraction = 1.0f;
	bool bExhausted = false;

	void DrawArc(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& AllottedGeometry, const FWidgetStyle& InWidgetStyle, float ArcFraction, const FLinearColor& Color) const;
};

/** Click-through text banner near the bottom of the screen, for short hints like "Collect all supplies first" */
UCLASS()
class KNIGHTHACKS9JAM_API UMessageToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(const FText& Message);

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> MessageText;
};
