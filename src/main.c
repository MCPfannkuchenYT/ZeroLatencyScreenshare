#include <winsock.h>
#include <windows.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

int run = 1;
int sock;

DWORD WINAPI ScreenGrabber(void* data) {
    // Init gdi
    HDC hdc = GetDC(NULL);
    HDC hDest = CreateCompatibleDC(hdc);
    HBITMAP hbDesktop = CreateCompatibleBitmap( hdc, WIDTH, HEIGHT);
    SelectObject(hDest, hbDesktop);
    BitBlt(hDest, 0, 0, WIDTH, HEIGHT, hdc, 0, 0, SRCCOPY);
    
    // Copy bitmap info
    BITMAPINFO bmpInfo;
	memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hdc, hbDesktop, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

    // Copy image data nonstop
    byte* imageData = malloc(WIDTH*HEIGHT*3);
    while (run) {
        if (!sock)
            continue;
        BitBlt(hDest, 0, 0, WIDTH, HEIGHT, hdc, 0, 0, SRCCOPY);
        GetDIBits(hdc, hbDesktop, 0, bmpInfo.bmiHeader.biHeight, imageData, &bmpInfo, DIB_RGB_COLORS);

        send(sock, imageData, WIDTH*HEIGHT*3, 0);
    }

    // Deinit gdi
    ReleaseDC(NULL, hdc);
    DeleteObject(hbDesktop);
    DeleteDC(hDest);

    return 0;
}

DWORD WINAPI SocketThread(void* data) {

    // setup socket
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);

    SOCKET s, client;
    struct sockaddr_in server, cc;

    s = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    bind(s, (struct sockaddr *)&server, sizeof(server));
    setsockopt(s, SOL_SOCKET, TCP_NODELAY, (const char*) 1, 1);
    listen(s, 3);
    int c = sizeof(struct sockaddr_in);

    while (run) {
        int sc = accept(s, (struct sockaddr *)&cc, &c);
        setsockopt(sc, SOL_SOCKET, TCP_NODELAY, (const char*) 1, 1);
        sock = sc;
    }

}

BOOL WINAPI ExitRun(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        printf("exiting\n");
        run = 0;
    }  
}

int main() {
    SetConsoleCtrlHandler(ExitRun, TRUE);
    HANDLE thread = CreateThread(NULL, 0, ScreenGrabber, NULL, 0, NULL);
    HANDLE thread2 = CreateThread(NULL, 0, SocketThread, NULL, 0, NULL);
    
    while (run) {

    }

    // ffplay -probesize 32 -tune zerolatency -fflags nobuffer -flags low_delay -pixel_format bgr24 -video_size 1920x1080 -framerate 60 -f rawvideo -vf "vflip" tcp://localhost:8888

    return 0;
}