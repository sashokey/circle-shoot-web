#pragma once
#include "Common.h"
namespace Sexy {
typedef std::map<std::string, std::string> XMLParamMap;
class XMLElement {
public:
 enum { TYPE_NONE, TYPE_START, TYPE_END, TYPE_ELEMENT, TYPE_INSTRUCTION, TYPE_COMMENT };
 int mType = TYPE_NONE;
 std::string mSection, mValue, mInstruction;
 XMLParamMap mAttributes;
 std::list<XMLParamMap::iterator> mAttributeIteratorList;
};
class XMLParser {
 std::string mText, mFile, mError, mPending;
 size_t mPosition = 0;
 bool mFailed = false;
public:
 enum XMLEncodingType { ASCII, UTF_8, UTF_16, UTF_16_LE, UTF_16_BE };
 void SetEncodingType(XMLEncodingType encoding) {}
 bool OpenFile(const std::string& filename);
 void SetStringSource(const std::string& text) { mText = text; mPosition = 0; mFailed = false; mPending.clear(); }
 bool NextElement(XMLElement* element);
 std::string GetErrorText() { return mError; }
 int GetCurrentLineNum() { return std::count(mText.begin(), mText.begin() + mPosition, '\n') + 1; }
 std::string GetFileName() { return mFile; }
 void AllowComments(bool value) {}
 bool HasFailed() { return mFailed; }
 bool EndOfFile() { return mPosition >= mText.size() && mPending.empty(); }
};
}
