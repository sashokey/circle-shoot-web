#include "WebFramework.h"
#include "CircleCommon.h"
#include "CircleButton.h"
#include "CircleCheckbox.h"
#include "Res.h"

extern "C" __attribute__((import_module("web"), import_name("control"))) void host_control(int operation, double value);

namespace Sexy {
void Widget::Layout(int flags, Widget* relative, int left, int top, int width, int height) {
 int x=mX, y=mY, w=mWidth, h=mHeight;
 if (flags & LAY_SameWidth) w=relative->mWidth+width;
 if (flags & LAY_SameHeight) h=relative->mHeight+height;
 if (flags & LAY_SetLeft) x=left;
 if (flags & LAY_SetTop) y=top;
 if (flags & LAY_SetWidth) w=width;
 if (flags & LAY_SetHeight) h=height;
 if (flags & LAY_Above) y=relative->mY-h+top;
 if (flags & LAY_Below) y=relative->Bottom()+top;
 if (flags & LAY_Right) x=relative->Right()+left;
 if (flags & LAY_Left) x=relative->mX-w+left;
 if (flags & LAY_SameLeft) x=relative->mX+left;
 if (flags & LAY_SameRight) x=relative->Right()-w+left;
 if (flags & LAY_SameTop) y=relative->mY+top;
 if (flags & LAY_SameBottom) y=relative->Bottom()-h+top;
 if (flags & LAY_GrowToRight) w=relative->Right()-x+width;
 if (flags & LAY_GrowToLeft) w=relative->mX-x+width;
 if (flags & LAY_GrowToTop) h=relative->mY-y+height;
 if (flags & LAY_GrowToBottom) h=relative->Bottom()-y+height;
 if (flags & LAY_HCenter) x=relative->mX+(relative->mWidth-w)/2+left;
 if (flags & LAY_VCenter) y=relative->mY+(relative->mHeight-h)/2+top;
 Resize(x,y,w,h);
}

static std::vector<std::string> wrap(Font* font, int width, const std::string& value) {
 std::vector<std::string> lines;
 size_t start=0;
 while (start < value.size()) {
  size_t end=start, space=std::string::npos;
  for (; end<value.size() && value[end]!='\n'; ++end) {
   if (value[end]==' ') space=end;
   if (font->StringWidth(value.substr(start,end-start+1))>width && end>start) { if (space!=std::string::npos && space>start) end=space; break; }
  }
  lines.push_back(value.substr(start,end-start));
  start=end;
  if (start<value.size() && value[start]=='\n') ++start;
  else while (start<value.size() && value[start]==' ') ++start;
 }
 return lines;
}
int Widget::WriteWordWrapped(Graphics* g, const Rect& rect, const std::string& value, int lineSpacing, int align) {
 if (value.empty() || !g->mFont) return 0;
 if (lineSpacing<0) lineSpacing=g->mFont->GetLineSpacing();
 auto lines=wrap(g->mFont,rect.mWidth,value);
 int y=rect.mY+g->mFont->GetAscent()-g->mFont->GetAscentPadding();
 for (auto& line : lines) {
  int width=g->StringWidth(line), x=rect.mX+(align==0 ? (rect.mWidth-width)/2 : align>0 ? rect.mWidth-width : 0);
  g->DrawString(line,x,y); y+=lineSpacing;
 }
 return lines.empty() ? 0 : (lines.size()-1)*lineSpacing+g->mFont->GetHeight()-g->mFont->GetAscentPadding();
}
int Widget::GetWordWrappedHeight(Graphics* g, int width, const std::string& value, int lineSpacing) {
 if (value.empty() || !g->mFont) return 0;
 if (lineSpacing<0) lineSpacing=g->mFont->GetLineSpacing();
 auto lines=wrap(g->mFont,width,value);
 return lines.empty() ? 0 : (lines.size()-1)*lineSpacing+g->mFont->GetHeight()-g->mFont->GetAscentPadding();
}

void Graphics::DrawImageBox(const Rect& destination, Image* image) { if(image) DrawImageBox(Rect(0,0,image->mWidth,image->mHeight),destination,image); }
void Graphics::DrawImageBox(const Rect& source, const Rect& destination, Image* image) {
 if (!image || source.mWidth<3 || source.mHeight<3) return;
 int widths[3]={source.mWidth/3,source.mWidth-source.mWidth/3*2,source.mWidth/3};
 int heights[3]={source.mHeight/3,source.mHeight-source.mHeight/3*2,source.mHeight/3};
 int destWidths[3]={widths[0],destination.mWidth-widths[0]-widths[2],widths[2]};
 int destHeights[3]={heights[0],destination.mHeight-heights[0]-heights[2],heights[2]};
 int sy=source.mY, dy=destination.mY;
 for (int row=0; row<3; ++row) {
  int sx=source.mX, dx=destination.mX;
  for (int col=0; col<3; ++col) {
   for (int y=0;y<destHeights[row];y+=heights[row]) for (int x=0;x<destWidths[col];x+=widths[col]) {
    int w=min(widths[col],destWidths[col]-x), h=min(heights[row],destHeights[row]-y);
    DrawImage(image,dx+x,dy+y,Rect(sx,sy,w,h));
   }
   sx+=widths[col]; dx+=destWidths[col];
  }
  sy+=heights[row]; dy+=destHeights[row];
 }
}

void ButtonWidget::MouseDown(int x,int y,int count) { if(count<0) return; mIsDown=true; if(mButtonListener) mButtonListener->ButtonPress(mId); }
std::string ButtonWidget::AccessibilityLabel() {
 if(!mLabel.empty())return mLabel;
 if(mButtonImage==IMAGE_MM_ARCADE)return "Adventure";
 if(mButtonImage==IMAGE_MM_GAUNTLET)return "Gauntlet";
 if(mButtonImage==IMAGE_MM_OPTIONS)return "Options";
 if(mButtonImage==IMAGE_MM_MOREGAMES)return "More Games";
 if(mButtonImage==IMAGE_MM_QUIT)return "Quit";
 if(mButtonImage==IMAGE_ADVMAINMENUBUTTON || mButtonImage==IMAGE_GAUNTMAINMENUBUTTON)return "Main Menu";
 if(mButtonImage==IMAGE_ADVPLAYBUTTON || mButtonImage==IMAGE_GAUNTPLAYBUTTON)return "Play";
 if(mButtonImage==IMAGE_GAUNTBACKBUTTON)return "Back";
 if(mButtonImage==IMAGE_GAUNTNEXTBUTTON)return "Next";
 if(mButtonImage==IMAGE_GAUNTPRACTICEBUTTON)return "Practice";
 if(mButtonImage==IMAGE_GAUNTSURVIVALBUTTON)return "Survival";
 if(mButtonImage==IMAGE_MENU_BUTTON)return "Menu";
 return "";
}
void ButtonWidget::MouseUp(int x,int y,int count) { bool pressed=mIsDown && mIsOver && !mDisabled; mIsDown=false; if(pressed && mButtonListener) mButtonListener->ButtonDepress(mId); }
void ButtonWidget::Update() {
 Widget::Update();
 if(mIsDown && mIsOver && mButtonListener) mButtonListener->ButtonDownTick(mId);
 if(!mIsDown && !mIsOver && mOverAlpha>0) mOverAlpha=mOverAlphaSpeed>0 ? max(0.0,mOverAlpha-mOverAlphaSpeed) : 0;
 else if(mIsOver && mOverAlphaFadeInSpeed>0) mOverAlpha=min(1.0,mOverAlpha+mOverAlphaFadeInSpeed);
}
void ButtonWidget::Draw(Graphics* g) {
 if(mBtnNoDraw) return;
 bool down=IsButtonDown() ^ mInverted;
 if(mButtonImage) {
  Rect rect=mNormalRect;
  if(down && mDownRect.mWidth) rect=mDownRect;
  else if(mDisabled && mDisabledRect.mWidth) rect=mDisabledRect;
  else if((mIsOver || mIsDown) && mOverRect.mWidth) rect=mOverRect;
  if(rect.mWidth) g->DrawImage(mButtonImage,0,0,rect); else g->DrawImage(mButtonImage,0,0);
 }
 if(mFont && !mLabel.empty()) {
  g->SetFont(mFont); g->SetColor(mColors[mIsOver?COLOR_LABEL_HILITE:COLOR_LABEL]);
  g->DrawString(mLabel,(mWidth-mFont->StringWidth(mLabel))/2+(down?1:0),(mHeight+mFont->GetAscent()-mFont->GetAscent()/6-1)/2+(down?1:0));
 }
}
void DialogButton::Draw(Graphics* g) {
 if(mBtnNoDraw) return;
 if(!mComponentImage) { ButtonWidget::Draw(g); return; }
 bool down=IsButtonDown();
 Rect rect=mNormalRect;
 if(mDisabled && mDisabledRect.mWidth) rect=mDisabledRect;
 else if(down) rect=mDownRect;
 else if(mIsOver || mOverAlpha>0) rect=mOverRect;
 if(!rect.mWidth) rect=Rect(0,0,mComponentImage->mWidth,mComponentImage->mHeight);
 g->DrawImageBox(rect,Rect(0,0,mWidth,mHeight),mComponentImage);
 if(mFont && !mLabel.empty()) {
  g->SetFont(mFont); g->SetColor(mColors[mIsOver?COLOR_LABEL_HILITE:COLOR_LABEL]);
  int x=(mWidth-mFont->StringWidth(mLabel))/2+mTextOffsetX+(down?mTranslateX:0);
  int y=(mHeight+mFont->GetAscent()-mFont->GetAscentPadding()-mFont->GetAscent()/6-1)/2+mTextOffsetY+(down?mTranslateY:0);
  g->DrawString(mLabel,x,y);
 }
}

Dialog::Dialog(Image* image, Image* button, int id, bool modal, const std::string& header, const std::string& lines, const std::string& footer, int mode) : mId(id),mButtonMode(mode),mButtonHeight(button?button->mHeight:24),mIsModal(modal),mComponentImage(image),mDialogHeader(header),mDialogLines(lines),mDialogFooter(footer) {
 if(mode!=BUTTONS_NONE) { mYesButton=new DialogButton(button,2000+id,this); mYesButton->mLabel=mode==BUTTONS_FOOTER?footer:mode==BUTTONS_OK_CANCEL?"OK":"YES"; }
 if(mode==BUTTONS_YES_NO || mode==BUTTONS_OK_CANCEL) { mNoButton=new DialogButton(button,3000+id,this); mNoButton->mLabel=mode==BUTTONS_OK_CANCEL?"CANCEL":"NO"; }
}
Dialog::~Dialog() { delete mYesButton; delete mNoButton; delete mHeaderFont; delete mLinesFont; }
int Dialog::GetPreferredHeight(int width) {
 int height=mContentInsets.mTop+mContentInsets.mBottom+mBackgroundInsets.mTop+mBackgroundInsets.mBottom;
 bool space=false;
 if(!mDialogHeader.empty()) { height+=mHeaderFont->GetHeight()-mHeaderFont->GetAscentPadding(); space=true; }
 if(!mDialogLines.empty()) { if(space)height+=mSpaceAfterHeader; Graphics g;g.SetFont(mLinesFont);height+=GetWordWrappedHeight(&g,width-mContentInsets.mLeft-mContentInsets.mRight-mBackgroundInsets.mLeft-mBackgroundInsets.mRight-4,mDialogLines,mLinesFont->GetLineSpacing()+mLineSpacingOffset);space=true; }
 if(!mDialogFooter.empty() && mButtonMode!=BUTTONS_FOOTER) { if(space)height+=8; height+=mHeaderFont->GetLineSpacing();space=true; }
 if(mYesButton) { if(space)height+=8; height+=mButtonHeight+8; }
 return height;
}
void Dialog::Resize(int x,int y,int width,int height) {
 Widget::Resize(x,y,width,height);
 int left=x+mContentInsets.mLeft+mBackgroundInsets.mLeft;
 int top=y+height-mContentInsets.mBottom-mBackgroundInsets.mBottom-mButtonHeight;
 int buttonWidth=width-mContentInsets.mLeft-mContentInsets.mRight-mBackgroundInsets.mLeft-mBackgroundInsets.mRight;
 if(mNoButton) buttonWidth=(buttonWidth-8)/2;
 if(mYesButton) mYesButton->Resize(left,top,buttonWidth,mButtonHeight);
 if(mNoButton) mNoButton->Resize(left+buttonWidth+8,top,buttonWidth,mButtonHeight);
}
void Dialog::Draw(Graphics* g) {
 g->DrawImageBox(Rect(mBackgroundInsets.mLeft,mBackgroundInsets.mTop,mWidth-mBackgroundInsets.mLeft-mBackgroundInsets.mRight,mHeight-mBackgroundInsets.mTop-mBackgroundInsets.mBottom),mComponentImage);
 int y=mContentInsets.mTop+mBackgroundInsets.mTop;
 if(mHeaderFont && !mDialogHeader.empty()) {
  y+=mHeaderFont->GetAscent()-mHeaderFont->GetAscentPadding();g->SetFont(mHeaderFont);g->SetColor(mColors[COLOR_HEADER]);WriteCenteredLine(g,y,mDialogHeader);
  y+=mHeaderFont->GetHeight()-mHeaderFont->GetAscent()+mSpaceAfterHeader;
 }
 if(mLinesFont) {
  g->SetFont(mLinesFont);g->SetColor(mColors[COLOR_LINES]);
  y+=WriteWordWrapped(g,Rect(mBackgroundInsets.mLeft+mContentInsets.mLeft+2,y,mWidth-mContentInsets.mLeft-mContentInsets.mRight-mBackgroundInsets.mLeft-mBackgroundInsets.mRight-4,0),mDialogLines,mLinesFont->GetLineSpacing()+mLineSpacingOffset,mTextAlign);
 }
 if(mHeaderFont && !mDialogFooter.empty() && mButtonMode!=BUTTONS_FOOTER) { y+=8+mHeaderFont->GetLineSpacing();g->SetFont(mHeaderFont);g->SetColor(mColors[COLOR_FOOTER]);WriteCenteredLine(g,y,mDialogFooter); }
}
void Dialog::AddedToManager(WidgetManager* manager) { Widget::AddedToManager(manager);manager->AddWidget(mYesButton);manager->AddWidget(mNoButton); }
void Dialog::RemovedFromManager(WidgetManager* manager) { Widget::RemovedFromManager(manager);manager->RemoveWidget(mYesButton);manager->RemoveWidget(mNoButton); }
void Dialog::OrderInManagerChanged() { if(mWidgetManager) {mWidgetManager->PutInfront(mYesButton,this);mWidgetManager->PutInfront(mNoButton,this);} }
void Dialog::ButtonDepress(int id) { if(id==ID_YES)id=mId+2000;else if(id==ID_NO)id=mId+3000;if(id==mId+2000 || id==mId+3000) gSexyAppBase->ButtonDepress(id); }
void HyperlinkWidget::Draw(Graphics* g) { if(!mFont) return;g->SetFont(mFont);g->SetColor(mIsOver?mOverColor:mColor);g->DrawString(mLabel,(mWidth-mFont->StringWidth(mLabel))/2,(mHeight+mFont->GetAscent())/2-1); }
void Checkbox::Draw(Graphics* g) { g->DrawImage(mChecked?mCheckedImage:mUncheckedImage,0,0,mChecked?mCheckedRect:mUncheckedRect); }
void Checkbox::MouseDown(int x,int y,int button,int count) { if(button<0) return;mChecked=!mChecked;if(mCheckboxListener)mCheckboxListener->CheckboxChecked(mId,mChecked); }
void Slider::Draw(Graphics* g) {
 int thumb=mThumbImage->mWidth;
 g->DrawImageBox(Rect(0,0,mTrackImage->mWidth,mTrackImage->mHeight),Rect(0,(mHeight-mTrackImage->mHeight)/2,mWidth,mTrackImage->mHeight),mTrackImage);
 g->DrawImage(mThumbImage,int((mWidth-thumb)*mVal),(mHeight-mThumbImage->mHeight)/2);
}
void Slider::MouseDown(int x,int y,int count) { if(count<0) return;SetValue(double(x-mThumbImage->mWidth/2)/max(1,mWidth-mThumbImage->mWidth));if(mSliderListener)mSliderListener->SliderVal(mId,mVal); }

static bool visible(Widget* widget) { for(auto* w=widget;w;w=w->mOwner) if(!w->mVisible) return false;return true; }
static Widget* owner(Widget* widget) { while(widget && widget->mOwner) widget=widget->mOwner;return widget; }
Widget* WidgetManager::InputRoot() {
 Widget* root=owner(mFocus);int focusIndex=-1;
 for(int i=0;i<int(mWidgets.size());++i)if(mWidgets[i]==root)focusIndex=i;
 for(int i=focusIndex+1;i<int(mWidgets.size());++i)for(auto& dialog:mApp->mDialogMap)if(dialog.second==mWidgets[i] && dialog.second->mIsModal && visible(dialog.second))root=dialog.second;
 return root;
}
void WidgetManager::AddWidget(Widget* widget) {
 if(!widget || std::find(mWidgets.begin(),mWidgets.end(),widget)!=mWidgets.end()) return;
 widget->mOwner=mAdding;mWidgets.push_back(widget);auto* previous=mAdding;mAdding=widget;widget->AddedToManager(this);mAdding=previous;
}
void WidgetManager::RemoveWidget(Widget* widget) {
 if(!widget || std::find(mWidgets.begin(),mWidgets.end(),widget)==mWidgets.end()) return;
 if(mFocus==widget)mFocus=nullptr;if(mOver==widget)mOver=nullptr;if(mDown==widget)mDown=nullptr;
 mWidgets.erase(std::remove(mWidgets.begin(),mWidgets.end(),widget),mWidgets.end());widget->RemovedFromManager(this);widget->mOwner=nullptr;
}
void WidgetManager::PutInfront(Widget* widget, Widget* reference) {
 if(!widget || !reference || widget==reference) return;
 auto found=std::find(mWidgets.begin(),mWidgets.end(),widget);if(found==mWidgets.end()) return;mWidgets.erase(found);
 auto position=std::find(mWidgets.begin(),mWidgets.end(),reference);mWidgets.insert(position==mWidgets.end()?mWidgets.end():position+1,widget);
}
void WidgetManager::PutBehind(Widget* widget, Widget* reference) {
 if(!widget || !reference || widget==reference) return;
 auto found=std::find(mWidgets.begin(),mWidgets.end(),widget);if(found==mWidgets.end()) return;mWidgets.erase(found);
 auto position=std::find(mWidgets.begin(),mWidgets.end(),reference);mWidgets.insert(position,widget);
}
void WidgetManager::BringToBack(Widget* widget) { if(!widget)return;auto found=std::find(mWidgets.begin(),mWidgets.end(),widget);if(found==mWidgets.end())return;mWidgets.erase(found);mWidgets.insert(mWidgets.begin(),widget);widget->OrderInManagerChanged(); }
void WidgetManager::UpdateAll() { auto list=mWidgets;for(auto* widget:list) if(std::find(mWidgets.begin(),mWidgets.end(),widget)!=mWidgets.end() && visible(widget))widget->Update(); }
void WidgetManager::DrawAll(Graphics* g) {
 for(auto* widget:mWidgets) if(visible(widget)) {
  bool modal=false;for(auto& entry:mApp->mDialogMap) if(entry.second==widget && entry.second->mIsModal)modal=true;
  if(modal){Graphics shade(*g);shade.SetColor(Color(0,0,0,96));shade.FillRect(0,0,640,480);}
  Graphics local(*g);local.Translate(widget->mX,widget->mY);widget->Draw(&local);
 }
}
Widget* WidgetManager::Hit(int x,int y) {
 Widget* root=InputRoot();
 for(auto i=mWidgets.rbegin();i!=mWidgets.rend();++i) {auto* widget=*i;if(visible(widget) && !widget->mDisabled && widget->mMouseVisible && (!root || owner(widget)==root) && Rect(widget->mX,widget->mY,widget->mWidth,widget->mHeight).Contains(x,y) && widget->IsPointVisible(x-widget->mX,y-widget->mY)) return widget;}
 return nullptr;
}
void WidgetManager::Pointer(int x,int y) {
 mLastMouseX=x;mLastMouseY=y;auto* hit=Hit(x,y);
 if(hit!=mOver) {if(mOver){mOver->mIsOver=false;mOver->MouseLeave();}mOver=hit;if(mOver){mOver->mIsOver=true;mOver->MouseEnter();}}
 if(mDown)mDown->MouseDrag(x-mDown->mX,y-mDown->mY);else if(hit)hit->MouseMove(x-hit->mX,y-hit->mY);
}
void WidgetManager::Press(int x,int y,int button) { Pointer(x,y);auto* hit=Hit(x,y);mDown=hit;if(hit)hit->MouseDown(x-hit->mX,y-hit->mY,button==2?-1:1,1); }
void WidgetManager::Release(int x,int y,int button) { Pointer(x,y);auto* hit=mDown;mDown=nullptr;if(hit)hit->MouseUp(x-hit->mX,y-hit->mY,button==2?-1:1,1); }

void SexyAppBase::AddDialog(int id,Dialog* dialog) { mDialogMap[id]=dialog;mWidgetManager->SetFocus(dialog);mWidgetManager->AddWidget(dialog); }
Dialog* SexyAppBase::DoDialog(int id,bool modal,const std::string& header,const std::string& lines,const std::string& footer,int mode) { KillDialog(id);auto* dialog=NewDialog(id,modal,header,lines,footer,mode);AddDialog(id,dialog);return dialog; }
void SexyAppBase::FlushDeletes() { auto list=mSafeDeletes;mSafeDeletes.clear();for(auto* widget:list) delete widget; }
void SexyAppBase::SetCursor(int cursor) { host_control(0,cursor); }
void SexyAppBase::EnableCustomCursor(bool enabled) { mCustomCursorsEnabled=enabled;host_control(4,enabled); }
void SexyAppBase::SwitchScreenMode(bool windowed,bool accelerated) { mIsWindowed=windowed;host_control(3,!windowed); }
void SexyAppBase::SetMusicVolume(double value) { mMusicVolume=value;host_control(1,value); }
void SexyAppBase::SetSfxVolume(double value) { mSfxVolume=value;host_control(2,value); }
}
