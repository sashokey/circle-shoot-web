#pragma once
#include "SexyAppFramework/Common.h"
#include "SexyAppFramework/Rect.h"
#include "SexyAppFramework/SexyVector.h"
#include "SexyAppFramework/SexyMatrix.h"
#include "SexyAppFramework/MTRand.h"
#include "SexyAppFramework/Buffer.h"
struct SDL_GameController {};
struct SDL_Event {};
struct SDL_Thread {};
struct SDL_Point { int x, y; };
namespace Sexy {
class SexyAppBase;
class Graphics;
class WidgetManager;
class Image;
class MemoryImage;
class Font;
class Widget;
class Dialog;
extern SexyAppBase* gSexyAppBase;
enum KeyCode { KEYCODE_UNKNOWN = 0, KEYCODE_BACK = 8, KEYCODE_RETURN = 13, KEYCODE_ESCAPE = 27, KEYCODE_SPACE = 32, KEYCODE_LEFT = 37, KEYCODE_UP = 38, KEYCODE_RIGHT = 39, KEYCODE_DOWN = 40, KEYCODE_F1 = 112, KEYCODE_F2 = 113, KEYCODE_F3 = 114, KEYCODE_F4 = 115, KEYCODE_F5 = 116, KEYCODE_F6 = 117, KEYCODE_F7 = 118, KEYCODE_F8 = 119, KEYCODE_F9 = 120, KEYCODE_F10 = 121, KEYCODE_F11 = 122, KEYCODE_F12 = 123 };
class Color {
public:
 int mRed, mGreen, mBlue, mAlpha;
 Color(int value = 0) : mRed((value >> 16) & 255), mGreen((value >> 8) & 255), mBlue(value & 255), mAlpha(255) {}
 Color(int r, int g, int b, int a = 255) : mRed(r), mGreen(g), mBlue(b), mAlpha(a) {}
 uint32_t ToInt() const { return uint32_t(mAlpha) << 24 | mRed << 16 | mGreen << 8 | mBlue; }
 static const Color White;
 static const Color Black;
};
class Image {
public:
 int mWidth = 1, mHeight = 1, mNumRows = 1, mNumCols = 1, mId = 0;
 bool mHasAlpha = true, mHasTrans = true;
 virtual ~Image() {}
 int GetWidth() const { return mWidth; }
 int GetHeight() const { return mHeight; }
 int GetCelWidth() const { return mWidth / mNumCols; }
 int GetCelHeight() const { return mHeight / mNumRows; }
 Rect GetCelRect(int cel) const { return Rect((cel % mNumCols) * GetCelWidth(), (cel / mNumCols) * GetCelHeight(), GetCelWidth(), GetCelHeight()); }
};
class MemoryImage : public Image {
 std::vector<uint32_t> mPixels;
public:
 MemoryImage(SexyAppBase* app = nullptr) {}
 MemoryImage(const MemoryImage& other);
 ~MemoryImage();
 void Create(int width, int height);
 uint32_t* GetBits();
 void BitsChanged();
 void SetBits(uint32_t* bits, int width, int height, bool commit = true);
 void CommitBits() {}
 void Clear();
 void SetImageMode(bool alpha, bool trans) { mHasAlpha = alpha; mHasTrans = trans; }
 void SetVolatile(bool value) {}
 void SetPurgeBits(bool value) {}
 void SetHasAlpha(bool value) { mHasAlpha = value; }
 void SetHasTrans(bool value) { mHasTrans = value; }
 void PurgeBits() {}
};
class Font {
public:
 int mId = 0, mHeight = 18, mAscent = 14, mAscentPadding = 0, mLineSpacing = 20;
 int StringWidth(const std::string& value) const;
 int CharWidth(char value) const;
 int CharWidthKern(char value, char previous) const;
 Font* Duplicate() const { return new Font(*this); }
 int GetHeight() const { return mHeight; }
 int GetAscent() const { return mAscent; }
 int GetAscentPadding() const { return mAscentPadding; }
 int GetLineSpacing() const { return mLineSpacing; }
};
class Graphics {
public:
 enum { DRAWMODE_NORMAL, DRAWMODE_ADDITIVE };
 Image* mDestImage;
 Color mColor;
 Font* mFont;
 Rect mClipRect;
 int mDrawMode = 0;
 bool mColorizeImages = false, mFastStretch = false;
 float mTransX = 0, mTransY = 0;
 Graphics(Image* target = nullptr) : mDestImage(target), mColor(Color::White), mFont(nullptr), mClipRect(0, 0, target ? target->mWidth : 640, target ? target->mHeight : 480) {}
 void SetFont(Font* font) { mFont = font; }
 Font* GetFont() { return mFont; }
 void SetColor(const Color& color) { mColor = color; }
 const Color& GetColor() { return mColor; }
 void SetDrawMode(int mode) { mDrawMode = mode; }
 void SetColorizeImages(bool value) { mColorizeImages = value; }
 void SetFastStretch(bool value) { mFastStretch = value; }
 bool GetFastStretch() { return mFastStretch; }
 void SetLinearBlend(bool value) {}
 void Translate(int x, int y) { mTransX += x; mTransY += y; }
 void TranslateF(float x, float y) { mTransX += x; mTransY += y; }
 void FillRect(int x, int y, int width, int height);
 void FillRect(const Rect& rect) { FillRect(rect.mX, rect.mY, rect.mWidth, rect.mHeight); }
 void DrawRect(int x, int y, int width, int height);
 void DrawRect(const Rect& rect) { DrawRect(rect.mX, rect.mY, rect.mWidth, rect.mHeight); }
 void DrawLine(int x1, int y1, int x2, int y2);
 void DrawLineAA(int x1, int y1, int x2, int y2) { DrawLine(x1, y1, x2, y2); }
 void PolyFill(const Point* points, int count, bool convex = false);
 void DrawString(const std::string& text, int x, int y);
 int StringWidth(const std::string& text) { return mFont ? mFont->StringWidth(text) : text.size() * 10; }
 void DrawImage(Image* image, int x, int y);
 void DrawImage(Image* image, int x, int y, const Rect& source);
 void DrawImage(Image* image, const Rect& destination, const Rect& source);
 void DrawImage(Image* image, int x, int y, int width, int height);
 void DrawImageF(Image* image, float x, float y);
 void DrawImageF(Image* image, float x, float y, const Rect& source);
 void DrawImageRotated(Image* image, int x, int y, double rotation, const Rect* source = nullptr);
 void DrawImageRotated(Image* image, int x, int y, double rotation, int cx, int cy, const Rect* source = nullptr);
 void DrawImageRotatedF(Image* image, float x, float y, double rotation, const Rect* source = nullptr);
 void DrawImageRotatedF(Image* image, float x, float y, double rotation, float cx, float cy, const Rect* source = nullptr);
 void DrawImageCel(Image* image, int x, int y, int cel);
 void DrawImageCel(Image* image, const Rect& destination, int cel);
 void SetClipRect(int x, int y, int width, int height) { mClipRect = Rect(x, y, width, height); }
 void SetClipRect(const Rect& r) { mClipRect = r; }
 void ClipRect(int x, int y, int width, int height) { mClipRect = mClipRect.Intersection(Rect(x + mTransX, y + mTransY, width, height)); }
 void ClipRect(const Rect& rect) { ClipRect(rect.mX, rect.mY, rect.mWidth, rect.mHeight); }
 void ClearClipRect() { mClipRect = Rect(0, 0, 640, 480); }
 void DrawImageBox(const Rect& destination, Image* image);
 void DrawImageBox(const Rect& source, const Rect& destination, Image* image);
 bool Is3D() { return true; }
};
class ButtonListener { public: virtual ~ButtonListener() {} virtual void ButtonPress(int id) {} virtual void ButtonDepress(int id) {} virtual void ButtonDownTick(int id) {} };
class CheckboxListener { public: virtual ~CheckboxListener() {} virtual void CheckboxChecked(int id, bool checked) {} };
class SliderListener { public: virtual ~SliderListener() {} virtual void SliderVal(int id, double value) {} };
class ListListener { public: virtual ~ListListener() {} virtual void ListClicked(int id,int index,int count) {} };
class EditListener { public: virtual ~EditListener() {} virtual void EditWidgetText(int id,const std::string& text) {} virtual bool AllowChar(int id,char value) { return true; } };
struct Insets { int mLeft, mTop, mRight, mBottom; Insets(int left=0, int top=0, int right=0, int bottom=0) : mLeft(left), mTop(top), mRight(right), mBottom(bottom) {} };
enum { LAY_SameWidth=1, LAY_SameHeight=2, LAY_SameSize=3, LAY_SetLeft=0x10, LAY_SetTop=0x20, LAY_SetWidth=0x40, LAY_SetHeight=0x80, LAY_Above=0x100, LAY_Below=0x200, LAY_Right=0x400, LAY_Left=0x800, LAY_SameLeft=0x1000, LAY_SameRight=0x2000, LAY_SameTop=0x4000, LAY_SameBottom=0x8000, LAY_GrowToRight=0x10000, LAY_GrowToLeft=0x20000, LAY_GrowToTop=0x40000, LAY_GrowToBottom=0x80000, LAY_HCenter=0x100000, LAY_VCenter=0x200000, LAY_Max=0x400000 };
enum { CURSOR_POINTER, CURSOR_HAND, CURSOR_DRAGGING, CURSOR_TEXT };
class Widget {
public:
 int mX = 0, mY = 0, mWidth = 0, mHeight = 0, mUpdateCnt = 0;
 bool mVisible = true, mDisabled = false, mMouseVisible = true, mHasTransparencies = true, mHasAlpha = true, mIsDown = false, mIsOver = false;
 WidgetManager* mWidgetManager = nullptr;
 Widget* mOwner = nullptr;
 std::vector<Color> mColors = std::vector<Color>(8, Color::White);
 virtual ~Widget() {}
 virtual void Update() { ++mUpdateCnt; }
 virtual void Draw(Graphics* g) {}
 virtual void Resize(int x, int y, int width, int height) { mX = x; mY = y; mWidth = width; mHeight = height; }
 virtual void MouseMove(int x, int y) {}
 virtual void MouseDrag(int x, int y) {}
 virtual void MouseDown(int x, int y, int count) {}
 virtual void MouseDown(int x, int y, int button, int count) { MouseDown(x, y, button < 0 ? -count : count); }
 virtual void MouseUp(int x, int y, int count) {}
 virtual void MouseUp(int x, int y) { MouseUp(x, y, 1); }
 virtual void MouseUp(int x, int y, int button, int count) { MouseUp(x, y, count); }
 virtual void MouseEnter() {}
 virtual void MouseLeave() {}
 virtual void OrderInManagerChanged() {}
 virtual void KeyChar(char key) {}
 virtual void KeyDown(KeyCode key) {}
 virtual void AddedToManager(WidgetManager* manager) { mWidgetManager = manager; }
 virtual void RemovedFromManager(WidgetManager* manager) { mWidgetManager = nullptr; }
 virtual bool IsPointVisible(int x, int y) { return Rect(0, 0, mWidth, mHeight).Contains(x, y); }
 virtual std::string AccessibilityLabel() { return ""; }
 void MarkDirty() {}
 void SetColors(int colors[][3],int count) { for(int i=0;i<count;++i)SetColor(i,Color(colors[i][0],colors[i][1],colors[i][2])); }
 void SetVisible(bool value) { mVisible = value; }
 void SetDisabled(bool value) { mDisabled = value; }
 virtual void SetColor(int index, const Color& color) { if (index >= int(mColors.size())) mColors.resize(index + 1); mColors[index] = color; }
 Color GetColor(int index, const Color& fallback=Color::White) { return index < int(mColors.size()) ? mColors[index] : fallback; }
 void Move(int x, int y) { Resize(x, y, mWidth, mHeight); }
 int Width() const { return mWidth; }
 int Height() const { return mHeight; }
 int Left() const { return mX; }
 int Top() const { return mY; }
 int Right() const { return mX + mWidth; }
 int Bottom() const { return mY + mHeight; }
 void Layout(int flags, Widget* relative, int left, int top, int width, int height);
 void AddWidget(Widget* w) { w->mWidgetManager = mWidgetManager; }
 void RemoveWidget(Widget* w) {}
 void WriteCenteredLine(Graphics* g, int y, const std::string& text) { g->DrawString(text, (mWidth - g->StringWidth(text)) / 2, y); }
 static int WriteWordWrapped(Graphics* g, const Rect& rect, const std::string& value, int lineSpacing=-1, int align=0);
 static int GetWordWrappedHeight(Graphics* g, int width, const std::string& value, int lineSpacing=-1);
};
class ButtonWidget : public Widget {
public:
 Image* mButtonImage = nullptr;
 Rect mNormalRect, mOverRect, mDownRect, mDisabledRect;
 std::string mLabel;
 Font* mFont = nullptr;
 int mId = 0;
 ButtonListener* mButtonListener = nullptr;
 bool mInverted = false, mBtnNoDraw = false;
 double mOverAlpha=0, mOverAlphaSpeed=0, mOverAlphaFadeInSpeed=0;
 enum { COLOR_LABEL, COLOR_LABEL_HILITE };
 void SetLabel(const std::string& label) { mLabel = label; }
 void SetFont(Font* font) { delete mFont; mFont = font ? font->Duplicate() : nullptr; }
 ~ButtonWidget() { delete mFont; }
 bool IsButtonDown() const { return mIsDown && mIsOver && !mDisabled; }
 std::string AccessibilityLabel() override;
 void Update() override;
 void Draw(Graphics* g) override;
 void MouseDown(int x, int y, int count) override;
 void MouseUp(int x, int y, int count) override;
};
class DialogButton : public ButtonWidget {
public:
 Image* mComponentImage = nullptr;
 int mTextOffsetX=0, mTextOffsetY=0, mTranslateX=1, mTranslateY=1;
 DialogButton(Image* image, int id, ButtonListener* listener) { mComponentImage = image; mId = id; mButtonListener = listener; mHeight = image ? image->mHeight : 24; }
 void Draw(Graphics* g) override;
};
class Dialog : public Widget, public ButtonListener {
public:
 enum { BUTTONS_NONE, BUTTONS_YES_NO, BUTTONS_OK_CANCEL, BUTTONS_FOOTER };
 enum { ID_YES=1000, ID_NO=1001 };
 enum { COLOR_HEADER, COLOR_LINES, COLOR_FOOTER, COLOR_BUTTON_TEXT, COLOR_BUTTON_TEXT_HILITE };
 int mId, mButtonMode, mButtonHeight, mTextAlign=0, mSpaceAfterHeader=10, mLineSpacingOffset=0;
 bool mIsModal;
 Image* mComponentImage;
 Font* mHeaderFont=nullptr;
 Font* mLinesFont=nullptr;
 DialogButton* mYesButton=nullptr;
 DialogButton* mNoButton=nullptr;
 Insets mContentInsets{24,24,24,24}, mBackgroundInsets;
 std::string mDialogHeader, mDialogLines, mDialogFooter;
 Dialog(Image* image, Image* button, int id, bool modal, const std::string& header, const std::string& lines, const std::string& footer, int mode);
 ~Dialog();
 void SetHeaderFont(Font* font) { delete mHeaderFont; mHeaderFont=font->Duplicate(); }
 void SetLinesFont(Font* font) { delete mLinesFont; mLinesFont=font->Duplicate(); }
 virtual int GetPreferredHeight(int width);
 void Resize(int x,int y,int width,int height) override;
 void Draw(Graphics* g) override;
 void AddedToManager(WidgetManager* manager) override;
 void RemovedFromManager(WidgetManager* manager) override;
 void OrderInManagerChanged() override;
 void ButtonDepress(int id) override;
};
class HyperlinkWidget : public ButtonWidget {
public:
 int mUnderlineSize=0;
 Color mColor, mOverColor;
 HyperlinkWidget(int id, ButtonListener* listener) { mId=id; mButtonListener=listener; }
 void Draw(Graphics* g) override;
};
class Checkbox : public Widget {
public:
 bool mChecked=false;
 bool IsChecked() const { return mChecked; }
 Image* mUncheckedImage;
 Image* mCheckedImage;
 Rect mCheckedRect, mUncheckedRect;
 int mId;
 CheckboxListener* mCheckboxListener;
 Checkbox(Image* unchecked, Image* checked, int id, CheckboxListener* listener) : mUncheckedImage(unchecked), mCheckedImage(checked), mId(id), mCheckboxListener(listener) {}
 void Draw(Graphics* g) override;
 void MouseDown(int x, int y, int button, int count) override;
};
class Slider : public Widget {
public:
 double mVal=0;
 int mId;
 Image* mTrackImage;
 Image* mThumbImage;
 SliderListener* mSliderListener;
 Slider(Image* track, Image* thumb, int id, SliderListener* listener) : mId(id), mTrackImage(track), mThumbImage(thumb), mSliderListener(listener) {}
 void SetValue(double value) { mVal=max(0.0,min(1.0,value)); }
 void Draw(Graphics* g) override;
 void MouseDown(int x,int y,int count) override;
 void MouseDrag(int x,int y) override { MouseDown(x,y,1); }
};
class ScrollbarWidget;
class ListWidget : public Widget {
public:
 enum { JUSTIFY_LEFT, JUSTIFY_CENTER, JUSTIFY_RIGHT };
 std::vector<std::string> mLines;
 int mSelectIdx=-1, mHiliteIdx=-1, mItemHeight=15, mJustify=JUSTIFY_LEFT, mPosition=0, mId;
 bool mDrawOutline=true;
 Font* mFont;
 ListListener* mListListener;
 ScrollbarWidget* mScrollbar=nullptr;
 ListWidget(int id,Font* font,ListListener* listener) : mId(id),mFont(font),mListListener(listener) {}
 int AddLine(const std::string& line,bool alphabetical) { mLines.push_back(line);return mLines.size()-1; }
 void SetSelect(int index) { mSelectIdx=index; }
 int GetLineCount() const { return mLines.size(); }
 std::string GetStringAt(int index) { return mLines.at(index); }
 void RemoveLine(int index) { if(index>=0 && index<GetLineCount())mLines.erase(mLines.begin()+index); }
 void SetLine(int index,const std::string& value) { mLines.at(index)=value; }
 void Draw(Graphics* g) override;
 void MouseMove(int x,int y) override;
 void MouseLeave() override { mHiliteIdx=-1; }
 void MouseDown(int x,int y,int count) override;
};
class ScrollbarWidget : public Widget {
public:
 ListWidget* mList;
 ScrollbarWidget(int id,ListWidget* list) : mList(list) {}
 void ResizeScrollbar(int x,int y,int width,int height) { Resize(x,y,width,height); }
 void Draw(Graphics* g) override;
 void MouseDown(int x,int y,int count) override;
 void MouseDrag(int x,int y) override { MouseDown(x,y,1); }
};
class EditWidget : public Widget {
public:
 enum { COLOR_BKG, COLOR_OUTLINE, COLOR_TEXT, COLOR_HILITE, COLOR_HILITE_TEXT };
 std::string mString;
 int mId, mMaxChars=-1, mMaxPixels=-1, mCursorPos=0, mHilitePos=-1;
 Font* mFont=nullptr;
 EditListener* mEditListener;
 EditWidget(int id,EditListener* listener) : mId(id),mEditListener(listener) {}
 ~EditWidget() { delete mFont; }
 void SetFont(Font* font) { delete mFont;mFont=font->Duplicate(); }
 void SetText(const std::string& text) { mString=text;mCursorPos=text.size();mHilitePos=0; }
 void Draw(Graphics* g) override;
 void KeyDown(KeyCode key) override;
 void KeyChar(char value) override;
 void MouseDown(int x,int y,int count) override;
};
class WidgetManager {
public:
 int mLastMouseX = 327, mLastMouseY = 400;
 bool mMouseIn=true;
 SexyAppBase* mApp=nullptr;
 Widget* mFocus=nullptr;
 Widget* mOver=nullptr;
 Widget* mDown=nullptr;
 Widget* mAdding=nullptr;
 std::vector<Widget*> mWidgets;
 void AddWidget(Widget* widget);
 void RemoveWidget(Widget* widget);
 void SetFocus(Widget* widget) { mFocus=widget; }
 void PutInfront(Widget* widget, Widget* reference);
 void PutBehind(Widget* widget, Widget* reference);
 void BringToBack(Widget* widget);
 void MarkAllDirty() {}
 void UpdateAll();
 void DrawAll(Graphics* g);
 void Pointer(int x,int y);
 void Press(int x,int y,int button);
 void Release(int x,int y,int button);
 Widget* Hit(int x,int y);
 Widget* InputRoot();
 bool IsKeyDown(KeyCode key) { return false; }
};
class ResourceManager { public: void ReplaceImage(const std::string& name, Image* image) {} void DeleteExtraImageBuffers(const std::string& name) {} };
class SoundInstance {
public:
 int mId, mAsset;
 double mVolume = 1, mPitch = 0;
 int mPan = 0;
 bool mPlaying = false;
 SoundInstance(int id, int asset) : mId(id), mAsset(asset) {}
 void SetVolume(double volume);
 void AdjustPitch(double pitch) { mPitch += pitch; }
 void SetPan(int pan) { mPan = pan; }
 void Play(bool loop, bool autoRelease);
 void Stop();
 void Release() { Stop(); delete this; }
 bool IsPlaying() { return mPlaying; }
};
class SoundManager { public: int mNext = 1; SoundInstance* GetSoundInstance(int id) { return new SoundInstance(mNext++, id); } };
class GLInterface { public: void BltTransformed(Image* image, const Rect* clip, const Color& color, int mode, const Rect& source, const SexyMatrix3& matrix, bool blend); };
class DDInterface { public: GLInterface* mD3DInterface = nullptr; };
class SexyAppBase : public ButtonListener {
public:
 int mUpdateCount = 0;
 WidgetManager* mWidgetManager;
 SoundManager* mSoundManager;
 ResourceManager* mResourceManager;
 GLInterface* mGLInterface;
 DDInterface* mDDInterface;
 bool mShutdown = false;
 bool mIsWindowed = true, mCustomCursorsEnabled = false, mForceFullscreen = false;
 int mWidth=640, mHeight=480;
 double mMusicVolume=0.38, mSfxVolume=0.7;
 std::vector<Widget*> mSafeDeletes;
 std::string mRegistryKey;
 std::map<std::string, std::string> mStrings;
 std::map<int, Dialog*> mDialogMap;
 SexyAppBase();
 virtual ~SexyAppBase() {}
 virtual void Init() {}
 virtual void Shutdown() {}
 virtual void UpdateFrames() {}
 virtual void HandleEvent(SDL_Event* ev) {}
 virtual void DrawAboveWidgets(Graphics* g) {}
 virtual void GotFocus() {}
 virtual void LostFocus() {}
 virtual Dialog* NewDialog(int id, bool modal, const std::string& header, const std::string& lines, const std::string& footer, int mode) { return nullptr; }
 virtual bool KillDialog(int id) { return true; }
 Dialog* GetDialog(int id) { auto i = mDialogMap.find(id); return i == mDialogMap.end() ? nullptr : i->second; }
 void AddDialog(int id, Dialog* dialog);
 Dialog* DoDialog(int id,bool modal,const std::string& header,const std::string& lines,const std::string& footer,int mode);
 void SafeDeleteWidget(Widget* widget) { if (widget) mSafeDeletes.push_back(widget); }
 void FlushDeletes();
 void SetCursor(int cursor);
 void EnableCustomCursor(bool enabled);
 void EnableCustomCursors(bool enabled) { EnableCustomCursor(enabled); }
 void SwitchScreenMode(bool windowed, bool accelerated);
 double GetMusicVolume() const { return mMusicVolume; }
 double GetSfxVolume() const { return mSfxVolume; }
 void SetMusicVolume(double value);
 void SetSfxVolume(double value);
 bool Is3DAccelerationSupported() { return true; }
 bool Is3DAccelerationRecommended() { return true; }
 bool Is3DAccelerated() { return true; }
 void ClearUpdateBacklog() {}
 void DemoAddMarker(const std::string& marker) {}
 void PrecacheAlpha(Image* image) {}
 void PrecacheNative(Image* image) {}
 void PlaySample(int id, int pan = 0);
 MemoryImage* GetImage(const std::string& path, bool commit = true);
 std::string GetString(const std::string& key, const std::string& fallback) { auto i = mStrings.find(key); return i == mStrings.end() ? fallback : i->second; }
 void SetString(const std::string& key, const std::string& value) { mStrings[key] = value; }
 bool ReadBufferFromFile(const std::string& path, Buffer* buffer);
 bool WriteBytesToFile(const std::string& path, const void* bytes, int length);
 bool EraseFile(const std::string& path);
 void Popup(const std::string& text);
};
}
