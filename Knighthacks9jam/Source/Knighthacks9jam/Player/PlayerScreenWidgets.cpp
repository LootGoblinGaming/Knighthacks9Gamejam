#include "PlayerScreenWidgets.h"
#include "PlayerScreensComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "PlayerScreens"

bool UDamageOverlayWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		OverlayImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("OverlayImage"));
		OverlayImage->SetColorAndOpacity(FLinearColor(0.55f, 0.0f, 0.0f, 0.5f));
		WidgetTree->RootWidget = OverlayImage;
	}

	// never block clicks or aim
	SetVisibility(ESlateVisibility::HitTestInvisible);
	return true;
}

void UDamageOverlayWidget::SetBloodTexture(UTexture2D* BloodTexture)
{
	if (OverlayImage && BloodTexture)
	{
		OverlayImage->SetBrushFromTexture(BloodTexture);
		OverlayImage->SetColorAndOpacity(FLinearColor::White);
	}
}

bool UPlayerMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
		Background->SetHorizontalAlignment(HAlign_Center);
		Background->SetVerticalAlignment(VAlign_Center);

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
		Background->SetContent(Column);
		WidgetTree->RootWidget = Background;

		BuildMenu(Column);
	}
	return true;
}

void UPlayerMenuWidget::AddTitle(UVerticalBox* Column, const FText& Text, const FLinearColor& Color)
{
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(Text);
	Title->SetColorAndOpacity(FSlateColor(Color));
	Title->SetJustification(ETextJustify::Center);

	FSlateFontInfo Font = Title->GetFont();
	Font.Size = 64;
	Title->SetFont(Font);

	UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title);
	TitleSlot->SetHorizontalAlignment(HAlign_Center);
	TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 32.0f));
}

UButton* UPlayerMenuWidget::AddButton(UVerticalBox* Column, const FText& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());

	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonText->SetText(Label);
	ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f)));
	ButtonText->SetJustification(ETextJustify::Center);

	FSlateFontInfo Font = ButtonText->GetFont();
	Font.Size = 28;
	ButtonText->SetFont(Font);

	Button->SetContent(ButtonText);

	UVerticalBoxSlot* ButtonSlot = Column->AddChildToVerticalBox(Button);
	ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
	ButtonSlot->SetPadding(FMargin(0.0f, 8.0f));
	return Button;
}

void UPlayerMenuWidget::OnResumeClicked()
{
	if (ScreensComponent.IsValid())
	{
		ScreensComponent->ResumeGame();
	}
}

void UPlayerMenuWidget::OnRestartClicked()
{
	if (ScreensComponent.IsValid())
	{
		ScreensComponent->RestartLevel();
	}
}

void UPlayerMenuWidget::OnMainMenuClicked()
{
	if (ScreensComponent.IsValid())
	{
		ScreensComponent->QuitToMainMenu();
	}
}

void UPauseMenuWidget::BuildMenu(UVerticalBox* Column)
{
	AddTitle(Column, LOCTEXT("PausedTitle", "PAUSED"), FLinearColor::White);
	AddButton(Column, LOCTEXT("Resume", "Resume"))->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	AddButton(Column, LOCTEXT("Restart", "Restart"))->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	AddButton(Column, LOCTEXT("MainMenu", "Main Menu"))->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
}

void UGameOverWidget::BuildMenu(UVerticalBox* Column)
{
	AddTitle(Column, LOCTEXT("GameOverTitle", "YOU WERE INFECTED"), FLinearColor(0.8f, 0.05f, 0.05f));
	AddButton(Column, LOCTEXT("TryAgain", "Try Again"))->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
	AddButton(Column, LOCTEXT("GameOverMainMenu", "Main Menu"))->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
}

void UVictoryWidget::BuildMenu(UVerticalBox* Column)
{
	AddTitle(Column, LOCTEXT("VictoryTitle", "YOU ESCAPED"), FLinearColor(0.2f, 0.95f, 0.5f));
	AddButton(Column, LOCTEXT("PlayAgain", "Play Again"))->OnClicked.AddDynamic(this, &UVictoryWidget::OnRestartClicked);
	AddButton(Column, LOCTEXT("VictoryMainMenu", "Main Menu"))->OnClicked.AddDynamic(this, &UVictoryWidget::OnMainMenuClicked);
}

bool UMessageToastWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		// transparent full-screen frame that pins the banner to the lower middle of the screen
		UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Frame"));
		Frame->SetBrushColor(FLinearColor::Transparent);
		Frame->SetHorizontalAlignment(HAlign_Center);
		Frame->SetVerticalAlignment(VAlign_Bottom);
		Frame->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 180.0f));

		UBorder* Banner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Banner"));
		Banner->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));
		Banner->SetPadding(FMargin(24.0f, 12.0f));
		Frame->SetContent(Banner);

		MessageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MessageText"));
		MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		MessageText->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = MessageText->GetFont();
		Font.Size = 26;
		MessageText->SetFont(Font);
		Banner->SetContent(MessageText);

		WidgetTree->RootWidget = Frame;
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	return true;
}

void UMessageToastWidget::SetMessage(const FText& Message)
{
	if (MessageText)
	{
		MessageText->SetText(Message);
	}
}

#undef LOCTEXT_NAMESPACE
