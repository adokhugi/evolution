#ifndef _TEXTOBJECT_H_
#define _TEXTOBJECT_H_

#include <windows.h>
#include "AlignmentTypes.h"

class TextObject
{
public:
	void PreprocessText ();
	void DrawFormatted (HDC hdc, RECT *prc, AlignmentTypes align);
	void SetText (char *src);

private:
	void RemoveChars (PTSTR pBegin, PTSTR pEnd);
	int ProcessTabsAndLineBreaks (PTSTR pText);
	char *text;
};

#endif