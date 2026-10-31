#include "WebFramework.h"
#include "CircleCommon.h"
#include "CircleButton.h"
#include "CircleCheckbox.h"
#include "Res.h"
#include <climits>
using namespace Sexy;
void Sexy::SetupButton(DialogButton *theButton, int numCols)
{
    if (theButton)
    {
        theButton->mTranslateX = -1;
        theButton->mTranslateY = 1;

        int width = theButton->mComponentImage->mWidth / numCols;
        int height = theButton->mComponentImage->mHeight;

        theButton->mNormalRect.mHeight = height;
        theButton->mNormalRect.mWidth = width;
        theButton->mNormalRect.mY = 0;
        theButton->mNormalRect.mX = 0;
        theButton->mOverRect.mHeight = height;
        theButton->mOverRect.mWidth = width;
        theButton->mOverRect.mY = 0;
        theButton->mOverRect.mX = width;
        theButton->mDownRect.mHeight = height;
        theButton->mDownRect.mWidth = width;
        theButton->mDownRect.mY = 0;
        theButton->mDownRect.mX = 2 * width;

        theButton->mTextOffsetX = 4;
        theButton->mTextOffsetY = -8;

        theButton->SetFont(Sexy::FONT_DIALOG); // TODO
        theButton->SetColor(0, Sexy::Color(0xD5E520));
        theButton->SetColor(1, Sexy::Color(0xFFFFFF));

        theButton->mHasAlpha = true;
        theButton->mHasTransparencies = true;

        if (theButton->mWidth == 0)
        {
            theButton->Resize(theButton->mX,
                              theButton->mY,
                              theButton->mFont->StringWidth(theButton->mLabel),
                              theButton->mComponentImage->mHeight);
        }
    }
}
CircleButton *Sexy::MakeSpoofButton(ButtonListener *theListener, int w, int h)
{
    MemoryImage *img = new MemoryImage();
    img->Create(w, h);
    return Sexy::MakeButton(INT_MAX, theListener, "", 0, img, 1);
}
CircleButton *Sexy::MakeButton(int id, ButtonListener *theListener, std::string const &label, int flags, Sexy::Image *image, int numCols)
{
    bool customImage;

    if (image)
    {
        customImage = true;
    }
    else
    {
        image = Sexy::IMAGE_DIALOG_BUTTON;
        customImage = false;
    }

    CircleButton *theButton = new CircleButton(image, id, theListener);
    theButton->mLabel = label;

    if ((flags & CircleButton::CB_ClickSound) != 0)
        theButton->mClickSound = Sexy::SOUND_BUTTON1;

    if ((flags & CircleButton::CB_ChoralSound) != 0)
    {
        int pitchTable[5];
        pitchTable[0] = 2;
        pitchTable[1] = 4;
        pitchTable[2] = 0;
        pitchTable[3] = -12;
        pitchTable[4] = -5;

        double pitch = pitchTable[id % 5] + 10;
        theButton->mHoverSound = Sexy::GetSoundById(Sexy::SOUND_CHORAL1_ID);
        theButton->mPitch = pitch;
    }

    if (customImage)
    {
        theButton->mWidth = image->mWidth / numCols;
        theButton->mHeight = image->mHeight;
    }

    SetupButton(theButton, numCols);

    if (customImage)
    {
        theButton->mButtonImage = image;
        theButton->mComponentImage = NULL;
    }

    return theButton;
}
CircleCheckbox *Sexy::MakeCheckbox(int id, CheckboxListener *theListener)
{
    CircleCheckbox *theCheckbox = new CircleCheckbox(Sexy::IMAGE_DIALOG_CHECKBOX, Sexy::IMAGE_DIALOG_CHECKBOX, id, theListener);
    theCheckbox->mCheckedRect.mHeight = Sexy::IMAGE_DIALOG_CHECKBOX->mHeight;
    theCheckbox->mCheckedRect.mWidth = Sexy::IMAGE_DIALOG_CHECKBOX->mWidth / 2;
    theCheckbox->mCheckedRect.mY = 0;
    theCheckbox->mCheckedRect.mX = 0;
    theCheckbox->mUncheckedRect.mHeight = Sexy::IMAGE_DIALOG_CHECKBOX->mHeight;
    theCheckbox->mUncheckedRect.mWidth = Sexy::IMAGE_DIALOG_CHECKBOX->mWidth / 2;
    theCheckbox->mUncheckedRect.mY = 0;
    theCheckbox->mUncheckedRect.mX = Sexy::IMAGE_DIALOG_CHECKBOX->mWidth / 2;

    theCheckbox->Resize(0, 0, Sexy::IMAGE_DIALOG_CHECKBOX->mWidth / 2, Sexy::IMAGE_DIALOG_CHECKBOX->mHeight);

    theCheckbox->mHasTransparencies = true;
    theCheckbox->mHasAlpha = true;
    theCheckbox->mClickSound = Sexy::SOUND_BUTTON2;

    return theCheckbox;
}
void Sexy::DrawCheckboxText(Graphics *g, std::string const &theText, Widget *theWidget)
{
    int aX = theWidget->mX + theWidget->mWidth - g->mTransX;
    int aY = theWidget->mY - g->mTransY;

    int aStrWidth = g->GetFont()->StringWidth(theText);
    g->DrawString(theText, aX, aY + 25);

    Image *aImage = Sexy::IMAGE_DIALOG_CHECKBOXLINE;
    int aStartX = aX - 5;
    int aEndX = aStrWidth + 5;
    int aY2 = aY + 29;

    for (int i = 0; i < aEndX; i += aImage->GetWidth())
    {
        Rect aRect(0, 0, aEndX - i, aImage->GetHeight());
        int aWidth = aEndX - i;
        if (aWidth > aImage->GetWidth())
            aWidth = aImage->GetWidth();
        aRect.mWidth = aWidth;
        g->DrawImage(aImage, i + aStartX, aY2, aRect);
    }

    g->DrawImage(Sexy::IMAGE_DIALOG_CHECKBOXCAP, aEndX + aStartX, aY2);
}
void Sexy::SetupDialog(Dialog *theDialog, int theMinWidth)
{
    theDialog->SetHeaderFont(Sexy::FONT_TITLE);
    theDialog->SetLinesFont(Sexy::FONT_DIALOG);
    theDialog->SetColor(0, Sexy::Color(203, 201, 187));
    theDialog->SetColor(1, Sexy::Color(0xD5E520));

    int aWinWidth = theDialog->mBackgroundInsets.mRight + theDialog->mBackgroundInsets.mLeft + theDialog->mContentInsets.mRight + theDialog->mContentInsets.mLeft + Sexy::FONT_TITLE->StringWidth(theDialog->mDialogHeader) + 20;

    if (aWinWidth <= theMinWidth)
        aWinWidth = theMinWidth;

    Sexy::Dialog *topDialog;
    if (!gSexyAppBase->mDialogMap.empty())
    {
        topDialog = gSexyAppBase->mDialogMap.begin()->second;
    }
    else
    {
        topDialog = NULL;
    }

    int winY = 90 - theDialog->mBackgroundInsets.mTop;
    if (topDialog != NULL)
    {
        winY += topDialog->mY + 20;
    }

    int winHeight = theDialog->GetPreferredHeight(aWinWidth);
    int screenWidth = 640;
    if (winY + winHeight >= 471)
    {
        winY = 10;
    }
    
    // make sure dialog is above onscreen keyboard
#if defined (__ANDROID__) || defined(__SWITCH__)
    winY /= 4;
#endif

    theDialog->Resize((screenWidth - aWinWidth) / 2, winY, aWinWidth, winHeight);

    SetupButton(theDialog->mYesButton, 3);
    SetupButton(theDialog->mNoButton, 3);
}
