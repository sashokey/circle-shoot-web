#include "WebFramework.h"
#include "CircleShootApp.h"
#include "Board.h"
#include "MainMenu.h"
#include "AdventureScreen.h"
#include "PracticeScreen.h"
#include "HelpScreen.h"
#include "CircleDialog.h"
#include "CircleButton.h"
#include "CircleCheckbox.h"
#include "WidgetMover.h"
#include "StatsDialog.h"
#include "OptionsDialog.h"
#include "ProfileMgr.h"
#include "HighScoreMgr.h"
#include "LevelParser.h"
#include "DataSync.h"
#include "Res.h"
using namespace Sexy;
void CircleShootApp::MakeBoard()
{
    mDidNextTempleDialog = false;
    CleanupWidgets();

    mBoard = new Board(this);

    mBoard->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mBoard);
    mWidgetManager->SetFocus(mBoard);
    mDoPlayCount = false;
}
void CircleShootApp::StartAdventureGame(int theStage)
{
    if (mDidNextTempleDialog && theStage == mProfile->mMaxStage && CheckSaveGame(false))
    {
        StartSavedGame(true);
    }
    else
    {
        MakeBoard();
        mBoard->SetStartLevel(mLevelParser->GetLevelNumByStage(theStage));
        mBoard->Reset(true);
        if (mProfile->mShowHelpScreen)
        {
            ShowHelpScreen();
        }

        PlaySong(0, 1, 0.01);
    }
}
void CircleShootApp::StartPracticeGame(const std::string &theLevelName, int theStartLevel, bool endless)
{
    MakeBoard();
    mBoard->SetPracticeBoard(theLevelName);
    mBoard->SetStartLevel(theStartLevel);
    mBoard->SetIsEndless(endless);
    mBoard->Reset(true);
    if (mProfile->mShowHelpScreen)
    {
        ShowHelpScreen();
    }

    PlaySong(0, true, 0.01);
}
bool CircleShootApp::CheckSaveGame(bool showConfirm)
{
    std::string aSaveGameName = GetSaveGameName(mIsPractice, mProfile->mId);
    if (!ReadBufferFromFile(aSaveGameName, &mSaveGameBuffer))
        return false;

    DataReader aReader;
    aReader.OpenMemory(mSaveGameBuffer.GetDataPtr(), mSaveGameBuffer.GetDataLen(), false);
    if (aReader.ReadLong() != gSaveGameVersion)
    {
        EraseFile(aSaveGameName);
        mSaveGameBuffer.Clear();
        return false;
    }

    std::string aVerboseLevelString;
    std::string aLevelDisplayName;
    bool aIsPractice = aReader.ReadBool();
    aReader.ReadString(aVerboseLevelString);
    aReader.ReadString(aLevelDisplayName);
    int aScore = aReader.ReadLong();
    if (showConfirm)
    {
        if (aScore > 0)
        {
            DoConfirmContinueDialog(aVerboseLevelString, aLevelDisplayName, aScore);
        }
        else
        {
            EraseFile(aSaveGameName);
            mSaveGameBuffer.Clear();
        }
    }
    else
    {
        EraseFile(aSaveGameName);
    }

    return mSaveGameBuffer.GetDataLen()>0;
}
void CircleShootApp::StartSavedGame(bool showConfirm)
{
    MakeBoard();
    mBoard->SetStartLevel(0);
    mBoard->LoadGame(mSaveGameBuffer);
    mSaveGameBuffer.Clear();
    if (!showConfirm)
    {
        if (mProfile->mShowHelpScreen)
        {
            ShowHelpScreen();
        }
        else
        {
            DoGetReadyDialog();
        }
    }
    PlaySong(0, true, 0.01);
}
void CircleShootApp::ShowHelpScreen()
{
    mHelpScreen = new HelpScreen();
    mHelpScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mHelpScreen);
    mWidgetManager->SetFocus(mHelpScreen);
    if (mBoard)
    {
        mBoard->Pause(true, true);
    }
    ClearUpdateBacklog();
}
void CircleShootApp::EndHelpScreen()
{
    if (mHelpScreen)
    {
        mWidgetMover->SafeDeleteWidget(mHelpScreen);
        mHelpScreen = NULL;
    }

    if (mBoard)
    {
        mWidgetManager->SetFocus(mBoard);

        if (mBoard)
        {
            mBoard->Pause(false, true);
        }

        if (mBoard->IsSavedGame())
        {
            DoGetReadyDialog();
        }
    }
}
void CircleShootApp::DoOptionsDialog()
{
    if (mBoard != NULL)
    {
        mBoard->Pause(true, true);
    }

    Dialog *dialog = new OptionsDialog(mBoard != NULL);
    SetupDialog(dialog, 400);
    AddDialog(DialogType_Options, dialog);
}
void CircleShootApp::FinishOptionsDialog(bool saveSettings)
{
    Sexy::OptionsDialog *dialog = (OptionsDialog *)GetDialog(DialogType_Options);
    if (dialog != NULL)
    {
        if (saveSettings)
        {
            bool fullscreen = dialog->mFullScreenCheckbox->IsChecked();
            bool acceleration = dialog->m3DAccelCheckbox->IsChecked();
            SwitchScreenMode(fullscreen != true, acceleration);
            bool cursorsEnabled = dialog->mCustomCursorsCheckbox->IsChecked();
            EnableCustomCursors(cursorsEnabled);
            ClearUpdateBacklog();
        }

        KillDialog(0);
        if (mBoard)
        {
            mBoard->Pause(false, true);
        }
    }
}
void CircleShootApp::DoStatsDialog(bool slide, bool doCounter)
{
    if (mBoard == NULL)
        return;

    mBoard->Pause(true, true);

    StatsDialog *aDialog = new StatsDialog(mBoard, doCounter);
    SetupDialog(aDialog, 460);
    AddDialog(DialogType_Stats, aDialog);

    if (slide)
    {
        mWidgetMover->MoveWidget(
            aDialog,
            aDialog->mX,
            aDialog->mHeight + 480,
            aDialog->mX,
            aDialog->mY,
            false);
    }
}
void CircleShootApp::FinishStatsDialog(bool confirm)
{
    KillDialog(DialogType_Stats);
    if (mBoard == NULL)
        return;

    if (!mBoard->IsGameOver())
    {
        mBoard->Pause(false, true);
    }
    else if (mBoard->IsPracticeMode())
    {
        ShowPracticeScreen(false);
    }
    else if (!mBoard->IsWinning())
    {
        ShowAdventureScreen(false, false);
    }
    else if (mBoard->GetCurrentStage() < 12)
    {
        ShowAdventureScreen(false, true);
    }
    else
    {
        ShowCreditsScreen(true);
    }
}
void CircleShootApp::DoConfirmContinueDialog(const std::string &theVerboseLevelString, const std::string &theDisplayName, int theScore)
{
    std::string aScore = Sexy::StrFormat(LS(STRING_ID_SCORE), theVerboseLevelString.c_str(), theDisplayName.c_str(), theScore);
    std::string aText = Sexy::StrFormat("%s\r\n"
                        "\r\n"
                        "%s"
                        "\r\n"
                        "\r\n", 
                        LS(STRING_ID_YOUR_GAME_WAS_SAVED_WHEN_YOU_QUIT),
                        LS(STRING_ID_DO_YOU_WANT_TO_CONTINUE_YOUR_LAST_GAME_QM)) +
                        aScore;

    Dialog *aDialog = NewDialog(DialogType_ConfirmContinue, true, LS(STRING_ID_CONTINUE_QM), aText, "", Dialog::BUTTONS_OK_CANCEL);

    aDialog->mYesButton->mLabel = LS(STRING_ID_SM_CONTINUE);
    aDialog->mNoButton->mLabel = LS(STRING_ID_NEW_GAME);

    AddDialog(DialogType_ConfirmContinue, aDialog);
}
void CircleShootApp::FinishConfirmContinueDialog(bool startGame)
{
    KillDialog(DialogType_ConfirmContinue);

    std::string aSaveGameName = GetSaveGameName(mIsPractice, mProfile->mId);
    EraseFile(aSaveGameName);

    if (startGame)
    {
        StartSavedGame(false);
    }
    else if (mIsPractice)
    {
        ShowPracticeScreen(false);
    }
    else
    {
        ShowAdventureScreen(false, false);
    }
}
void CircleShootApp::DoNextTempleDialog()
{
    mDidNextTempleDialog = true;
    DoDialog(DialogType_NextTemple, true, LS(STRING_ID_ENTER_NEXT_TEMPLE), LS(STRING_ID_YOU_ARE_NOW_GOING_TO_ENTER_THE_NEXT_TEMPLE), "", Dialog::BUTTONS_OK_CANCEL);
}
void CircleShootApp::FinishNextTempleDialog(bool save)
{
    KillDialog(DialogType_NextTemple);

    if (save && CheckSaveGame(false))
    {
        StartSavedGame(true);
    }
    else if (mAdventureScreen)
    {
        mWidgetManager->SetFocus(mAdventureScreen);
    }
}
void CircleShootApp::DoGetReadyDialog()
{
    if (mDialogMap.empty())
    {
        DoDialog(DialogType_GetReady, true, LS(STRING_ID_GET_READY), "", LS(STRING_ID_GO), Dialog::BUTTONS_FOOTER);
        if (mBoard)
        {
            mBoard->Pause(true, true);
        }

        mBoard->SetShowBallsDuringPause(true);
        mBoard->SetFullPauseFade();
    }
}
void CircleShootApp::FinishGetReadyDialog()
{
    KillDialog(DialogType_GetReady);
    if (mBoard)
    {
        mBoard->Pause(false, true);
    }
}
void CircleShootApp::DoConfirmMainMenuDialog()
{
    if (mBoard && mBoard->NeedSaveGame())
    {
        Dialog *aDialog = DoDialog(DialogType_ConfirmMainMenu, true, LS(STRING_ID_LEAVE_GAME_QM), LS(STRING_ID_DO_YOU_WANT_TO_CONTINUE_QM), "", Dialog::BUTTONS_OK_CANCEL);
        aDialog->mYesButton->mLabel = LS(STRING_ID_LEAVE);

        OptionsDialog *aDialogOptions = (OptionsDialog *)GetDialog(DialogType_Options);
        if (aDialogOptions)
        {
            aDialog->Resize(aDialog->mX, aDialogOptions->mY + aDialogOptions->mHeight - aDialog->mHeight, aDialog->mWidth, aDialog->mHeight);
        }
    }
    else
    {
        ShowMainMenu();
    }
}
void CircleShootApp::FinishConfirmMainMenuDialog(bool mainMenu)
{
    KillDialog(DialogType_ConfirmMainMenu);
    if (mainMenu)
    {
        ShowMainMenu();
    }
}
void CircleShootApp::DoConfirmQuitDialog()
{
    // HACK: black window left around after quitting android
    // disable this and just rely on the user swiping the app away instead
#if !defined(__ANDROID__)
    DoDialog(DialogType_ConfirmQuit, true, LS(STRING_ID_QUIT_ZUMA_QM), LS(STRING_ID_QUIT_THE_GAME_QM), "", Dialog::BUTTONS_YES_NO);
#endif
}
void CircleShootApp::FinishConfirmQuitDialog(bool confirm)
{
    KillDialog(DialogType_ConfirmQuit);

    if (confirm)
    {
        Shutdown();
    }
}
void CircleShootApp::ShowMainMenu()
{
    if (mPracticeScreen != NULL)
    {
        mWidgetMover->MoveWidget(mPracticeScreen, mPracticeScreen->mX, mPracticeScreen->mY, -mPracticeScreen->mWidth, mPracticeScreen->mY, true);
        mPracticeScreen = NULL;
    }

    if (mAdventureScreen != NULL)
    {
        mWidgetMover->MoveWidget(mAdventureScreen, mAdventureScreen->mX, mAdventureScreen->mY, -mAdventureScreen->mWidth, mAdventureScreen->mY, true);
        mAdventureScreen = NULL;
    }



    FinishOptionsDialog(true);
    CleanupWidgets();

    mMainMenu = new MainMenu();
    mMainMenu->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mMainMenu);
    mWidgetManager->BringToBack(mMainMenu);
    mWidgetManager->SetFocus(mMainMenu);

    if (!mProfile)
    {
        DoCreateUserDialog();
    }

    PlaySong(28, true, 0.01);
    ClearUpdateBacklog();
}
void CircleShootApp::ShowAdventureScreen(bool fromMenu, bool revealTemple)
{
    mIsPractice = false;
    if (fromMenu && CheckSaveGame(true))
        return;

    MainMenu *aMainMenu = mMainMenu;
    mWidgetMover->DelayDeleteWidget(mMainMenu);
    mMainMenu = NULL;

    CleanupWidgets();

    mAdventureScreen = new AdventureScreen();
    mAdventureScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mAdventureScreen);
    mWidgetManager->SetFocus(mAdventureScreen);

    if (revealTemple)
    {
        mAdventureScreen->RevealTemple(75, mProfile->mMaxStage / 3 + 1);
        mAdventureScreen->SetStartNextTempleOnRevel(true);
    }

    if (aMainMenu)
    {
        mWidgetMover->MoveWidget(mAdventureScreen, -mAdventureScreen->mWidth, 0, 0, 0, false);
    }

    PlaySong(32, true, 0.01);
    ClearUpdateBacklog();
}
void CircleShootApp::ShowPracticeScreen(bool fromMenu)
{
    mIsPractice = true;
    if (fromMenu && CheckSaveGame(true))
        return;

    MainMenu *aMainMenu = mMainMenu;
    mWidgetMover->DelayDeleteWidget(mMainMenu);
    mMainMenu = NULL;

    CleanupWidgets();

    mPracticeScreen = new PracticeScreen();
    mPracticeScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mPracticeScreen);
    mWidgetManager->SetFocus(mPracticeScreen);

    if (aMainMenu)
    {
        mWidgetMover->MoveWidget(mPracticeScreen, -mPracticeScreen->mWidth, 0, 0, 0, false);
    }

    PlaySong(34, true, 0.01);
    ClearUpdateBacklog();
}
void CircleShootApp::ReturnToMainMenu()
{
    DoConfirmMainMenuDialog();
}
Dialog *CircleShootApp::NewDialog(int theDialogId,
                                  bool isModal,
                                  const SexyString &theDialogHeader,
                                  const SexyString &theDialogLines,
                                  const SexyString &theDialogFooter,
                                  int theButtonMode)
{
    CircleDialog *aDialog = new CircleDialog(Sexy::IMAGE_DIALOG_BACK,
                                             Sexy::IMAGE_DIALOG_BUTTON,
                                             theDialogId,
                                             isModal,
                                             theDialogHeader,
                                             theDialogLines,
                                             theDialogFooter,
                                             theButtonMode,
                                             false);

    Sexy::SetupDialog(aDialog, 348);
    return aDialog;
}
