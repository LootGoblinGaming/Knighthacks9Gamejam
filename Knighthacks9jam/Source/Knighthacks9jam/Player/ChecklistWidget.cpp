#include "ChecklistWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

UChecklistWidget::UChecklistWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> PaperFinder(TEXT("/Game/UI/T_Checklist.T_Checklist"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> StrikeFinder(TEXT("/Game/UI/T_ChecklistStrike.T_ChecklistStrike"));
	PaperTexture = PaperFinder.Object;
	StrikeTexture = StrikeFinder.Object;

	// measured from the hand-drawn paper art, top to bottom
	auto AddLine = [this](const TCHAR* Flag, float Left, float Right, float CenterY)
	{
		FChecklistLine& Line = Lines.AddDefaulted_GetRef();
		Line.CollectedFlag = FName(Flag);
		Line.Left = Left;
		Line.Right = Right;
		Line.CenterY = CenterY;
	};
	AddLine(TEXT("HasWater"), 0.27f, 0.62f, 0.371f);
	AddLine(TEXT("HasFirstAid"), 0.26f, 0.76f, 0.447f);
	AddLine(TEXT("HasFlashlight"), 0.26f, 0.69f, 0.549f);
	AddLine(TEXT("HasDocuments"), 0.22f, 0.77f, 0.663f);
	AddLine(TEXT("HasFood"), 0.20f, 0.49f, 0.769f);
	AddLine(TEXT("HasRadio"), 0.18f, 0.53f, 0.892f);
}

bool UChecklistWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Canvas"));
		WidgetTree->RootWidget = Canvas;

		const float AspectRatio = PaperTexture && PaperTexture->GetSizeX() > 0 ? static_cast<float>(PaperTexture->GetSizeY()) / PaperTexture->GetSizeX() : 1.235f;
		const FVector2D PaperSize(PaperWidth, PaperWidth * AspectRatio);

		UImage* Paper = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Paper"));
		if (PaperTexture)
		{
			Paper->SetBrushFromTexture(PaperTexture);
		}
		UCanvasPanelSlot* PaperSlot = Canvas->AddChildToCanvas(Paper);
		PaperSlot->SetPosition(ScreenOffset);
		PaperSlot->SetSize(PaperSize);

		for (const FChecklistLine& Line : Lines)
		{
			UImage* Strike = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			if (StrikeTexture)
			{
				Strike->SetBrushFromTexture(StrikeTexture);
			}

			// drawn left to right by scaling from the left edge
			Strike->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
			Strike->SetRenderScale(FVector2D(0.0f, 1.0f));

			const FVector2D StrikeSize((Line.Right - Line.Left) * PaperSize.X, StrikeHeight * PaperSize.Y);
			UCanvasPanelSlot* StrikeSlot = Canvas->AddChildToCanvas(Strike);
			StrikeSlot->SetPosition(ScreenOffset + FVector2D(Line.Left * PaperSize.X, Line.CenterY * PaperSize.Y - StrikeSize.Y * 0.5f));
			StrikeSlot->SetSize(StrikeSize);

			Strikes.Add(Strike);
			StrikeProgress.Add(0.0f);
		}
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	return true;
}

void UChecklistWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (int32 Index = 0; Index < Strikes.Num() && Index < Lines.Num(); Index++)
	{
		if (StrikeProgress[Index] >= 1.0f || !IsCollected(Lines[Index].CollectedFlag))
		{
			continue;
		}

		StrikeProgress[Index] = FMath::Min(StrikeProgress[Index] + InDeltaTime / StrikeDrawTime, 1.0f);
		const float Eased = FMath::InterpEaseOut(0.0f, 1.0f, StrikeProgress[Index], 2.0f);
		Strikes[Index]->SetRenderScale(FVector2D(Eased, 1.0f));
	}
}

bool UChecklistWidget::IsCollected(const FName& Flag) const
{
	const APawn* Player = GetOwningPlayerPawn();
	if (!Player || Flag.IsNone())
	{
		return false;
	}

	// the Has* flags are Blueprint variables on the player, so look them up by name
	const FBoolProperty* FlagProperty = FindFProperty<FBoolProperty>(Player->GetClass(), Flag);
	return FlagProperty && FlagProperty->GetPropertyValue_InContainer(Player);
}
