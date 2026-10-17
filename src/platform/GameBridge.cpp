#include "WebFramework.h"
#include "CircleShootApp.h"
#include "Board.h"
#include "Ball.h"
#include "Bullet.h"
#include "Gun.h"
#include "CurveMgr.h"
#include "LevelParser.h"
#include "ProfileMgr.h"
#include "HighScoreMgr.h"
#include "DataSync.h"
#include "WorkerThread.h"
#include "SpriteMgr.h"
#include "TransitionMgr.h"
#include "StatsDialog.h"
#include "MainMenu.h"
#include "AdventureScreen.h"
#include "PracticeScreen.h"
#include "HelpScreen.h"
#include "CreditsScreen.h"
#include "WidgetMover.h"
#include "Res.h"
#include <sstream>

extern "C" __attribute__((import_module("web"), import_name("audio"))) void host_audio(int op, int id, int asset, double volume, double pitch, int loop, int pan);
extern "C" __attribute__((import_module("web"), import_name("message"))) void host_message(const char* message);
extern "C" void web_save();
static std::string screen = "menu";
static int pendingBonus = 0, appliedBonus = 0;
static int statsTicks = 0;
static std::string result, snapshot, ballData, levelData;
static bool ready = false;
static Sexy::CircleShootApp* app = nullptr;
void WebFinishScoreInc() { if (app) if (auto* dialog=static_cast<Sexy::StatsDialog*>(app->GetDialog(Sexy::DialogType_Stats))) dialog->FinishScoreInc(); }
static std::string escape(const std::string& value) { std::string out; for (char c : value) { if (c == '"' || c == '\\') out += '\\'; if (c == '\n') { out += "\\n"; continue; } out += c; } return out; }

namespace Sexy {
void WebInitResources();
CircleShootApp::CircleShootApp() {
 mWidgetMover = new WidgetMover(); mBoard = nullptr; mMainMenu = nullptr; mPracticeScreen = nullptr; mAdventureScreen = nullptr; mHelpScreen = nullptr; mMoreGamesScreen = nullptr; mLoadingScreen = nullptr; mCreditsScreen = nullptr;
 mWorkerThread = new WorkerThread(); mLevelParser = new LevelParser(); mProfileMgr = new ProfileMgr(); mHighScoreMgr = new HighScoreMgr(); mProfile = nullptr;
 mDidNextTempleDialog = false; mSongId = 24; mLastSong = 24; mLastSongSwitchTime = 0; mUnk28 = 0; mUnk29 = 0; mDoPlayCount = false; mPlayCount = 0; mMaxExecutions = 0; mMaxPlays = 0; mMaxTime = 0; mIsPractice = false; mUnk36 = 0;
 mController = nullptr; mControllerIndex = -1; mControllerWidget = nullptr;
}
CircleShootApp::~CircleShootApp() { delete mBoard; delete mLevelParser; delete mProfileMgr; delete mHighScoreMgr; delete mWorkerThread; }
void CircleShootApp::Init() {
 mProfileMgr->Load();
 auto* current=WebFileData("userdata/current-profile.txt");
 mProfile=current?mProfileMgr->GetProfile(std::string(current->begin(),current->end())):mProfileMgr->GetAnyProfile();
 if (!mProfile) mProfile = mProfileMgr->AddProfile("Player");
 mProfile->LoadDetails(); mProfile->mShowHelpScreen = false;
 mHighScoreMgr->Load();
 if (!mLevelParser->ParseLevelFile("levels/levels.xml")) { Popup(mLevelParser->GetErrorText()); return; }
 if (mLevelParser->mStageMap.size() != 13) { Popup("Incomplete campaign data"); return; }
 SetupLocalizations(); ready = true; mLastSong=-1; ShowMainMenu();
}
void CircleShootApp::Shutdown() { web_save(); mShutdown = true; }
void CircleShootApp::UpdateFrames() {
 ++mUpdateCount; mWidgetMover->Update(); mWidgetManager->UpdateAll(); FlushDeletes();
}
void CircleShootApp::ButtonDepress(int id) {
 if(id==2007 || id==3007) FinishUserDialog(id==2007);
 else if(id==2008 || id==3008) FinishCreateUserDialog(id==2008);
 else if(id==2009 || id==3009) FinishRenameUserDialog(id==2009);
 else if(id==2012 || id==3012) FinishConfirmDeleteUserDialog(id==2012);
 else if(id==2000) FinishOptionsDialog(true);
 else if(id==2013 || id==3013) FinishConfirmContinueDialog(id==2013);
 else if(id==2014 || id==3014) FinishStatsDialog(id==2014);
 else if(id==2015) FinishGetReadyDialog();
 else if(id==2016 || id==3016) FinishNextTempleDialog(id==2016);
 else if(id==2020 || id==3020) FinishConfirmMainMenuDialog(id==2020);
 else if(id==2021 || id==3021) FinishConfirmQuitDialog(id==2021);
 else if(id>=2000 && id<4000) KillDialog(id>=3000?id-3000:id-2000);
}
void CircleShootApp::HandleEvent(SDL_Event* ev) {}
void CircleShootApp::DrawAboveWidgets(Graphics* g) {}

bool CircleShootApp::KillDialog(int id) {
 auto i=mDialogMap.find(id);
 if(i!=mDialogMap.end()) { auto* dialog=i->second;mWidgetMover->SafeDeleteWidget(dialog);mDialogMap.erase(i); }
 Widget* focus=mHelpScreen?static_cast<Widget*>(mHelpScreen):mBoard?static_cast<Widget*>(mBoard):mAdventureScreen?static_cast<Widget*>(mAdventureScreen):mPracticeScreen?static_cast<Widget*>(mPracticeScreen):static_cast<Widget*>(mMainMenu);
 if(!mDialogMap.empty())focus=mDialogMap.rbegin()->second;
 mWidgetManager->SetFocus(focus);return true;
}
void CircleShootApp::GotFocus() {}
void CircleShootApp::LostFocus() { if (mBoard && screen == "game") { mBoard->Pause(true); screen = "pause"; } }

void CircleShootApp::CleanupWidgets() {
 while(!mDialogMap.empty())KillDialog(mDialogMap.begin()->first);
 if(mBoard) { if(mBoard->NeedSaveGame())mBoard->SaveGame(GetSaveGameName(mBoard->IsPracticeMode(),mProfile->mId));mWidgetMover->SafeDeleteWidget(mBoard);mBoard=nullptr; }
 if(mMainMenu){mWidgetMover->SafeDeleteWidget(mMainMenu);mMainMenu=nullptr;}
 if(mAdventureScreen){mWidgetMover->SafeDeleteWidget(mAdventureScreen);mAdventureScreen=nullptr;}
 if(mPracticeScreen){mWidgetMover->SafeDeleteWidget(mPracticeScreen);mPracticeScreen=nullptr;}
 if(mHelpScreen){mWidgetMover->SafeDeleteWidget(mHelpScreen);mHelpScreen=nullptr;}
 if(mCreditsScreen){mWidgetMover->SafeDeleteWidget(mCreditsScreen);mCreditsScreen=nullptr;}
}


void CircleShootApp::SaveProfile() { if (mProfile) {mProfile->SaveDetails();WriteBytesToFile("userdata/current-profile.txt",mProfile->mName.data(),mProfile->mName.size());} mProfileMgr->Save(); }
void CircleShootApp::SwitchSong(int id) { if (mLastSong == id || AppGetTickCount() - mLastSongSwitchTime >= 5000) PlaySong(id, true, 0.01); }
void CircleShootApp::PlaySong(int id, bool fade, double fadeSpeed) { if (mLastSong == id) return; mLastSong = id; mLastSongSwitchTime = AppGetTickCount(); host_audio(3, 0, fade ? 1 : 0, 1, id, 1, 0); }






void CircleShootApp::ShowMoreGamesScreen() {}
void CircleShootApp::DoCheckForUpdatesDialog() {}
void CircleShootApp::DoRegisterDialog() {}

}

extern "C" void web_save();
extern "C" void web_prepare() { if (!app) { app = new Sexy::CircleShootApp(); Sexy::WebInitResources(); } }
extern "C" int web_init(int seed) { Sexy::gAppRand.SRand(seed); Sexy::gThreadRand.SRand(seed); app->Init(); return ready ? 1 : 0; }
extern "C" void web_start(int stage, const char* practice, int difficulty, int endless) {
 if (!ready) return;
 app->CleanupWidgets();
 app->mIsPractice=practice && practice[0];
 app->EraseFile(Sexy::GetSaveGameName(practice && practice[0], app->mProfile->mId));
 if (practice && practice[0]) app->StartPracticeGame(practice, max(0, difficulty), endless != 0);
 else app->StartAdventureGame(max(0, min(12, stage)));
}
extern "C" int web_resume(int practice) {
 if (!ready) return 0;
 Sexy::Buffer buffer;
 auto path = Sexy::GetSaveGameName(practice != 0, app->mProfile->mId);
 if (!app->ReadBufferFromFile(path, &buffer) || buffer.GetDataLen() < 32) return 0;
 app->mIsPractice = practice != 0; screen = "game"; app->MakeBoard(); app->mBoard->SetStartLevel(0); app->mBoard->LoadGame(buffer);
 app->EraseFile(path); app->PlaySong(0, true, 0.01); return 1;
}
extern "C" void web_tick(int count) { for (int i = 0; i < min(20000, max(0, count)); ++i) app->UpdateFrames(); }
extern "C" void web_draw() { Sexy::Graphics g;app->mWidgetManager->DrawAll(&g); }
extern "C" void web_pointer(int x, int y) { app->mWidgetManager->Pointer(x,y); }
extern "C" void web_press(int x,int y,int button) { app->mWidgetManager->Press(x,y,button); }
extern "C" void web_release(int x,int y,int button) { app->mWidgetManager->Release(x,y,button); }
extern "C" void web_click(int x,int y,int button) { web_press(x,y,button);web_release(x,y,button); }
extern "C" void web_key(int key,int character) { auto* focus=app->mWidgetManager->mFocus;auto* root=focus;while(root && root->mOwner)root=root->mOwner;auto* active=app->mWidgetManager->InputRoot();if(root!=active)focus=active;if(key==27)for(auto& item:app->mDialogMap)if(item.second==active){item.second->ButtonDepress(item.second->mNoButton?item.first+3000:item.first+2000);return;}if(focus) {if(key)focus->KeyDown(static_cast<Sexy::KeyCode>(key));if(character)focus->KeyChar(character);} }
extern "C" void web_pause(int pause) { if(!app->mBoard || !app->mDialogMap.empty() || app->mHelpScreen)return; if(pause && app->mBoard->mPauseCount==0)app->mBoard->Pause(true);else if(!pause && app->mBoard->mPauseCount>0)app->mBoard->Pause(false); }
extern "C" void web_continue(int menu) { if(menu){web_save();app->ShowMainMenu();}else if(app->GetDialog(Sexy::DialogType_Stats)){WebFinishScoreInc();app->FinishStatsDialog(true);}else if(app->mBoard && app->mBoard->mPauseCount)web_pause(0);else if(app->mBoard && app->mBoard->mContinueButton->mVisible)app->mBoard->ButtonDepress(1); }
extern "C" void web_window(int fullscreen) { app->mIsWindowed=!fullscreen; }
extern "C" void web_save() { if (!app) return; app->SaveProfile(); app->mHighScoreMgr->Save(); if (app->mBoard && app->mBoard->NeedSaveGame()) app->mBoard->SaveGame(Sexy::GetSaveGameName(app->mIsPractice, app->mProfile->mId)); }
extern "C" const char* web_snapshot() {
 auto* b = app->mBoard;
 screen=app->mHelpScreen?"help":app->mCreditsScreen?"victory":app->mBoard?(app->mBoard->mPauseCount?"pause":"game"):app->mAdventureScreen?"adventure":app->mPracticeScreen?"gauntlet":app->mMainMenu?"menu":screen;
 auto* stats=static_cast<Sexy::StatsDialog*>(app->GetDialog(Sexy::DialogType_Stats));
 if(stats){screen="stats";pendingBonus=stats->mTargetTimeBonus;appliedBonus=stats->mTimeBonus;result=stats->mDialogHeader;}
 else if(app->GetDialog(Sexy::DialogType_Options) && !app->mHelpScreen)screen="options";
 std::ostringstream out;
 out << "{\"ready\":" << (ready ? "true" : "false") << ",\"screen\":\"" << screen << "\",\"result\":\"" << result << "\",\"bonus\":" << pendingBonus << ",\"bonusApplied\":" << appliedBonus << ",\"maxStage\":" << (app->mProfile ? app->mProfile->mMaxStage : 0) << ",\"inputFocus\":" << (app->mWidgetManager->mFocus && (app->GetDialog(Sexy::DialogType_CreateUser) || app->GetDialog(Sexy::DialogType_RenameUser)) && app->mWidgetManager->mFocus->mOwner?"true":"false");
 if (b) out << ",\"state\":" << b->mGameState << ",\"stage\":" << b->mLevelDesc->mStage << ",\"level\":" << b->mLevelDesc->mLevel << ",\"index\":" << b->mLevel << ",\"name\":\"" << escape(b->mLevelDesc->mName) << "\",\"title\":\"" << escape(b->mLevelDesc->mDisplayName) << "\",\"score\":" << b->mScore << ",\"scoreDisplay\":" << b->mScoreDisplay << ",\"target\":" << b->mScoreTarget << ",\"lives\":" << b->mLives << ",\"ticks\":" << b->mStateCount << ",\"curves\":" << b->mNumCurves << ",\"gunX\":" << b->mGun->GetCenterX() << ",\"gunY\":" << b->mGun->GetCenterY() << ",\"gunAngle\":" << b->mGun->GetAngle() << ",\"accuracy\":" << b->mAccuracyCount << ",\"cleared\":" << b->mLevelStats.mNumBallsCleared << ",\"combos\":" << b->mLevelStats.mNumCombos << ",\"gaps\":" << b->mLevelStats.mNumGaps << ",\"coins\":" << b->mLevelStats.mNumGemsCleared << ",\"practice\":" << (b->IsPracticeMode() ? "true" : "false") << ",\"endless\":" << (b->mIsEndless ? "true" : "false");
 if (b) out << ",\"continue\":" << (b->mContinueButton->mVisible ? "true" : "false") << ",\"gunColor\":" << (b->mGun->GetBullet() ? b->mGun->GetBullet()->GetType() : -1) << ",\"nextColor\":" << (b->mGun->GetNextBullet() ? b->mGun->GetNextBullet()->GetType() : -1) << ",\"time\":" << b->mLevelStats.mTimePlayed << ",\"par\":" << b->mLevelDesc->mParTime << ",\"paused\":" << b->mPauseCount << ",\"powerCounts\":[" << b->mCurveMgr[0]->WebSlowCount() << ',' << b->mCurveMgr[0]->WebBackwardCount() << ']';
 out << ",\"adventureSaved\":" << (app->mProfile && WebFileData(Sexy::GetSaveGameName(false, app->mProfile->mId)) ? "true" : "false") << ",\"practiceSaved\":" << (app->mProfile && WebFileData(Sexy::GetSaveGameName(true, app->mProfile->mId)) ? "true" : "false");
 out << ",\"boards\":["; bool first = true;
 if(app->mProfile)for (const auto& pair : app->mProfile->mMaxBoardLevel) { if (!first) out << ','; first = false; out << "{\"name\":\"" << escape(pair.first) << "\",\"difficulty\":" << pair.second << '}'; }
 out << "],\"controls\":["; first=true;
 for(auto* widget:app->mWidgetManager->mWidgets) {
  bool visible=widget->mVisible;auto* root=widget;while(root->mOwner){root=root->mOwner;visible=visible && root->mVisible;}
  auto* focus=app->mWidgetManager->InputRoot();
  auto label=widget->AccessibilityLabel();
  if(!visible || root!=focus || label.empty())continue;
  if(!first)out<<',';first=false;
  out<<"{\"label\":\""<<escape(label)<<"\",\"x\":"<<widget->mX<<",\"y\":"<<widget->mY<<",\"width\":"<<widget->mWidth<<",\"height\":"<<widget->mHeight<<",\"disabled\":"<<(widget->mDisabled?"true":"false")<<'}';
 }
 out << "]}"; snapshot = out.str(); return snapshot.c_str();
}
extern "C" const char* web_balls() {
 std::ostringstream out; out << '['; bool first = true;
 auto* b = app->mBoard;
 if (b) for (int i = 0; i < b->mNumCurves; ++i) for (auto* ball : b->mCurveMgr[i]->WebBalls()) {
  if (!first) out << ','; first = false;
  out << "{\"id\":" << ball->GetId() << ",\"curve\":" << i << ",\"x\":" << ball->GetX() << ",\"y\":" << ball->GetY() << ",\"color\":" << ball->GetType() << ",\"position\":" << ball->GetWayPoint() << ",\"power\":" << ball->GetPowerTypeWussy() << ",\"clear\":" << ball->GetClearCount() << '}';
 }
 out << ']'; ballData = out.str(); return ballData.c_str();
}
extern "C" const char* web_levels() {
 std::ostringstream out; out << '['; bool first = true; int index = 0;
 for (const auto& stage : app->mLevelParser->mStageMap) for (const auto& level : stage) {
  if (!first) out << ','; first = false;
  out << "{\"index\":" << index++ << ",\"stage\":" << level.mStage << ",\"level\":" << level.mLevel << ",\"name\":\"" << escape(level.mName) << "\",\"title\":\"" << escape(level.mDisplayName) << "\",\"curves\":" << const_cast<Sexy::LevelDesc&>(level).GetNumCurves() << ",\"speed\":" << level.mCurveDesc[0].mSpeed << ",\"target\":" << level.mCurveDesc[0].mScoreTarget << '}';
 }
 out << ']'; levelData = out.str(); return levelData.c_str();
}
extern "C" void web_qa(int action, int value) {
 auto* b = app->mBoard; if (!b) return;
 if (action == 0) { b->mStartLevel = max(0, min(75, value)); b->Reset(true); screen = "game"; }
 if (action == 1) { if (b->mGameState == Sexy::GameState_LevelBegin) b->StartLevel(); b->DoLevelUp(false, true); }
 if (action == 2) for (int i = 0; i < b->mNumCurves; ++i) b->mCurveMgr[i]->AddPowerUp(static_cast<Sexy::PowerType>(value));
 if (action == 3) b->SetLosing();
 if (action == 4) { auto* bullet = b->mGun->GetBullet(); if (bullet) bullet->SetType(value); }
 if (action == 5) { for (int i = 0; i < b->mNumCurves; ++i) { auto& balls = b->mCurveMgr[i]->WebBalls(); if (!balls.empty()) { auto* ball = balls.front(); ball->SetPowerType(static_cast<Sexy::PowerType>(value), false); b->ActivatePower(ball); } } }
 if (action == 6) { b->IncScore(value, false); }
 if (action == 7) { b->mLives = value; }
}
