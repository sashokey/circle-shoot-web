#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <strings.h>
#include <exception>
using std::min;
using std::max;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef uint32_t DWORD;
typedef int BOOL;
struct RECT { int left, top, right, bottom; };
#define TRUE 1
#define FALSE 0
#define _S(x) x
#define DBG_ASSERT(x) assert(x)
#define DBG_ASSERTE(x) assert(x)
#define SDL_min(a,b) ((a) < (b) ? (a) : (b))
#define SDL_max(a,b) ((a) > (b) ? (a) : (b))
#define SDL_arraysize(a) (sizeof(a) / sizeof((a)[0]))
#define SDL_Log(...) ((void)0)
namespace Sexy {
using SexyString = std::string;
struct StringLessNoCase { bool operator()(const std::string& a, const std::string& b) const { return strcasecmp(a.c_str(), b.c_str()) < 0; } };
inline uint32_t FromLE32(uint32_t v) { return v; }
inline ushort FromLE16(ushort v) { return v; }
inline uint32_t ToLE32(uint32_t v) { return v; }
inline ushort ToLE16(ushort v) { return v; }
std::string StrFormat(const char* format, ...);
int StrFindNoCase(const char* text, const char* needle);
std::string StringToLower(const std::string& text);
std::string Trim(const std::string& text);
std::string RemoveTrailingSlash(const std::string& text);
std::string GetFileDir(const std::string& text, bool trailing = false);
std::string GetAppDataPath(const std::string& text = "");
std::string GetFileName(const std::string& text, bool extension = false);
bool MkDir(const std::string& path);
bool Deltree(const std::string& path);
int Rand();
int Rand(int range);
float Rand(float range);
void SRand(unsigned long seed);
}
const std::vector<uchar>* WebFileData(const std::string& path);
extern "C" int casepath(const char* path, char* result);
