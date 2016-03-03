// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"

#include "HRTimer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	


#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format(_TEXT("%s - Proc. time = %.2f ms"), _TEXT(Titlu), elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCTSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);	\

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_MENU1_INTENSITYGROW, &CDibView::OnMenu1Intensitygrow)
	ON_COMMAND(ID_MENU1_RED, &CDibView::OnMenu1Red)
	ON_COMMAND(ID_MENU1_ONLYREDPARTS, &CDibView::OnMenu1Onlyredparts)
	ON_COMMAND(ID_MENU1_TRYPOINTERS, &CDibView::OnMenu1Trypointers)
	ON_COMMAND(ID_MENU1_RESIZE, &CDibView::OnMenu1Resize)
	ON_COMMAND(ID_MENU1_FISHEYE, &CDibView::OnMenu1Fisheye)
	ON_COMMAND(ID_MENU1_BULLETS, &CDibView::OnMenu1Bullets)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL) hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

void CDibView::OnProcessingParcurgereSimpla() 
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// Makes a grayscale image by equalizing the R, G, B components from the LUT
	for (int k=0;  k < iColors ; k++)
		bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;

	//  Goes through the bitmap pixels and performs their negative	
	for (int i=0;i<dwHeight;i++)
		for (int j=0;j<dwWidth;j++)
		  {	
			lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
	  }

	END_PROCESSING("Negativ imagine");
}

void clamp(int &n) {

	if (n <= 0) n = 0;
	if (n >= 255) n = 255;


}
float intensity = 1.25;
void CDibView::OnMenu1Intensitygrow()
{
	
	BEGIN_PROCESSING();

	
	if (iColors > 0) {

		for (int k = 0; k < iColors; k++) {

			int r = bmiColorsSrc[k].rgbRed *intensity;
			clamp(r);
			bmiColorsDst[k].rgbRed = r;

			int g = bmiColorsSrc[k].rgbGreen *intensity;
			clamp(g);
			bmiColorsDst[k].rgbGreen = g;

			int b = bmiColorsSrc[k].rgbBlue *intensity;
			clamp(b);
			bmiColorsDst[k].rgbBlue = b;

		}
		
	}
	else {
	
		for (int i = 0; i < dwHeight; i++)
			for (int j = 0; j < dwWidth; j++)
			{
				int val = lpSrc[i*w + 3 * j] * intensity;
				clamp(val);
				lpDst[i*w + 3 * j] = val; 
				int val2 = lpSrc[i*w + 3 * j + 1] * intensity;
				clamp(val);
				lpDst[i*w + 3 * j + 1] = val2;
				int val3 = lpSrc[i*w + 3 * j + 2] * intensity;
				clamp(val);
				lpDst[i*w + 3 * j + 2] = val3;
			}
	}
	END_PROCESSING("Intensitate marita");
}



void CDibView::OnMenu1Red()
{

	BEGIN_PROCESSING();
	if (iColors > 0) {
		
			for (int k = 0; k < iColors; k++) {
				int r = bmiColorsSrc[k].rgbRed *intensity;
				clamp(r);
				bmiColorsDst[k].rgbRed = r;
			}
	}
	else {
		
		for (int i = 0; i < dwHeight; i++)
			for (int j = 0; j < dwWidth; j++)
			{
				int val = lpSrc[i*w + 3 * j + 2] * intensity;
				clamp(val);
				lpDst[i*w + 3 * j + 2] = val; 
			}
	}
	END_PROCESSING("Mai rosu");
}


void CDibView::OnMenu1Onlyredparts()
{
	
	BEGIN_PROCESSING();

	
	if (iColors > 0) {

				for (int k = 0; k < iColors; k++) {

					int r = bmiColorsSrc[k].rgbRed;
					if (r < 128) {

						int g = bmiColorsSrc[k].rgbGreen;
						int b = bmiColorsSrc[k].rgbBlue;
						int media = (r + g + b) / 3;
						clamp(media);
						bmiColorsDst[k].rgbRed = media;
						bmiColorsDst[k].rgbGreen = media;
						bmiColorsDst[k].rgbBlue = media;

					}

				}
	}
	else {
		
		for (int i = 0; i < dwHeight; i++)
			for (int j = 0; j < dwWidth; j++)
			{

				int r = lpSrc[i*w + 3 * j + 2];
				if (r < 128) {
					int g = lpSrc[i*w + 3 * j + 1];
					int b = lpSrc[i*w + 3 * j];
					int media = (r + g + b) / 3;
					clamp(media);
					lpDst[i*w + 3 * j] = media;
					lpDst[i*w + 3 * j + 1] = media;
					lpDst[i*w + 3 * j + 2] = media;
				}

			}
	}
	END_PROCESSING("numai rosu");
}
void CDibView::OnMenu1Trypointers() {

}
int inRange(int &n, int jos, int sus) {
	if ((n<jos) || (n>sus)) return 0;
	return 1;
}

double k = 0.5;
//Doar pe bitmpa RGB
void CDibView::OnMenu1Resize()
{
	
	BEGIN_PROCESSING();

	if (iColors > 0) {

		MessageBox(_T("Paleta"), _T("MEsaj"), 0);
		return;
	}
	else {

		for (int i = 0; i < dwHeight; i++)
			for (int j = 0; j < dwWidth; j++)
			{
				int h = dwHeight;
				int wi = dwWidth;

				int is = (i - h / 2)*k + h / 2;
				int js = (j - wi / 2)*k + wi / 2;





				if (inRange(is,0,h) && inRange(js,0,wi))

				{
					int r = lpSrc[is*w + 3 * js + 2];
					int g = lpSrc[is*w + 3 * js + 1];
					int b = lpSrc[is*w + 3 * js];
					lpDst[i*w + 3 * j] = b;
					lpDst[i*w + 3 * j + 1] = g;
					lpDst[i*w + 3 * j + 2] = r;
				}
				else
				{
					lpDst[i*w + 3 * j] = 0;
					lpDst[i*w + 3 * j + 1] = 0;
					lpDst[i*w + 3 * j + 2] = 0;
				}


			}
	}
	END_PROCESSING("resize");
}
double k0 = 0.1;
double dk = 0.005;

double make_k(int dist) {
	return k0 + dk*dist;
}

void CDibView::OnMenu1Fisheye()
{
	
	BEGIN_PROCESSING();

	
	if (iColors > 0) {

		MessageBox(_T("Paleta"), _T("MEsaj"), 0);
		return;
	}
	else {

		for (int i = 0; i < dwHeight; i++)
			for (int j = 0; j < dwWidth; j++)
			{
				int h = dwHeight;
				int wi = dwWidth;

				double dist = sqrt((double)(i - h / 2)*(i - h / 2) + (j - wi / 2)*(j - wi / 2));
				int is = (i - h / 2)*make_k(dist) + h / 2;
				int js = (j - wi / 2)*make_k(dist) + wi / 2;





				if (inRange(is, 0, h) && (inRange(js, 0, wi)))

				{
					int r = lpSrc[is*w + 3 * js + 2];
					int g = lpSrc[is*w + 3 * js + 1];
					int b = lpSrc[is*w + 3 * js];
					lpDst[i*w + 3 * j] = b;
					lpDst[i*w + 3 * j + 1] = g;
					lpDst[i*w + 3 * j + 2] = r;
				}
				else
				{
					lpDst[i*w + 3 * j] = 0;
					lpDst[i*w + 3 * j + 1] = 0;
					lpDst[i*w + 3 * j + 2] = 0;
				}


			}
	}
	END_PROCESSING("resize");
}

int radiux = 10;
#define PI 3.14159265

void CDibView::OnMenu1Bullets()
{
	
	BEGIN_PROCESSING();

	
	if (iColors > 0) {

		MessageBox(_T("Paleta"), _T("MEsaj"), 0);
		return;
	}
	else {

		for (int i = radiux; i < dwHeight; i += radiux)
			for (int j = radiux; j < dwWidth; j += radiux)
			{

				for (int teta = 0; teta < 360; teta++) {



					for (int r = 1; r < radiux; r++) {

						int x = i + r*cos(teta*PI / 180);
						int y = j + r*sin(teta*PI / 180);
						if ((inRange(x, 0, dwHeight)) && (inRange(y, 0, dwWidth))) {
							lpDst[x*w + 3 * y] = lpSrc[i*w + 3 * j];
							lpDst[x*w + 3 * y + 1] = lpSrc[i*w + 3 * j + 1];
							lpDst[x*w + 3 * y + 2] = lpSrc[i*w + 3 * j + 2];
						}
					}
				}



			}
	}
	END_PROCESSING("resize");
}
