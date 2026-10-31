#include "WebFramework.h"
#include "CircleCommon.h"

namespace Sexy {
void SetupEditWidget(EditWidget* widget) {
 widget->SetColor(0,Color(0x294918));widget->SetColor(1,Color(0));widget->SetColor(2,Color(0xCEE321));widget->SetColor(3,Color(0xAD28C6));widget->SetColor(4,Color(0xffffff));
}
void ListWidget::Draw(Graphics* g) {
 g->SetColor(mColors[0]);g->FillRect(0,0,mWidth,mHeight);
 Graphics text(*g),selection(*g);text.ClipRect(4,4,mWidth-8,mHeight-8);selection.ClipRect(0,4,mWidth,mHeight-8);text.SetFont(mFont);
 int last=min(GetLineCount()-1,mPosition+(mHeight-8)/mItemHeight+1);
 for(int i=mPosition;i<=last;++i) {
  int y=4+(i-mPosition)*mItemHeight;
  if(i==mSelectIdx) {selection.SetColor(mColors[4]);selection.FillRect(0,y,mWidth,mItemHeight);}
  text.SetColor(mColors[i==mHiliteIdx?3:i==mSelectIdx?5:2]);
  int x=mJustify==JUSTIFY_CENTER?(mWidth-mFont->StringWidth(mLines[i]))/2:mJustify==JUSTIFY_RIGHT?mWidth-mFont->StringWidth(mLines[i])-4:4;
  text.DrawString(mLines[i],x,y+mFont->GetAscent()+(mItemHeight-mFont->GetHeight())/2);
 }
 if(mDrawOutline){g->SetColor(mColors[1]);g->DrawRect(0,0,mWidth-1,mHeight-1);}
}
void ListWidget::MouseMove(int x,int y) {int index=(y-4)/mItemHeight+mPosition;mHiliteIdx=y>=4 && index<GetLineCount()?index:-1;}
void ListWidget::MouseDown(int x,int y,int count) {if(count<0)return;MouseMove(x,y);if(mHiliteIdx>=0 && mListListener)mListListener->ListClicked(mId,mHiliteIdx,count);}
void ScrollbarWidget::Draw(Graphics* g) {
 g->SetColor(Color(41,73,24));g->FillRect(0,0,mWidth,mHeight);g->SetColor(Color::Black);g->DrawRect(0,0,mWidth-1,mHeight-1);
 int height=max(20,(mHeight-4)*max(1,(mList->mHeight-8)/mList->mItemHeight)/max(1,mList->GetLineCount()));
 int limit=max(1,mList->GetLineCount()-(mList->mHeight-8)/mList->mItemHeight);
 g->SetColor(Color(203,201,187));g->FillRect(2,2+(mHeight-height-4)*mList->mPosition/limit,mWidth-4,height);
}
void ScrollbarWidget::MouseDown(int x,int y,int count) {if(count<0)return;int page=max(1,(mList->mHeight-8)/mList->mItemHeight);mList->mPosition=max(0,min(mList->GetLineCount()-page,y*max(0,mList->GetLineCount()-page)/max(1,mHeight)));}
void EditWidget::Draw(Graphics* g) {
 g->SetColor(mColors[COLOR_BKG]);g->FillRect(0,0,mWidth,mHeight);
 Graphics text(*g);text.ClipRect(4,0,mWidth-8,mHeight);text.SetFont(mFont);
 int y=(mHeight-mFont->GetHeight())/2+mFont->GetAscent();
 bool focus=mWidgetManager && mWidgetManager->mFocus==this;
 int start=min(mCursorPos,mHilitePos),end=max(mCursorPos,mHilitePos);
 if(focus && mHilitePos>=0 && start!=end) {
  int x=mFont->StringWidth(mString.substr(0,start));int width=mFont->StringWidth(mString.substr(0,end))-x;
  text.SetColor(mColors[COLOR_HILITE]);text.FillRect(4+x,(mHeight-mFont->GetHeight())/2,width,mFont->GetHeight());
 }
 text.SetColor(mColors[COLOR_TEXT]);text.DrawString(mString,4,y);
 if(focus && mHilitePos>=0 && start!=end) {
  Graphics highlight(text);int x=mFont->StringWidth(mString.substr(0,start));highlight.ClipRect(4+x,0,mFont->StringWidth(mString.substr(0,end))-x,mHeight);highlight.SetColor(mColors[COLOR_HILITE_TEXT]);highlight.DrawString(mString,4,y);
 } else if(focus && (mUpdateCnt/40)%2==0) {
  text.SetColor(mColors[COLOR_HILITE]);text.FillRect(4+mFont->StringWidth(mString.substr(0,mCursorPos)),(mHeight-mFont->GetHeight())/2,2,mFont->GetHeight());
 }
 g->SetColor(mColors[COLOR_OUTLINE]);g->DrawRect(0,0,mWidth-1,mHeight-1);
}
void EditWidget::MouseDown(int x,int y,int count) {
 if(count<0)return;mWidgetManager->SetFocus(this);mHilitePos=-1;mCursorPos=0;
 while(mCursorPos<int(mString.size()) && mFont->StringWidth(mString.substr(0,mCursorPos+1))<x-4)++mCursorPos;
}
static bool eraseSelection(EditWidget* edit) {
 if(edit->mHilitePos<0 || edit->mHilitePos==edit->mCursorPos)return false;
 int start=min(edit->mHilitePos,edit->mCursorPos),end=max(edit->mHilitePos,edit->mCursorPos);edit->mString.erase(start,end-start);edit->mCursorPos=start;edit->mHilitePos=-1;return true;
}
void EditWidget::KeyDown(KeyCode key) {
 if(key==KEYCODE_RETURN){mEditListener->EditWidgetText(mId,mString);return;}
 if(key==KEYCODE_BACK){if(!eraseSelection(this) && mCursorPos>0)mString.erase(--mCursorPos,1);return;}
 if(int(key)==46){if(!eraseSelection(this) && mCursorPos<int(mString.size()))mString.erase(mCursorPos,1);return;}
 if(key==KEYCODE_LEFT)mCursorPos=max(0,mCursorPos-1);else if(key==KEYCODE_RIGHT)mCursorPos=min(int(mString.size()),mCursorPos+1);else if(int(key)==36)mCursorPos=0;else if(int(key)==35)mCursorPos=mString.size();else return;mHilitePos=-1;
}
void EditWidget::KeyChar(char value) {
 if(static_cast<unsigned char>(value)<32 || !mEditListener->AllowChar(mId,value))return;
 auto previous=mString;int cursor=mCursorPos,highlight=mHilitePos;eraseSelection(this);mString.insert(mCursorPos,1,value);
 if((mMaxChars>=0 && int(mString.size())>mMaxChars) || (mMaxPixels>=0 && mFont->StringWidth(mString)>mMaxPixels)){mString=previous;mCursorPos=cursor;mHilitePos=highlight;return;}
 ++mCursorPos;mHilitePos=-1;
}
}
