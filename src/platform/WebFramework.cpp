#include "WebFramework.h"
#include "SexyAppFramework/XMLParser.h"
#include "CircleCommon.h"
#include "CircleButton.h"
#include "CircleShootApp.h"
#include "Board.h"
#include "WorkerThread.h"
#include "Res.h"
#include "CircleCheckbox.h"

extern "C" {
__attribute__((import_module("web"), import_name("surface"))) int host_surface(int width, int height);
__attribute__((import_module("web"), import_name("release"))) void host_release(int texture);
__attribute__((import_module("web"), import_name("image"))) void host_image(int target, int image, float x, float y, float width, float height, float sx, float sy, float sw, float sh, float rotation, float cx, float cy, uint32_t color, int mode, int clipX, int clipY, int clipW, int clipH);
__attribute__((import_module("web"), import_name("shape"))) void host_shape(int target, int type, float x, float y, float w, float h, uint32_t color, int clipX, int clipY, int clipW, int clipH);
__attribute__((import_module("web"), import_name("polygon"))) void host_polygon(int target, const void* points, int count, uint32_t color);
__attribute__((import_module("web"), import_name("text"))) void host_text(int target, int font, const char* text, int x, int y, uint32_t color, int clipX, int clipY, int clipW, int clipH);
__attribute__((import_module("web"), import_name("width"))) int host_width(int font, const char* text);
__attribute__((import_module("web"), import_name("pixels"))) void host_pixels(int image, uint32_t* pixels, int width, int height, int write);
__attribute__((import_module("web"), import_name("audio"))) void host_audio(int operation, int id, int asset, double volume, double pitch, int loop, int pan);
__attribute__((import_module("web"), import_name("store"))) void host_store(const char* path, const void* data, int length);
__attribute__((import_module("web"), import_name("message"))) void host_message(const char* message);
}

static std::map<std::string, std::vector<uchar>> files;
static std::string normalize(std::string path) {
 std::replace(path.begin(), path.end(), '\\', '/');
 for (auto& c : path) c = std::tolower((unsigned char)c);
 if (path.starts_with("content/")) path.erase(0, 8);
 return path;
}
const std::vector<uchar>* WebFileData(const std::string& path) {
 auto item = files.find(normalize(path));
 return item == files.end() ? nullptr : &item->second;
}
extern "C" void web_register_file(const char* path, const uchar* data, int length) { files[normalize(path)] = std::vector<uchar>(data, data + length); }
extern "C" int casepath(const char* path, char* result) { strcpy(result, path); return 1; }

namespace Sexy {
SexyAppBase* gSexyAppBase = nullptr;
const Color Color::White(255, 255, 255);
const Color Color::Black(0, 0, 0);
MTRand gThreadRand;
MTRand gAppRand;
int gButtonSequenceCount = 0, gLastButtonOver = 4, gSaveGameVersion = 5;
uint gBallColors[6] = {0x1980FF, 0xFFFF00, 0xFF0000, 0x00FF00, 0xFF00FF, 0xFFFFFF};
uint gBrightBallColors[6] = {0x80FFFF, 0xFFFF40, 0xFFAAAA, 0x80FF80, 0xFF80FF, 0xFFFFFF};
uint gDarkBallColors[6] = {0x231679, 0x60510A, 0xA00F14, 0x204422, 0x561643, 0x381B22};
uint gTextBallColors[6] = {0x2D8BFF, 0xFFFF00, 0xFF0000, 0x00FF00, 0xFF00FF, 0xFFFFFF};

std::string StrFormat(const char* format, ...) {
 va_list args, copy;
 va_start(args, format);
 va_copy(copy, args);
 int count = vsnprintf(nullptr, 0, format, args);
 va_end(args);
 std::string result(max(0, count), '\0');
 if (count > 0) vsnprintf(result.data(), count + 1, format, copy);
 va_end(copy);
 return result;
}
std::string StringToLower(const std::string& text) { std::string result = text; for (auto& c : result) c = std::tolower((unsigned char)c); return result; }
int StrFindNoCase(const char* text, const char* needle) { auto n = StringToLower(text).find(StringToLower(needle)); return n == std::string::npos ? -1 : int(n); }
std::string Trim(const std::string& text) { auto a = text.find_first_not_of(" \t\r\n"); if (a == std::string::npos) return ""; return text.substr(a, text.find_last_not_of(" \t\r\n") - a + 1); }
std::string RemoveTrailingSlash(const std::string& text) { auto value = text; while (!value.empty() && (value.back() == '/' || value.back() == '\\')) value.pop_back(); return value; }
std::string GetFileDir(const std::string& text, bool trailing) { auto pos = text.find_last_of("/\\"); return pos == std::string::npos ? "" : text.substr(0, pos + (trailing ? 1 : 0)); }
std::string GetFileName(const std::string& text, bool extension) { auto value = text.substr(text.find_last_of("/\\") + 1); if (!extension && value.find_last_of('.') != std::string::npos) value.resize(value.find_last_of('.')); return value; }
std::string GetAppDataPath(const std::string& text) { return text; }
bool MkDir(const std::string& path) { return true; }
bool Deltree(const std::string& path) { return true; }
int Rand() { return gAppRand.Next(); }
int Rand(int range) { return range > 0 ? gAppRand.Next(static_cast<unsigned long>(range)) : 0; }
float Rand(float range) { return gAppRand.Next(range); }
void SRand(unsigned long seed) { gAppRand.SRand(seed); }
int AppGetTickCount() { return gSexyAppBase ? gSexyAppBase->mUpdateCount * 10 : 0; }
int BoardGetTickCount() { return GetBoardStateCount() * 10; }
int GetBoardStateCount() { auto* board = GetCircleShootApp()->mBoard; return board ? board->GetStateCount() : 0; }
int AppRand() { return gAppRand.Next(); }
int ThreadRand() { return gAppRand.Next(); }
std::string GetTimeString(int time) { return time >= 3600 ? StrFormat("%d:%02d:%02d", time / 3600, (time / 60) % 60, time % 60) : StrFormat("%d:%02d", time / 60, time % 60); }
std::string GetSaveGameName(bool practice, int user) { return StrFormat("userdata/%s%d.dat", practice ? "practice" : "adventure", user); }
void LoadResourceGroup(const char* group) {}
void FreeResourceGroup(const char* group) {}
void MirrorPoint(float& x, float& y, MirrorType type) { if (type == MirrorType_X || type == MirrorType_XY) x = 640 - x; if (type == MirrorType_Y || type == MirrorType_XY) y = 480 - y; }
void MirrorPoint(int& x, int& y, MirrorType type) { float a = x, b = y; MirrorPoint(a, b, type); x = a; y = b; }
void MirrorRect(int& x, int& y, int w, int h, MirrorType type) { if (type == MirrorType_X || type == MirrorType_XY) x = 640 - x - w; if (type == MirrorType_Y || type == MirrorType_XY) y = 480 - y - h; }
void ImageMirror(MemoryImage* image, MirrorType type) {
 if (type == MirrorType_None || !image) return;
 auto* bits = image->GetBits();
 std::vector<uint32_t> original(bits, bits + image->mWidth * image->mHeight);
 for (int y = 0; y < image->mHeight; ++y) for (int x = 0; x < image->mWidth; ++x) {
  int sx = type == MirrorType_X || type == MirrorType_XY ? image->mWidth - x - 1 : x;
  int sy = type == MirrorType_Y || type == MirrorType_XY ? image->mHeight - y - 1 : y;
  bits[y * image->mWidth + x] = original[sy * image->mWidth + sx];
 }
 image->BitsChanged();
}
MemoryImage* CutoutImageFromAlpha(MemoryImage* background, MemoryImage* alpha, int x, int y) {
 auto* result = new MemoryImage();
 result->Create(alpha->mWidth, alpha->mHeight);
 Graphics g(result);
 g.DrawImage(background, 0, 0, Rect(x, y, alpha->mWidth, alpha->mHeight));
 auto* pixels = result->GetBits();
 auto* mask = alpha->GetBits();
 for (int i = 0; i < alpha->mWidth * alpha->mHeight; ++i) pixels[i] = (pixels[i] & 0x00FFFFFF) | (mask[i] & 0xFF000000);
 result->BitsChanged();
 return result;
}
void FillCircle(Graphics* g, int x, int y, int radius) {
 for (int dy = -radius; dy <= radius; ++dy) { int dx = sqrtf(float(radius * radius - dy * dy)); g->FillRect(x - dx, y + dy, dx * 2 + 1, 1); }
}
MemoryImage::MemoryImage(const MemoryImage& other) { Create(other.mWidth, other.mHeight); mNumRows = other.mNumRows; mNumCols = other.mNumCols; Graphics g(this); g.DrawImage(const_cast<MemoryImage*>(&other), 0, 0); }
MemoryImage::~MemoryImage() { if (mId > 0) host_release(mId); }
void MemoryImage::Create(int w, int h) { if (mId > 0) host_release(mId); mWidth = max(1, w); mHeight = max(1, h); mPixels.clear(); mId = host_surface(mWidth, mHeight); }
void MemoryImage::Clear() { host_shape(mId, 3, 0, 0, mWidth, mHeight, 0, 0, 0, mWidth, mHeight); mPixels.clear(); }
uint32_t* MemoryImage::GetBits() { if (mPixels.empty()) { mPixels.resize(mWidth * mHeight); host_pixels(mId, mPixels.data(), mWidth, mHeight, 0); } return mPixels.data(); }
void MemoryImage::BitsChanged() { if (!mPixels.empty()) host_pixels(mId, mPixels.data(), mWidth, mHeight, 1); }
void MemoryImage::SetBits(uint32_t* pixels, int w, int h, bool commit) { Create(w, h); mPixels.assign(pixels, pixels + w * h); BitsChanged(); }
int Font::StringWidth(const std::string& value) const { return host_width(mId, value.c_str()); }
int Font::CharWidth(char value) const { return StringWidth(std::string(1, value)); }
int Font::CharWidthKern(char value, char previous) const { return previous ? StringWidth(std::string{previous,value})-CharWidth(previous) : CharWidth(value); }
void Graphics::FillRect(int x, int y, int w, int h) { host_shape(mDestImage ? mDestImage->mId : 0, 0, x + mTransX, y + mTransY, w, h, mColor.ToInt(), mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight); }
void Graphics::DrawRect(int x, int y, int w, int h) { host_shape(mDestImage ? mDestImage->mId : 0, 1, x + mTransX, y + mTransY, w, h, mColor.ToInt(), mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight); }
void Graphics::DrawLine(int x, int y, int x2, int y2) { host_shape(mDestImage ? mDestImage->mId : 0, 2, x + mTransX, y + mTransY, x2 + mTransX, y2 + mTransY, mColor.ToInt(), mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight); }
void Graphics::PolyFill(const Point* points, int count, bool convex) { host_polygon(mDestImage ? mDestImage->mId : 0, points, count, mColor.ToInt()); }
void Graphics::DrawString(const std::string& text, int x, int y) { host_text(mDestImage ? mDestImage->mId : 0, mFont ? mFont->mId : 0, text.c_str(), x + mTransX, y + mTransY, mColor.ToInt(), mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight); }
static void imageCommand(Graphics* g, Image* image, float x, float y, float w, float h, const Rect& source, float angle, float cx, float cy) {
 if (!image || image->mId == 0 || w <= 0 || h <= 0 || source.mWidth <= 0 || source.mHeight <= 0) return;
 auto color = g->mColorizeImages ? g->mColor : Color::White;
 host_image(g->mDestImage ? g->mDestImage->mId : 0, image->mId, x + g->mTransX, y + g->mTransY, w, h, source.mX, source.mY, source.mWidth, source.mHeight, angle, cx, cy, color.ToInt(), g->mDrawMode, g->mClipRect.mX, g->mClipRect.mY, g->mClipRect.mWidth, g->mClipRect.mHeight);
}
void Graphics::DrawImage(Image* image, int x, int y) { DrawImageF(image, x, y); }
void Graphics::DrawImage(Image* image, int x, int y, const Rect& source) { DrawImageF(image, x, y, source); }
void Graphics::DrawImage(Image* image, const Rect& destination, const Rect& source) { imageCommand(this, image, destination.mX, destination.mY, destination.mWidth, destination.mHeight, source, 0, 0, 0); }
void Graphics::DrawImage(Image* image, int x, int y, int w, int h) { if (image) DrawImage(image, Rect(x, y, w, h), Rect(0, 0, image->mWidth, image->mHeight)); }
void Graphics::DrawImageF(Image* image, float x, float y) { if (image) DrawImageF(image, x, y, Rect(0, 0, image->mWidth, image->mHeight)); }
void Graphics::DrawImageF(Image* image, float x, float y, const Rect& source) { imageCommand(this, image, x, y, source.mWidth, source.mHeight, source, 0, 0, 0); }
void Graphics::DrawImageRotated(Image* image, int x, int y, double angle, const Rect* source) { DrawImageRotatedF(image, x, y, angle, source); }
void Graphics::DrawImageRotated(Image* image, int x, int y, double angle, int cx, int cy, const Rect* source) { DrawImageRotatedF(image, x, y, angle, cx, cy, source); }
void Graphics::DrawImageRotatedF(Image* image, float x, float y, double angle, const Rect* source) { if (!image) return; Rect r = source ? *source : Rect(0, 0, image->mWidth, image->mHeight); DrawImageRotatedF(image, x, y, angle, r.mWidth * 0.5f, r.mHeight * 0.5f, &r); }
void Graphics::DrawImageRotatedF(Image* image, float x, float y, double angle, float cx, float cy, const Rect* source) { if (!image) return; Rect r = source ? *source : Rect(0, 0, image->mWidth, image->mHeight); imageCommand(this, image, x, y, r.mWidth, r.mHeight, r, angle, cx, cy); }
void Graphics::DrawImageCel(Image* image, int x, int y, int cel) { if (!image || cel < 0 || cel >= image->mNumCols * image->mNumRows) return; DrawImage(image, x, y, image->GetCelRect(cel)); }
void Graphics::DrawImageCel(Image* image, const Rect& destination, int cel) { if (!image || cel < 0 || cel >= image->mNumCols * image->mNumRows) return; DrawImage(image, destination, image->GetCelRect(cel)); }
void GLInterface::BltTransformed(Image* image, const Rect* clip, const Color& color, int mode, const Rect& source, const SexyMatrix3& matrix, bool blend) {
 if (!image) return;
 Graphics g;
 if (clip) g.mClipRect = *clip;
 g.SetColor(color); g.SetColorizeImages(true); g.SetDrawMode(mode);
 float scaleX = sqrtf(matrix.m00 * matrix.m00 + matrix.m10 * matrix.m10);
 float scaleY = sqrtf(matrix.m01 * matrix.m01 + matrix.m11 * matrix.m11);
 imageCommand(&g, image, matrix.m02, matrix.m12, source.mWidth * scaleX, source.mHeight * scaleY, source, -atan2f(matrix.m10, matrix.m00), 0, 0);
}
void SoundInstance::SetVolume(double volume) { mVolume = volume; if (mPlaying) host_audio(2, mId, mAsset, mVolume, mPitch, 0, mPan); }
void SoundInstance::Play(bool loop, bool release) { mPlaying = true; host_audio(0, mId, mAsset, mVolume, mPitch, loop, mPan); }
void SoundInstance::Stop() { if (mPlaying) host_audio(1, mId, mAsset, 0, 0, 0, 0); mPlaying = false; }
SexyAppBase::SexyAppBase() { gSexyAppBase = this; mWidgetManager = new WidgetManager(); mWidgetManager->mApp=this; mSoundManager = new SoundManager(); mResourceManager = new ResourceManager(); mGLInterface = new GLInterface(); mDDInterface = new DDInterface(); }
extern std::map<std::string, MemoryImage*> webImages;
MemoryImage* SexyAppBase::GetImage(const std::string& path, bool commit) { auto name = normalize(path); auto i = webImages.find(name); if (i == webImages.end()) i = webImages.find(name + ".png"); if (i == webImages.end()) return nullptr; return new MemoryImage(*i->second); }
void SexyAppBase::PlaySample(int id, int pan) { auto* sample = mSoundManager->GetSoundInstance(id); sample->SetPan(pan); sample->Play(false, true); delete sample; }
bool SexyAppBase::ReadBufferFromFile(const std::string& path, Buffer* buffer) { auto* data = WebFileData(path); if (!data) return false; buffer->SetData(const_cast<uchar*>(data->data()), data->size()); return true; }
bool SexyAppBase::WriteBytesToFile(const std::string& path, const void* data, int length) { files[normalize(path)] = std::vector<uchar>((uchar*)data, (uchar*)data + length); host_store(path.c_str(), data, length); return true; }
bool SexyAppBase::EraseFile(const std::string& path) { files.erase(normalize(path)); host_store(path.c_str(), nullptr, 0); return true; }
void SexyAppBase::Popup(const std::string& text) { host_message(text.c_str()); }
WorkerThread::WorkerThread() : mTaskProc(nullptr) {}
WorkerThread::~WorkerThread() {}
void WorkerThread::DoTask(SexyThreadCallback* func, void* param) { func(param); }
void WorkerThread::WaitForTask() {}

bool XMLParser::OpenFile(const std::string& filename) { mFile = filename; auto* data = WebFileData(filename); if (!data) { mFailed = true; mError = "File not found: " + filename; return false; } SetStringSource(std::string(data->begin(), data->end())); return true; }
bool XMLParser::NextElement(XMLElement* element) {
 *element = XMLElement();
 if (!mPending.empty()) { element->mType = XMLElement::TYPE_END; element->mValue = mPending; mPending.clear(); return true; }
 while (mPosition < mText.size()) {
  auto start = mText.find('<', mPosition);
  if (start == std::string::npos) { mPosition = mText.size(); return false; }
  if (mText.compare(start, 4, "<!--") == 0) { auto end = mText.find("-->", start + 4); mPosition = end == std::string::npos ? mText.size() : end + 3; continue; }
  auto end = mText.find('>', start + 1);
  if (end == std::string::npos) { mFailed = true; mError = "Unclosed tag"; return false; }
  mPosition = end + 1;
  std::string tag = Trim(mText.substr(start + 1, end - start - 1));
  if (tag.empty() || tag.front() == '?' || tag.front() == '!') continue;
  if (tag.front() == '/') { element->mType = XMLElement::TYPE_END; element->mValue = Trim(tag.substr(1)); return true; }
  bool closed = tag.back() == '/';
  if (closed) tag.pop_back();
  size_t pos = 0;
  while (pos < tag.size() && !isspace((unsigned char)tag[pos])) ++pos;
  element->mType = XMLElement::TYPE_START;
  element->mValue = tag.substr(0, pos);
  while (pos < tag.size()) {
   while (pos < tag.size() && isspace((unsigned char)tag[pos])) ++pos;
   size_t a = pos;
   while (pos < tag.size() && tag[pos] != '=' && !isspace((unsigned char)tag[pos])) ++pos;
   std::string key = tag.substr(a, pos - a);
   while (pos < tag.size() && isspace((unsigned char)tag[pos])) ++pos;
   if (pos >= tag.size() || tag[pos++] != '=') break;
   while (pos < tag.size() && isspace((unsigned char)tag[pos])) ++pos;
   if (pos >= tag.size() || (tag[pos] != '\'' && tag[pos] != '"')) break;
   char quote = tag[pos++]; a = pos;
   while (pos < tag.size() && tag[pos] != quote) ++pos;
   element->mAttributes[key] = tag.substr(a, pos - a); ++pos;
  }
  if (closed) mPending = element->mValue;
  return true;
 }
 return false;
}
}
int gLastTypeFrame = 0;
