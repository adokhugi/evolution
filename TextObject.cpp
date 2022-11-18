#include "TextObject.h"

void TextObject::RemoveChars (PTSTR pBegin, PTSTR pEnd)
{
	do
	{
		*pBegin = *pEnd;
		pBegin++;
		pEnd++;
	}
	while (*(pEnd - 1) != '\0');
}

int TextObject::ProcessTabsAndLineBreaks (PTSTR pText)
{
	bool loop = true;
	int cSpaceChars = 0;

	while (loop)
	{
		loop = false;

		if (*pText == '\t' || *pText == '\n' || *pText == '\r')
		{
			if (cSpaceChars == 0)
			{
				*pText = ' ';
				pText++;
				cSpaceChars++;
			}
			else
				RemoveChars (pText, pText + 1);
			loop = true;
		}
	}

	return cSpaceChars;
}

void TextObject::PreprocessText ()
{
	int cSpaceChars;
	PTSTR pText, pPrevious;

	pText = text;

	while (*pText != '\0')
	{
		pPrevious = pText;
		cSpaceChars = 0;

		while (*pText == ' ')
		{
			cSpaceChars++;
			pText++;
		}

		if (cSpaceChars > 1)
			RemoveChars (pPrevious + 1, pText);

		cSpaceChars += ProcessTabsAndLineBreaks (pText);

		pText++;
	}
}

void TextObject::DrawFormatted (HDC hdc, RECT *prc, AlignmentTypes align)
{
	int		xStart, yStart, cSpaceChars;
	PTSTR	pBegin, pEnd, pText;
	SIZE	size;

	pText = text;
	yStart = prc->top;

	do
	{
		cSpaceChars = 0;
		
		while (*pText == ' ')
			pText++;

		pBegin = pText;

		do
		{
			pEnd = pText;
			
			while (*pText != '\0' && *pText != ' ')
				pText++;

			if (*pText == ' ')
			{
				pText++;
				cSpaceChars++;
				GetTextExtentPoint32A (hdc, pBegin, pText - pBegin, &size);
			}
		} while (*pText != '\0' && size.cx < (prc->right - prc->left));

		if (*pText != '\0')
			cSpaceChars--;

		while (*(pEnd - 1) == ' ')
		{
			pEnd--;
			cSpaceChars--;
		}

		if (cSpaceChars < 0 && *pText != '\0' && size.cx >= (prc->right - prc->left))
		{
			pText = pBegin;

			cSpaceChars = 0;

			do
			{
				pEnd = pText;
			
				pText++;
				GetTextExtentPoint32A (hdc, pBegin, pText - pBegin, &size);

				if (*pText == ' ')
					cSpaceChars++;
			} while (size.cx < (prc->right - prc->left));
		}

		if (*pText == '\0')
		{
			GetTextExtentPoint32 (hdc, pBegin, pText - pBegin - 1, &size);
			if (size.cx < (prc->right - prc->left))
				pEnd = pText;
		}

		GetTextExtentPoint32 (hdc, pBegin, pEnd - pBegin, &size);

		switch (align)
		{
		case Align_Left:
			xStart = prc->left;
			break;

		case Align_Right:
			xStart = prc->right - size.cx;
			break;

		case Align_Center:
			xStart = (prc->right + prc->left - size.cx) / 2;
			break;

		case Align_Justify:
			if (*pEnd != '\0' && cSpaceChars > 0)
				SetTextJustification (hdc, prc->right - prc->left - size.cx, cSpaceChars);
			xStart = prc->left;
			break;
		}

		TextOut (hdc, xStart, yStart, pBegin, pEnd - pBegin);

		SetTextJustification (hdc, 0, 0);
		yStart += size.cy;
		pText = pEnd;
	}
	while (*pText && yStart < prc->bottom - size.cy);
}

void TextObject::SetText (char *src)
{
	text = new char [strlen (src) + 1];
	strcpy (text, src);
}