// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include "Chain.h"
#include "Commdlg.h"
#include "time.h"

#include <fstream>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// my variables
float rotX = 0;
float rotY = 0;
float rotZ = 0;

int xFirst;
int yFirst;
int xLast;
int yLast;
bool dragged = false;
float tempdx, tempdy;
clock_t lastMoveTime;
clock_t currentMoveTime;
clock_t lastZoomTime;
clock_t currentZoomTime;

float scale = 1.0;

float alpha = 1;

string chainCode = "UUFDD\0";
string tailCode = "\0";
CHAR buff1[1024] = "UUFDD\0";
CHAR buff2[1024] = "\0";
int period = 7;
int mode = ID_MODE_0;

CHAR buff3[1024] = "\0";

Chain chain;
HWND hWnd_for_dialog;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Values(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Keys(HWND, UINT, WPARAM, LPARAM);


// Set up pixel format for graphics initialization
void SetupPixelFormat(HWND hWnd)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int pixelformat;

    ppfd = &pfd;

    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;
    ppfd->iPixelType = PFD_TYPE_COLORINDEX;
    ppfd->cColorBits = 16;
    ppfd->cDepthBits = 16;
    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;

	HDC hDC = GetDC(hWnd);
    pixelformat = ChoosePixelFormat(hDC, ppfd);
    SetPixelFormat(hDC, pixelformat, ppfd);
}


// Initialize OpenGL graphics
void InitGraphics(HWND hWnd)
{
	SetupPixelFormat(hWnd);

	HDC hDC = GetDC(hWnd);
    HGLRC hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    //glClearColor(0, 0, 0, 0.5);
    //glClearDepth(1.0);
    //glEnable(GL_DEPTH_TEST);

	/* Use depth buffering for hidden surface elimination. */
	glEnable(GL_DEPTH_TEST);

	/* Setup the view of the cube. */
	glMatrixMode(GL_PROJECTION);
	gluPerspective( /* field of view in degree */ 40,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 20.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 16.0,  /* eye is at (0,0,5) */
		0.0, 0.0, 0.0,      /* center is at (0,0,0) */
		0.0, 1.0, 0.0);      /* up is in positive Y direction */
}

// Resize graphics to fit window
void ResizeGraphics(HWND hWnd)
{
    // Get new window size
    RECT rect;
	int width, height;
	GLfloat aspect;

    GetClientRect(hWnd, &rect);
    width = rect.right;
    height = rect.bottom;
    aspect = (GLfloat)width / height;

    // Adjust graphics to window size
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspect, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

float make360(float angle){
	if(angle >= 360.0){
		while(angle >= 360.0) angle -= 360.0;
	}
	if(angle < 0){
		while(angle < 360.0) angle += 360.0;
	}
	return angle;
}

// Draw frame
void DrawGraphics(HDC hDC)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();

	glRotatef(make360(rotX + tempdy), 1, 0, 0);
	glRotatef(make360(rotY), 0, 1, 0);
	glRotatef(make360(rotZ + tempdx), 0, 0, 1);

	glScalef(scale, scale, scale);

	switch (mode)
	{
	case ID_MODE_0:		// red green blue
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		chain.setColors(0);
		chain.draw(alpha);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		chain.setColors(3);
		chain.draw(alpha);
		chain.setColors(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case ID_MODE_1:		// wired
		chain.setColors(1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		chain.draw(alpha);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case ID_MODE_2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		chain.setColors(2);
		chain.draw(alpha);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		chain.setColors(3);
		chain.draw(alpha);
		chain.setColors(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}

	glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	SwapBuffers(hDC);
}

// One function for both "Open" and "SaveAs" commands:
BOOL RunFileDialog (HWND hWnd, CHAR* pstrResultPath, BOOL bSaveAs) {
	OPENFILENAME ofn = {0};
	BOOL bRetCode;

	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = hWnd;
	ofn.Flags = OFN_PATHMUSTEXIST;
	ofn.lpstrFile = pstrResultPath;
	ofn.lpstrFilter = "Bitmap Files (*.Bmp)\0*.Bmp\0";
	ofn.lpstrDefExt = "Bmp";
	ofn.nMaxFile = MAX_PATH;
	pstrResultPath [0] = '\0';

	if (bSaveAs) {
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		bRetCode = GetSaveFileName (&ofn);
	}else{
		ofn.Flags |= OFN_FILEMUSTEXIST;
		bRetCode = GetOpenFileName (&ofn);
	}
	return bRetCode;
}

void ScreenShot(CHAR* pstrPath)
{
	FILE *file;
    unsigned long imageSize;
    GLbyte *data=NULL;
    GLint viewPort[4];
    GLenum lastBuffer;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    bmfh.bfType='MB';
    bmfh.bfReserved1=0;
    bmfh.bfReserved2=0;
    bmfh.bfOffBits=54;
    glGetIntegerv(GL_VIEWPORT,viewPort);
    imageSize=((viewPort[2]+((4-(viewPort[2]%4))%4))*viewPort[3]*3)+2;
    bmfh.bfSize=imageSize+sizeof(bmfh)+sizeof(bmih);
    data=(GLbyte*)malloc(imageSize);
    glPixelStorei(GL_PACK_ALIGNMENT,4);
    glPixelStorei(GL_PACK_ROW_LENGTH,0);
    glPixelStorei(GL_PACK_SKIP_ROWS,0);
    glPixelStorei(GL_PACK_SKIP_PIXELS,0);
    glPixelStorei(GL_PACK_SWAP_BYTES,1);
    glGetIntegerv(GL_READ_BUFFER,(GLint*)&lastBuffer);
    glReadBuffer(GL_FRONT);
	glReadPixels(0,0,viewPort[2],viewPort[3],GL_BGR_EXT,GL_UNSIGNED_BYTE,data);
    data[imageSize-1]=0;
    data[imageSize-2]=0;
    glReadBuffer(lastBuffer);
    file=fopen(pstrPath,"wb");
    bmih.biSize=40;
    bmih.biWidth=viewPort[2];
    bmih.biHeight=viewPort[3];
    bmih.biPlanes=1;
    bmih.biBitCount=24;
    bmih.biCompression=0;
    bmih.biSizeImage=imageSize;
    bmih.biXPelsPerMeter=45089;
    bmih.biYPelsPerMeter=45089;
    bmih.biClrUsed=0;
    bmih.biClrImportant=0;
    fwrite(&bmfh,sizeof(bmfh),1,file);
    fwrite(&bmih,sizeof(bmih),1,file);
    fwrite(data,imageSize,1,file);
    free(data);
    fclose(file);
}


// Drawing operations:
VOID WmPaint (HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint (hWnd, &ps);
	DrawGraphics(hDC);
	EndPaint (hWnd, &ps);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)){
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)){
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32PROJECT1);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   // Initialize OpenGL
   InitGraphics(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	HMENU hMenu;
	short zDelta;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_ROTATEYPLUS:
			rotY += 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ROTATEYMINUS:
			rotY -= 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ROTATEXPLUS:
			rotX += 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ROTATEXMINUS:
			rotX -= 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ROTATEZPLUS:
			rotZ += 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ROTATEZMINUS:
			rotZ -= 10;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_SCALEPLUS:
			scale *= 1.1f;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_SCALEMINUS:
			scale /= 1.1f;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ALPHA_PLUS:
			alpha += 0.05f;
			if(alpha > 1 )	alpha = 1;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_ALPHA_MINUS:
			alpha -= 0.2f;
			if(alpha < 0 )	alpha = 0;
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;


		case ID_ACTION_VALUES:
			hWnd_for_dialog = hWnd;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CONTROLBOX), NULL, Values);
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_MODE_0:
			mode = ID_MODE_0;
			hMenu = GetMenu(hWnd);
			CheckMenuItem(hMenu, ID_MODE_0, MF_CHECKED);
			CheckMenuItem(hMenu, ID_MODE_1, MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_MODE_2, MF_UNCHECKED);
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_MODE_1:
			mode = ID_MODE_1;
			hMenu = GetMenu(hWnd);
			CheckMenuItem(hMenu, ID_MODE_0, MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_MODE_1, MF_CHECKED);
			CheckMenuItem(hMenu, ID_MODE_2, MF_UNCHECKED);
			InvalidateRect (hWnd, NULL, TRUE);
			UpdateWindow (hWnd);
			break;
		case ID_MODE_2:
			mode = ID_MODE_2;
			hMenu = GetMenu(hWnd);
			CheckMenuItem(hMenu, ID_MODE_0, MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_MODE_1, MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_MODE_2, MF_CHECKED);
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_HELP_KEYS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_KEYS), hWnd, About);
			break;
		case ID_FILE_OPEN:
			if (RunFileDialog (hWnd, buff3, FALSE)) {
			}
			break;
		case ID_FILE_SAVE:
			if(RunFileDialog(hWnd, buff3, true)){
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
				ScreenShot(buff3);
			}
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_MOUSEWHEEL:
		zDelta = GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA;
		if(zDelta>0){
			scale *= 1.1f;
		}else{
			scale /= 1.1f;
		}
		currentZoomTime = clock();
		if(currentZoomTime-lastZoomTime > 50){
			lastZoomTime = currentZoomTime;
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		xFirst = LOWORD(lParam);
		yFirst = HIWORD(lParam);
		dragged = true;
		lastMoveTime = clock();
		break;
	case WM_MOUSEMOVE:
		if(dragged){
			xLast = LOWORD(lParam);
			yLast = HIWORD(lParam);
			tempdx = (xLast - xFirst)/3.0f;
			tempdy = (yLast - yFirst)/3.0f;
			currentMoveTime = clock();
			if(currentMoveTime-lastMoveTime > 50){
				lastMoveTime = currentMoveTime;
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		rotZ += tempdx;
		tempdx = 0;
		rotX += tempdy;
		tempdy = 0;
		dragged = false;
		InvalidateRect (hWnd, NULL, TRUE);
		UpdateWindow (hWnd);
		break;
	case WM_SIZE:
		ResizeGraphics(hWnd);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_PAINT:
		WmPaint(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Keys(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Values(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, buff1);
		SetDlgItemText(hDlg, IDC_EDIT3, buff2);
		SetDlgItemInt(hDlg, IDC_EDIT2, period, false);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			
			strncpy_s(buff1, "FFDFFUDUDUDUFDUFF\0", 1024);
			chainCode = buff1;
			strncpy_s(buff2, "\0", 1024);
			tailCode = buff2;
			period = 1;
			SetDlgItemText(hDlg, IDC_EDIT1, buff1);
			SetDlgItemText(hDlg, IDC_EDIT3, buff2);
			SetDlgItemInt(hDlg, IDC_EDIT2, period, false);
			break;
		case IDC_BUTTON2:
			strncpy_s(buff1, "UUFDD", 1024);
			chainCode = buff1;
			strncpy_s(buff2, "", 1024);
			tailCode = buff2;
			period = 7;
			SetDlgItemText(hDlg, IDC_EDIT1, buff1);
			SetDlgItemText(hDlg, IDC_EDIT3, buff2);
			SetDlgItemInt(hDlg, IDC_EDIT2, period, false);
			break;
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, buff1, 1024);
			chainCode = buff1;
			GetDlgItemText(hDlg, IDC_EDIT3, buff2, 1024);
			tailCode = buff2;
			period = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, false);
			chain.reset();
			chain.addTerrahedron(chainCode, period, tailCode);
			//EndDialog(hDlg, LOWORD(wParam));
			InvalidateRect(hWnd_for_dialog, NULL, TRUE);
			UpdateWindow(hWnd_for_dialog);
			return (INT_PTR)TRUE;
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
