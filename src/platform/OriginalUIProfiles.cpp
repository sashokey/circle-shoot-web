#include "WebFramework.h"
#include "CircleShootApp.h"
#include "CircleCommon.h"
#include "CreditsScreen.h"
#include "MainMenu.h"
#include "UserDialog.h"
#include "CreateUserDialog.h"
#include "CircleCommon.h"
#include "WidgetMover.h"
#include "ProfileMgr.h"
#include "Res.h"
using namespace Sexy;
void CircleShootApp::ShowCreditsScreen(bool happyEnd)
{
    CleanupWidgets();
    mCreditsScreen = new CreditsScreen(happyEnd);
    mCreditsScreen->Resize(0, 0, mWidth, mHeight);
    mWidgetManager->AddWidget(mCreditsScreen);
    mWidgetManager->SetFocus(mCreditsScreen);
    PlaySong(0, true, 0.01);
}
void CircleShootApp::DoUserDialog()
{
    KillDialog(DialogType_User);
    UserDialog *aDialog = new UserDialog();
    SetupDialog(aDialog, 400);
    AddDialog(DialogType_User, aDialog);
}
void CircleShootApp::DoCreateUserDialog()
{
    KillDialog(DialogType_CreateUser);
    CreateUserDialog *aDialog = new CreateUserDialog(false);
    SetupDialog(aDialog, 400);
    AddDialog(DialogType_CreateUser, aDialog);
}
void CircleShootApp::DoRenameUserDialog(const std::string &theName)
{
    KillDialog(DialogType_RenameUser);
    CreateUserDialog *aDialog = new CreateUserDialog(true);
    aDialog->SetName(theName);
    SetupDialog(aDialog, 400);
    AddDialog(DialogType_RenameUser, aDialog);
}
void CircleShootApp::DoConfirmDeleteUserDialog(const std::string &theName)
{
    KillDialog(DialogType_ConfirmDeleteUser);

    std::string aText = Sexy::StrFormat(LS(STRING_ID_THIS_WILL_PERMANENTLY_REMOVE_PLAYER_ROSTER), theName.c_str());
    DoDialog(DialogType_ConfirmDeleteUser, true, LS(STRING_ID_ARE_YOU_SURE_QM), aText, "", Dialog::BUTTONS_YES_NO);
}
void CircleShootApp::FinishUserDialog(bool confirm)
{
    UserDialog *aDialog = (UserDialog *)GetDialog(DialogType_User);
    if (!aDialog)
        return;

    if (confirm)
    {
        std::string aName = aDialog->GetSelName();
        UserProfile *aProfile = mProfileMgr->GetProfile(aName);

        if (aProfile)
        {
            mProfile = aProfile;
            mWidgetManager->MarkAllDirty();
            if (mMainMenu)
            {
                mMainMenu->SyncProfile();
            }
        }
    }

    KillDialog(DialogType_User);
}
void CircleShootApp::FinishCreateUserDialog(bool confirm)
{
    CreateUserDialog *aDialog = (CreateUserDialog *)GetDialog(DialogType_CreateUser);
    if (!aDialog)
        return;

    std::string aName = aDialog->GetName();
    if (confirm && aName.empty())
    {
        DoDialog(DialogType_NameEntry, true, 
                 LS(STRING_ID_ENTER_YOUR_NAME),
                 LS(STRING_ID_ENTER_YOUR_NAME_TO_CREATE_A_NEW_USER_PROFILE),
                 LS(DIALOG_BUTTON_OK),
                 Dialog::BUTTONS_FOOTER);
        return;
    }

    if (mProfile)
    {
        if (!confirm)
        {
            KillDialog(DialogType_CreateUser);
            return;
        }
    }
    else if (!confirm || aName.empty())
    {
        DoDialog(DialogType_NameEntry, true, 
                 LS(STRING_ID_ENTER_YOUR_NAME),
                 LS(STRING_ID_ENTER_YOUR_NAME_TO_CREATE_A_NEW_USER_PROFILE),
                 LS(DIALOG_BUTTON_OK),
                 Dialog::BUTTONS_FOOTER);
        return;
    }

    UserProfile *aProfile = mProfileMgr->AddProfile(aName);
    if (aProfile)
    {
        mProfileMgr->Save();
        mProfile = aProfile;
        KillDialog(DialogType_User);
        KillDialog(DialogType_CreateUser);
        mWidgetManager->MarkAllDirty();
        if (mMainMenu)
        {
            mMainMenu->SyncProfile();
        }
    }
    else
    {
        // yup, it's 10 here
        DoDialog(DialogType_NameEntry, true, 
                 LS(STRING_ID_NAME_CONFLICT),
                 LS(STRING_ID_THE_NAME_YOU_ENTERED_IS_ALREADY_BEING_USED),
                 LS(DIALOG_BUTTON_OK),
                 Dialog::BUTTONS_FOOTER);
    }
}
void CircleShootApp::FinishRenameUserDialog(bool confirm)
{
    if (!confirm)
    {
        KillDialog(DialogType_RenameUser);
        return;
    }

    UserDialog *aUserDialog = (UserDialog *)GetDialog(DialogType_User);
    CreateUserDialog *aCreateDialog = (CreateUserDialog *)GetDialog(DialogType_RenameUser);
    if (!aUserDialog || !aCreateDialog)
        return;

    std::string aSelName = aUserDialog->GetSelName();
    std::string aNewName = aCreateDialog->GetName();

    if (!aNewName.empty())
    {
        if (mProfileMgr->RenameProfile(aSelName, aNewName))
        {
            mProfileMgr->Save();
            if (strcasecmp(aSelName.c_str(), aNewName.c_str()) != 0)
            {
                mProfile = mProfileMgr->GetProfile(aNewName);
            }
            aUserDialog->FinishRenameUser(aNewName);
            mWidgetManager->MarkAllDirty();
            KillDialog(DialogType_RenameUser);
        }
        else
        {
            DoDialog(DialogType_NameConflict, true, 
                     LS(STRING_ID_NAME_CONFLICT),
                     LS(STRING_ID_THE_NAME_YOU_ENTERED_IS_ALREADY_BEING_USED),
                     LS(DIALOG_BUTTON_OK),
                     Dialog::BUTTONS_FOOTER);
        }
    }
}
void CircleShootApp::FinishConfirmDeleteUserDialog(bool confirm)
{
    KillDialog(DialogType_ConfirmDeleteUser);
    if (!confirm)
        return;

    UserDialog *aDialog = (UserDialog *)GetDialog(DialogType_User);
    if (!aDialog)
        return;

    std::string aCurProfileName = mProfile ? mProfile->mName : "";
    std::string aSelName = aDialog->GetSelName();

    if (aSelName == aCurProfileName)
    {
        mProfile = NULL;
    }

    mProfileMgr->DeleteProfile(aSelName);
    aDialog->FinishDeleteUser();

    if (!mProfile)
    {
        std::string aNewName = aDialog->GetSelName();
        mProfile = mProfileMgr->GetProfile(aNewName);
        if (!mProfile)
        {
            mProfile = mProfileMgr->GetAnyProfile();
        }
    }

    mProfileMgr->Save();

    if (!mProfile)
    {
        DoCreateUserDialog();
    }

    mWidgetManager->MarkAllDirty();

    if (mMainMenu)
    {
        mMainMenu->SyncProfile();
    }
}
