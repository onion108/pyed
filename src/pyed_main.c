/*
 * Name:        pyed_main.c
 * Description: Console based Pinyin input method.
 * Author:      cosh.cage#hotmail.com
 * File ID:     0701251600A0701252004L00223
 * License:     Public Domain.
 */
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <wctype.h>
#include "svstring.h"
#include "svtree.h"

#define BUF_SIZ 4096
#define LN_BUF  500

wchar_t pybuf[BUF_SIZ], * pyp = pybuf;
wchar_t chbuf[BUF_SIZ], * chp = chbuf;

wchar_t * lnbuf[LN_BUF] = { NULL }, ** lbp = lnbuf;

int cbfcmpWchar_t(const void * px, const void * py)
{
	return *(wchar_t *)px - *(wchar_t *)py;
}

P_TRIE_A GetBase(const char * szfile)
{
	P_TRIE_A ptaBase;
	size_t * psiz = NULL;
	if (NULL == (ptaBase = treCreateTrieA()))
		return NULL;
	else
	{
		FILE * fpbase = fopen(szfile, "r");
		if (NULL != fpbase)
		{
			while (!feof(fpbase))
			{
				wchar_t ch = fgetwc(fpbase);
				if (pyp - pybuf >= BUF_SIZ || chp - chbuf >= BUF_SIZ)
					goto Lbl_Failed;
				if (ch >= L'a' && ch <= L'z')
				{
					chp = chbuf;
					*pyp++ = ch;
					*pyp = 0;
				}
				else if ((L' ' == ch) || (L'\n' == ch) || (L'\t' == ch) || (L'\r' == ch))
				{
					if (pyp != pybuf)
					{
						treInsertTrieA(ptaBase, pybuf, wcslen(pybuf), sizeof(wchar_t), 0, cbfcmpWchar_t);
					}
					pyp = pybuf;
					if (chp != chbuf)
					{
						psiz = treSearchTrieA(ptaBase, pybuf, wcslen(pybuf), sizeof(wchar_t), cbfcmpWchar_t);
						if (NULL != psiz)
						{
							*psiz = (size_t)wcsdup(chbuf);
							*lbp++ = (wchar_t *)*psiz;
						} 
					}
					chp = chbuf;
				}
				else
				{
					
					pyp = pybuf;
					*chp++ = ch;
					*chp = 0;
				}
			}
			fclose(fpbase);
		}
		else
			goto Lbl_Failed;
	}
	return ptaBase;
Lbl_Failed:
	if (NULL != ptaBase)
	{
		treDeleteTrieA(ptaBase, sizeof(wchar_t));
	}
	return NULL;
}

void PrintHelp(void)
{
	wprintf(L"[exit|quit|x]\tExit program.\n");
	wprintf(L"[show|s]\tShow buffer.\n");
	wprintf(L"[del|d]\tDelete a character from buffer.\n");
	wprintf(L"[pinyin]\tSelect a character into buffer.\n");
}

int main()
{
	size_t i;
	P_TRIE_A ptaBase;
	wchar_t b[BUFSIZ];
	P_ARRAY_Z pbuf = strCreateArrayZ(1, sizeof(wchar_t));
		
	if (NULL == pbuf)
		return 1;
		
	*(wchar_t *)strLocateItemArrayZ(pbuf, sizeof(wchar_t), 0) = 0;
	
	setlocale(LC_ALL, "");
	ptaBase = GetBase("pybase");
	
	if (NULL != ptaBase)
	{
		while (wprintf(L"? "), fgetws(b, BUFSIZ - 1, stdin))
		{
			/* Commands. */
			if (0 == wcscmp(L"quit\n", b) || 0 == wcscmp(L"exit\n", b) || 0 == wcscmp(L"x\n", b))
			{
				goto Lbl_Quit;
			}
			else if (0 == wcscmp(L"show\n", b) || 0 == wcscmp(L"s\n", b))
			{
				wprintf(L"> %ls\n", (const wchar_t *)(pbuf->pdata));
				b[0] = 0;
			}
			else if (0 == wcscmp(L"del\n", b) || 0 == wcscmp(L"d\n", b))
			{
				if(strLevelArrayZ(pbuf) > 1)
				{
					if (NULL == strResizeBufferedArrayZ(pbuf, sizeof(wchar_t), -1))
					{
						wprintf(L"Buffer crashed!\n");
						pbuf = NULL;
						goto Lbl_Quit;
					}
					*(wchar_t *)strLocateItemArrayZ(pbuf, sizeof(wchar_t), strLevelArrayZ(pbuf) - 1) = 0;
					wprintf(L"> %ls\n", (const wchar_t *)(pbuf->pdata));
					b[0] = 0;
				}
			}
			if (0 == wcscmp(L"help\n", b))
			{
				PrintHelp();
				b[0] = 0;
			}
			else
			{
				int n, m;
				int up, down;
				size_t * psiz = treSearchTrieA(ptaBase, b, wcslen(b) - 1, sizeof(wchar_t), cbfcmpWchar_t);
				if (NULL != psiz)
				{
					wchar_t * psel = (wchar_t *)*psiz, * pp = psel;
					do
					{
						size_t j;
						
						up = down = 0;
						for (j = 1; j <= 9; ++j)
						{
							if (pp - psel >= wcslen(psel) - 1)
								break;
							wprintf(L"%d: %lc\n", j, *pp++);
						}			
						if (pp > psel + 9)
						{
							up = j;
							wprintf(L"%d: -\n", up);
							++j;
						}
						if (pp != psel + wcslen(psel) - 1)
						{
							down = j;
							wprintf(L"%d: +\n", down);
						}
						
						wprintf(L"?? ");
						fgetws(b, BUFSIZ - 1, stdin);
						m = swscanf(b, L"%d", &n);
						
						if (n == up && pp > psel + 9)
						{
							pp -= 18;
						}
						else if (n != down)
						{
							if (NULL == strResizeBufferedArrayZ(pbuf, sizeof(wchar_t), +1))
							{
								wprintf(L"Buffer crashed!\n");
								pbuf = NULL;
								goto Lbl_Quit;
							}
							*(wchar_t *)strLocateItemArrayZ(pbuf, sizeof(wchar_t), strLevelArrayZ(pbuf) - 1) = 0;
							*(wchar_t *)strLocateItemArrayZ(pbuf, sizeof(wchar_t), strLevelArrayZ(pbuf) - 2) = *(pp - j + n);
							wprintf(L"> %ls\n", (const wchar_t *)(pbuf->pdata));
							
							break;
						}
						else if (0 == n)
							break;
					}
					while (EOF != m && 0 != m);
				}
			}
			b[0] = 0;
		}
	Lbl_Quit:
		treDeleteTrieA(ptaBase, sizeof(wchar_t));
	}
	for (i = 0; i < LN_BUF; ++i)
	{
		if (NULL != lnbuf[i])
			free(lnbuf[i]);
		else
			break;
	}
	if (NULL != pbuf)
		strDeleteArrayZ(pbuf);
	return 0;
}

