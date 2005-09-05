/*
AutoHotkey

Copyright 2003-2005 Chris Mallett

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef util_h
#define util_h

#include "stdafx.h" // pre-compiled headers
#include "defines.h"
EXTERN_G;  // For ITOA() and related functions' use of g.FormatIntAsHex

#define IS_SPACE_OR_TAB(c) (c == ' ' || c == '\t')
#define IS_SPACE_OR_TAB_OR_NBSP(c) (c == ' ' || c == '\t' || c == -96) // Use a negative to support signed chars.

//inline int iround(double x)  // Taken from someone's "Snippets".
//{
//	return (int)floor(x + ((x >= 0) ? 0.5 : -0.5));
//}


inline char *StrToTitleCase(char *aStr)
{
	if (!aStr) return aStr;
	char *aStr_orig = aStr;	
	for (bool convert_next_alpha_char_to_upper = true; *aStr; ++aStr)
	{
		if (IsCharAlpha(*aStr)) // Use this to better support chars from non-English languages.
		{
			if (convert_next_alpha_char_to_upper)
			{
				*aStr = (char)CharUpper((LPTSTR)(UCHAR)*aStr);
				convert_next_alpha_char_to_upper = false;
			}
			else
				*aStr = (char)CharLower((LPTSTR)(UCHAR)*aStr);
		}
		else
			if (isspace((UCHAR)*aStr))
				convert_next_alpha_char_to_upper = true;
		// Otherwise, it's a digit, punctuation mark, etc. so nothing needs to be done.
	}
	return aStr_orig;
}



inline size_t strnlen(char *aBuf, size_t aMax)
// Returns the length of aBuf or aMax, whichever is least.
// But it does so efficiently, in case aBuf is huge.
{
	if (!aMax || !aBuf || !*aBuf) return 0;
	size_t i;
	for (i = 0; aBuf[i] && i < aMax; ++i);
	return i;
}



inline char *StrChrAny(char *aStr, char *aCharList)
// Returns the position of the first char in aStr that is of any one of
// the characters listed in aCharList.  Returns NULL if not found.
// Update: Yes, this seems identical to strpbrk().  However, since the corresponding code would
// have to be added to the EXE regardless of which was used, there doesn't seem to be much
// advantage to switching (especially since if the two differ in behavior at all, things might
// get broken).  Another reason is the name "strpbrk()" is not as easy to remember.
{
	if (aStr == NULL || aCharList == NULL) return NULL;
	if (!*aStr || !*aCharList) return NULL;
	// Don't use strchr() because that would just find the first occurrence
	// of the first search-char, which is not necessarily the first occurrence
	// of *any* search-char:
	char *look_for_this_char, char_being_analyzed;
	for (; *aStr; ++aStr) // It's safe to use the value-parameter itself.
		// If *aStr is any of the search char's, we're done:
		for (char_being_analyzed = *aStr, look_for_this_char = aCharList; *look_for_this_char; ++look_for_this_char)
			if (char_being_analyzed == *look_for_this_char)
				return aStr;  // Match found.
	return NULL; // No match.
}



inline char *omit_leading_whitespace(char *aBuf)
// While aBuf points to a whitespace, moves to the right and returns the first non-whitespace
// encountered.
{
	for (; IS_SPACE_OR_TAB(*aBuf); ++aBuf);
	return aBuf;
}



inline char *omit_leading_any(char *aBuf, char *aOmitList, size_t aLength)
// Returns the address of the first character in aBuf that isn't a member of aOmitList.
// But no more than aLength characters of aBuf will be considered.  If aBuf is composed
// entirely of omitted characters, the address of the char after the last char in the
// string will returned (that char will be the zero terminator unless aLength explicitly
// caused only part of aBuf to be considered).
{
	char *cp;
	for (size_t i = 0; i < aLength; ++i, ++aBuf)
	{
		// Check if the current char is a member of the omitted-char list:
		for (cp = aOmitList; *cp; ++cp)
			if (*aBuf == *cp) // Match found.
				break;
		if (!*cp) // No match found, so this character is not omitted, thus we immediately return it's position.
			return aBuf;
	}
	// Since the above didn't return, aBuf is the position of the zero terminator or (if aLength
	// indicated only a substring) the position of the char after the last char in the substring.
	return aBuf;
}



inline char *omit_trailing_whitespace(char *aBuf, char *aBuf_marker)
// aBuf_marker must be a position in aBuf (to the right of it).
// Starts at aBuf_marker and keeps moving to the left until a non-whitespace
// char is encountered.  Returns the position of that char.
{
	for (; aBuf_marker > aBuf && IS_SPACE_OR_TAB(*aBuf_marker); --aBuf_marker);
	return aBuf_marker;  // Can equal aBuf.
}



inline size_t omit_trailing_any(char *aBuf, char *aOmitList, char *aBuf_marker)
// aBuf_marker must be a position in aBuf (to the right of it).
// Starts at aBuf_marker and keeps moving to the left until a char that isn't a member
// of aOmitList is found.  The length of the remaining substring is returned.
// That length will be zero if the string consists entirely of omitted characters.
{
	char *cp;
	for (; aBuf_marker > aBuf; --aBuf_marker)
	{
		// Check if the current char is a member of the omitted-char list:
		for (cp = aOmitList; *cp; ++cp)
			if (*aBuf_marker == *cp) // Match found.
				break;
		if (!*cp) // No match found, so this character is not omitted, thus we immediately return.
			return (aBuf_marker - aBuf) + 1; // The length of the string when trailing chars are omitted.
	}
	// Since the above didn't return, aBuf_marker is now equal to aBuf.  If this final character is itself
	// a member of the omitted-list, the length returned will be zero.  Otherwise it will be 1:
	for (cp = aOmitList; *cp; ++cp)
		if (*aBuf_marker == *cp) // Match found.
			return 0;
	return 1;
}



inline size_t ltrim(char *aStr, size_t aLength = -1)
// Caller must ensure that aStr is not NULL.
// v1.0.25: Returns the length if it was discovered as a result of the operation, or aLength otherwise.
// This greatly improves the performance of PerformAssign().
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids
// trimming newlines because some callers want to retain those.
{
	if (!*aStr) return 0;
	char *ptr;
	// Find the first non-whitespace char (which might be the terminator):
	for (ptr = aStr; IS_SPACE_OR_TAB(*ptr); ++ptr);
	// v1.0.25: If no trimming needed, don't do the memmove.  This seems to make a big difference
	// in the performance of critical sections of the program such as PerformAssign():
	size_t offset;
	if (offset = ptr - aStr) // Assign.
	{
		if (aLength == -1)
			aLength = strlen(ptr); // Set aLength as new/trimmed length, for use below and also as the return value.
		else // v1.0.25.05 bug-fix: Must adjust the length provided by caller to reflect what we did here.
			aLength -= offset;
		memmove(aStr, ptr, aLength + 1); // +1 to include the '\0'.  memmove() permits source & dest to overlap.
	}
	return aLength;
}

inline size_t rtrim(char *aStr, size_t aLength = -1)
// Caller must ensure that aStr is not NULL.
// v1.0.25: Always returns the new length of the string.  This greatly improves the performance of
// PerformAssign().
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids trimming newlines because
// some callers want to retain those.
{
	if (!*aStr) return 0; // The below relies upon this check having been done.
	// It's done this way in case aStr just happens to be address 0x00 (probably not possible
	// on Intel & Intel-clone hardware) because otherwise --cp would decrement, causing an
	// underflow since pointers are probably considered unsigned values, which would
	// probably cause an infinite loop.  Extremely unlikely, but might as well try
	// to be thorough:
	if (aLength == -1)
		aLength = strlen(aStr); // Set aLength for use below and also as the return value.
	for (char *cp = aStr + aLength - 1; ; --cp, --aLength)
	{
		if (!IS_SPACE_OR_TAB(*cp))
		{
			cp[1] = '\0';
			return aLength;
		}
		// Otherwise, it is a space or tab...
		if (cp == aStr) // ... and we're now at the first character of the string...
		{
			if (IS_SPACE_OR_TAB(*cp)) // ... and that first character is also a space or tab...
				*cp = '\0'; // ... so the entire string is made empty...
			return aLength; // ... and we return in any case.
		}
		// else it's a space or tab, and there are still more characters to check.  Let the loop
		// do its decrements.
	}
}

inline size_t rtrim_with_nbsp(char *aStr, size_t aLength = -1)
// Returns the new length of the string.
// Caller must ensure that aStr is not NULL.
// Same as rtrim but also gets rid of those annoying nbsp (non breaking space) chars that sometimes
// wind up on the clipboard when copied from an HTML document, and thus get pasted into the text
// editor as part of the code (such as the sample code in some of the examples).
{
	if (!*aStr) return 0; // The below relies upon this check having been done.
	if (aLength == -1)
		aLength = strlen(aStr); // Set aLength for use below and also as the return value.
	for (char *cp = aStr + aLength - 1; ; --cp, --aLength)
	{
		if (!IS_SPACE_OR_TAB_OR_NBSP(*cp))
		{
			cp[1] = '\0';
			return aLength;
		}
		if (cp == aStr)
		{
			if (IS_SPACE_OR_TAB_OR_NBSP(*cp))
				*cp = '\0';
			return aLength;
		}
	}
}

inline size_t trim(char *aStr, size_t aLength = -1)
// Caller must ensure that aStr is not NULL.
// Returns new length of aStr.
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids
// trimming newlines because some callers want to retain those.
{
	aLength = ltrim(aStr, aLength);  // It may return -1 to indicate that it still doesn't know the length.
    return rtrim(aStr, aLength);
	// v1.0.25: rtrim() always returns the new length of the string.  This greatly improves the
	// performance of PerformAssign() and possibly other things.
}



// Transformation is the same in either direction because the end bytes are swapped
// and the middle byte is left as-is:
#define bgr_to_rgb(aBGR) rgb_to_bgr(aBGR)
inline COLORREF rgb_to_bgr(DWORD aRGB)
// Fancier methods seem prone to problems due to byte alignment or compiler issues.
{
	return RGB(GetBValue(aRGB), GetGValue(aRGB), GetRValue(aRGB));
}



inline bool IsHex(char *aBuf)
// Note: AHK support for hex ints reduces performance by only 10% for decimal ints, even in the tightest
// of math loops that have SetBatchLines set to -1.
{
	// For whatever reason, omit_leading_whitespace() benches consistently faster (albeit slightly) than
	// the same code put inline:
	//for (; IS_SPACE_OR_TAB(*aBuf); ++aBuf);
	aBuf = omit_leading_whitespace(aBuf); // i.e. caller doesn't have to have ltrimmed.
	if (!*aBuf)
		return false;
	if (*aBuf == '-' || *aBuf == '+')
		++aBuf;
	// The "0x" prefix must be followed by at least one hex digit, otherwise it's not considered hex:
	#define IS_HEX(buf) (*buf == '0' && (*(buf + 1) == 'x' || *(buf + 1) == 'X') && isxdigit(*(buf + 2)))
	return IS_HEX(aBuf);
}



// As of v1.0.30, ATOI(), ITOA() and the other related functions below are no longer macros
// because there are too many places where something like ATOI(++cp) is done, which would be a
// bug if not caught since cp would be incremented more than once if the macro referred to that
// arg more than once.  In addition, a non-comprehensive, simple benchmark shows that the
// macros don't perform any better anyway, probably in part because there are many times when
// something like ATOI(ARG1) is called, which forces the ARG1 macro to be expanded two or more
// times within ATOI (when it was a macro).  So for now, the below are declared as inline.
// However, it seems that the compiler chooses not to make them truly inline, which as it
// turns out is probably the right decision since a simple benchmark shows that even with
// __forceinline in effect for all of them (which is confirmed to actually force inline),
// the performance isn't any better.

inline __int64 ATOI64(char *buf)
// The following comment only applies if the code is a macro or actually put inline by the compiler,
// which is no longer true:
// A more complex macro is used for ATOI64(), since it is more often called from places where
// performance matters (e.g. ACT_ADD).  It adds about 500 bytes to the code size  in exchance for
// a 8% faster math loops.  But it's probably about 8% slower when used with hex integers, but
// those are so rare that the speed-up seems worth the extra code size:
//#define ATOI64(buf) _strtoi64(buf, NULL, 0) // formerly used _atoi64()
{
	return IsHex(buf) ? _strtoi64(buf, NULL, 16) : _atoi64(buf);  // _atoi64() has superior performance, so use it when possible.
}

inline unsigned __int64 ATOU64(char *buf)
{
	return _strtoui64(buf, NULL, IsHex(buf) ? 16 : 10);
}

inline int ATOI(char *buf)
{
	// Below has been updated because values with leading zeros were being intepreted as
	// octal, which is undesirable.
	// Formerly: #define ATOI(buf) strtol(buf, NULL, 0) // Use zero as last param to support both hex & dec.
	return IsHex(buf) ? strtol(buf, NULL, 16) : atoi(buf); // atoi() has superior performance, so use it when possible.
}

// v1.0.38.01: Make ATOU a macro that refers to ATOI64() to improve performance (takes advantage of _atoi64()
// being considerably faster than strtoul(), at least when the number is non-hex).  This relies on the fact
// that ATOU() and (UINT)ATOI64() produce the same result due to the way casting works.  For example:
// ATOU("-1") == (UINT)ATOI64("-1")
// ATOU("-0xFFFFFFFF") == (UINT)ATOI64("-0xFFFFFFFF")
#define ATOU(buf) (UINT)ATOI64(buf)
//inline unsigned long ATOU(char *buf)
//{
//	// As a reminder, strtoul() also handles negative numbers.  For example, ATOU("-1") is
//	// 4294967295 (0xFFFFFFFF) and ATOU("-2") is 4294967294.
//	return strtoul(buf, NULL, IsHex(buf) ? 16 : 10);
//}

inline double ATOF(char *buf)
// Unlike some Unix versions of strtod(), the VC++ version does not seem to handle hex strings
// such as "0xFF" automatically.  So this macro must check for hex because some callers rely on that.
// Also, it uses _strtoi64() vs. strtol() so that more of a double's capacity can be utilized:
{
	return IsHex(buf) ? (double)_strtoi64(buf, NULL, 16) : atof(buf);
}

inline char *ITOA(int value, char *buf)
{
	if (g.FormatIntAsHex)
	{
		char *our_buf_temp = buf;
		// Negative hex numbers need special handling, otherwise something like zero minus one would create
		// a huge 0xffffffffffffffff value, which would subsequently not be read back in correctly as
		// a negative number (but UTOA() doesn't need this since there can't be negatives in that case).
		if (value < 0)
			*our_buf_temp++ = '-';
		*our_buf_temp++ = '0';
		*our_buf_temp++ = 'x';
		_itoa(value < 0 ? -(int)value : value, our_buf_temp, 16);
		// Must not return the result of the above because it's our_buf_temp and we want buf.
		return buf;
	}
	else
		return _itoa(value, buf, 10);
}

inline char *ITOA64(__int64 value, char *buf)
{
	if (g.FormatIntAsHex)
	{
		char *our_buf_temp = buf;
		if (value < 0)
			*our_buf_temp++ = '-';
		*our_buf_temp++ = '0';
		*our_buf_temp++ = 'x';
		_i64toa(value < 0 ? -(__int64)value : value, our_buf_temp, 16);
		// Must not return the result of the above because it's our_buf_temp and we want buf.
		return buf;
	}
	else
		return _i64toa(value, buf, 10);
}

inline char *UTOA(unsigned long value, char *buf)
{
	if (g.FormatIntAsHex)
	{
		*buf = '0';
		*(buf + 1) = 'x';
		_ultoa(value, buf + 2, 16);
		// Must not return the result of the above because it's buf + 2 and we want buf.
		return buf;
	}
	else
		return _ultoa(value, buf, 10);
}

// Not currently used:
//inline char *UTOA64(unsigned __int64 value, char *buf) 
//{
//	if (g.FormatIntAsHex)
//	{
//		*buf = '0';
//		*(buf + 1) = 'x';
//		return _ui64toa(value, buf + 2, 16);
//	}
//	else
//		return _ui64toa(value, buf, 10);
//}



inline void strlcpy (char *aDst, const char *aSrc, size_t aDstSize)
// Same as strncpy() but guarantees null-termination of aDst upon return.
// No more than aDstSize - 1 characters will be copied from aSrc into aDst
// (leaving room for the zero terminator, which is always inserted).
// This function is defined in some Unices but is not standard.  But unlike
// other versions, this one uses void for return value for reduced code size
// (since it's called in so many places).
{
	// Disabled for performance and reduced code size:
	//if (!aDst || !aSrc || !aDstSize) return aDstSize;  // aDstSize must not be zero due to the below method.
	// It might be worthwhile to have a custom char-copying-loop here someday so that number of characters
	// actually copied (not including the zero terminator) can be returned to callers who want it.
	strncpy(aDst, aSrc, aDstSize - 1);
	aDst[aDstSize - 1] = '\0';
}



//inline char *strcatmove(char *aDst, char *aSrc)
//// Same as strcat() but allows aSrc and aDst to overlap.
//// Unlike strcat(), it doesn't return aDst.  Instead, it returns the position
//// in aDst where aSrc was appended.
//{
//	if (!aDst || !aSrc || !*aSrc) return aDst;
//	char *aDst_end = aDst + strlen(aDst);
//	return (char *)memmove(aDst_end, aSrc, strlen(aSrc) + 1);  // Add 1 to include aSrc's terminator.
//}



#define DATE_FORMAT_LENGTH 14 // "YYYYMMDDHHMISS"
#define IS_LEAP_YEAR(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

int GetYDay(int aMon, int aDay, bool aIsLeapYear);
int GetISOWeekNumber(char *aBuf, int aYear, int aYDay, int aWDay);
ResultType YYYYMMDDToFileTime(char *aYYYYMMDD, FILETIME &aFileTime);
DWORD YYYYMMDDToSystemTime2(char *aYYYYMMDD, SYSTEMTIME *aSystemTime);
ResultType YYYYMMDDToSystemTime(char *aYYYYMMDD, SYSTEMTIME &aSystemTime, bool aDoValidate);
char *FileTimeToYYYYMMDD(char *aBuf, FILETIME &aTime, bool aConvertToLocalTime = false);
char *SystemTimeToYYYYMMDD(char *aBuf, SYSTEMTIME &aTime);
__int64 YYYYMMDDSecondsUntil(char *aYYYYMMDDStart, char *aYYYYMMDDEnd, bool &aFailed);
__int64 FileTimeSecondsUntil(FILETIME *pftStart, FILETIME *pftEnd);

SymbolType IsPureNumeric(char *aBuf, bool aAllowNegative = false
	, bool aAllowAllWhitespace = true, bool aAllowFloat = false, bool aAllowImpure = false);

int snprintf(char *aBuf, int aBufSize, const char *aFormat, ...);
int snprintfcat(char *aBuf, int aBufSize, const char *aFormat, ...);
// Not currently used by anything, so commented out to possibly reduce code size:
//int strlcmp (char *aBuf1, char *aBuf2, UINT aLength1 = UINT_MAX, UINT aLength2 = UINT_MAX);
int strlicmp(char *aBuf1, char *aBuf2, UINT aLength1 = UINT_MAX, UINT aLength2 = UINT_MAX);
char *strrstr(char *aStr, char *aPattern, bool aCaseSensitive = true, int aOccurrence = 1);
char *strcasestr (const char *phaystack, const char *pneedle);
char *StrReplace(char *aBuf, char *aOld, char *aNew, bool aCaseSensitive = true);
char *StrReplaceAll(char *aBuf, char *aOld, char *aNew, bool aAlwaysUseSlow, bool aCaseSensitive = true
	, DWORD aReplacementsNeeded = UINT_MAX); // Caller can provide this value to avoid having to calculate it again.
int StrReplaceAllSafe(char *aBuf, size_t aBufSize, char *aOld, char *aNew, bool aCaseSensitive = true);
char *TranslateLFtoCRLF(char *aString);
bool DoesFilePatternExist(char *aFilePattern, DWORD *aFileAttr = NULL);
#ifdef _DEBUG
	ResultType FileAppend(char *aFilespec, char *aLine, bool aAppendNewline = true);
#endif
char *ConvertFilespecToCorrectCase(char *aFullFileSpec);
char *FileAttribToStr(char *aBuf, DWORD aAttr);
unsigned __int64 GetFileSize64(HANDLE aFileHandle);
char *GetLastErrorText(char *aBuf, int aBufSize);
void AssignColor(char *aColorName, COLORREF &aColor, HBRUSH &aBrush);
COLORREF ColorNameToBGR(char *aColorName);
HRESULT MySetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
//HRESULT MyEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
char *ConvertEscapeSequences(char *aBuf, char aEscapeChar, bool aAllowEscapedSpace);
POINT CenterWindow(int aWidth, int aHeight);
bool FontExist(HDC aHdc, char *aTypeface);
void GetVirtualDesktopRect(RECT &aRect);
LPVOID AllocInterProcMem(HANDLE &aHandle, DWORD aSize, HWND aHwnd);
void FreeInterProcMem(HANDLE aHandle, LPVOID aMem);
ResultType RegReadString(HKEY aRootKey, char *aSubkey, char *aValueName, char *aBuf, size_t aBufSize);
HBITMAP LoadPicture(char *aFilespec, int aWidth, int aHeight, int &aImageType, int aIconIndex
	, bool aUseGDIPlusIfAvailable);
HBITMAP IconToBitmap(HICON ahIcon, bool aDestroyIcon);
int CALLBACK FontEnumProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);
bool IsStringInList(char *aStr, char *aList, bool aFindExactMatch, bool aCaseSensitive);

#endif
