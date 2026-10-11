#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScreenWidgets.generated.h"

class UPlayerScreensComponent;
class UImage;
class UTexture2D;
class UVerticalBox;
class UButton;

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
