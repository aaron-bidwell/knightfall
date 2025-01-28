#include <windows.h>

// Window Procedure (handles events like close, resize, etc.)
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);  // Tells the message loop to exit
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

// Entry point for a Windows app
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Define a window class
    const char CLASS_NAME[] = "MyWindowClass";

    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;    // Redraw on horizontal/vertical movement
    wc.lpfnWndProc   = WndProc;                    // Our window procedure
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    // 2. Register the window class
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    // 3. Create the window
    HWND hwnd = CreateWindowEx(
        0,                  // Optional window styles
        CLASS_NAME,         // Window class name
        "Hello Window",     // Window title
        WS_OVERLAPPEDWINDOW,// Window style (a typical resizable window)
        CW_USEDEFAULT, CW_USEDEFAULT,  // Position (x, y)
        800, 600,           // Width, Height
        NULL,               // Parent window
        NULL,               // Menu
        hInstance,          // Instance handle
        NULL                // Additional application data
    );

    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    // 4. Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 5. The message loop
    MSG msg = {};
    while (true) {
        // If there's a message to process, process it.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // **Game/render loop** could run here (when no new messages).
            // e.g., Update, Render, etc.
        }
    }

    return static_cast<int>(msg.wParam);
}
