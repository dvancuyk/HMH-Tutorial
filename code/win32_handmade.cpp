#include <windows.h>
#include <stdint.h> // Returns back precise types for the machine we're on?
#include <xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct window_dimensions {
	int height;
	int width;
};

struct offscreen_buffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	const int bytesPerPixel = 4;
};

// Declares the expected definition for the XINputGetState function defined within the Xinput.h file
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
// Not sure what this does
typedef X_INPUT_GET_STATE(x_input_get_state);
// Creates a stub method for XInputGetState which simply returns 0;
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return 0;
}

// Create a global pointer pointing at the memory block for the stub on initialization.
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
// Defines the XInputGetState to the pointer declared above
#define XInputGetState XInputGetState_

// Declares the expected definition for the XInputSetState function defined within the Xinput.h file
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
// ?
typedef X_INPUT_SET_STATE(x_input_set_state);

// Creates a stub method for XInputSetState which simply returns 0;
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return 0;
}

global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputSetState XInputSetState_

global_variable bool isRunning = true;
global_variable offscreen_buffer backBuffer;

internal void DefineControllerFuntions()
{
	HMODULE library = LoadLibrary("xinput1_4.dll");
	if (library)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(library, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(library, "XInputSetState");
	}
}

internal window_dimensions GetWindowDimensions(const HWND& window)
{
	window_dimensions dimensions;

	RECT screen;
	GetClientRect(window, &screen);

	dimensions.height = screen.bottom - screen.top;
	dimensions.width = screen.right - screen.left;;

	return dimensions;
}

/*
  Pixel in memory: 00 00 00 00 following the pattern RR GG BB XX where XX is padding.
  LITTLE ENDIAN architecture and MS decisions, the format will be BB RR GG with the last byte as padding
*/
internal void renderGradient(const offscreen_buffer& buffer, int blueOffset, int greenOffset) {
	uint8 *row = (uint8 *)buffer.memory;
	for (int y = 0; y < buffer.height; y++)
	{
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < buffer.width; x++)
		{
			uint8 green = (uint8)(y + greenOffset);
			uint8 blue = (uint8)(x + blueOffset);
			*pixel = (blue | (green << 8));
			++pixel;
		}

		row += buffer.pitch; // Move to the next row
	}
}

// DIB => MSDN: Device Independent Bitmap
//
internal void ResizeDibSection(offscreen_buffer& buffer, int width, int height)
{
	if (buffer.memory) {
		VirtualFree(buffer.memory, 0, MEM_RELEASE);
	}

	buffer.width = width;
	buffer.height = height;

	// NOTE: When the nitHeight is negative, this is the clue that Windows is to treat this bitmap as
	// as top-down, not bottom-up, meaning that the first byte of the images is the top left pixel
	buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader); // Used to determine how much memory to skip to get to the pallate info with MS code
	buffer.info.bmiHeader.biWidth = width;
	buffer.info.bmiHeader.biHeight = -height;
	buffer.info.bmiHeader.biPlanes = 1; // Legacy code... always 1
	buffer.info.bmiHeader.biBitCount = 32; // Number of bits per pixel
	buffer.info.bmiHeader.biCompression = BI_RGB; // No compression
	buffer.pitch = buffer.width * buffer.bytesPerPixel;

	int bitmapMemorySize = (buffer.width * buffer.height)* buffer.bytesPerPixel;

	buffer.memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

// Correlates to Win32UpdateWindow from videos
internal void DisplayBufferInWindow(HDC deviceContext, const window_dimensions& window, offscreen_buffer& buffer)
{
	StretchDIBits(deviceContext,
		0, 0, window.width, window.height,
		0, 0, buffer.width, buffer.height,
		buffer.memory, &buffer.info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (message)
	{
	case WM_DESTROY:
		isRunning = false;
		break;
	case WM_CLOSE:
		isRunning = false;
		break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 virtualKeyCode = wParam;
		bool wasPreviouslyDown = (lParam & (1 << 30)) != 0;
		bool isCurrentlyDown = (lParam & (1 << 31)) == 0;

		if (virtualKeyCode == 'W') {

		}
		else if (virtualKeyCode == 'A') {

		}
		else if (virtualKeyCode == 'S') {

		}
		else if (virtualKeyCode == 'D') {

		}
		else if (virtualKeyCode == 'Q') {

		}
		else if (virtualKeyCode == 'E') {

		}
		else if (virtualKeyCode == VK_UP) {

		}
		else if (virtualKeyCode == VK_LEFT) {

		}
		else if (virtualKeyCode == VK_DOWN) {

		}
		else if (virtualKeyCode == VK_RIGHT) {

		}
		else if (virtualKeyCode == VK_SPACE) {

		}
		else if (virtualKeyCode == VK_ESCAPE) {

		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT painting;
		HDC context = BeginPaint(window, &painting);

		DisplayBufferInWindow(context, GetWindowDimensions(window), backBuffer);

		EndPaint(window, &painting);
	}
	break;
	default:
		result = DefWindowProc(window, message, wParam, lParam);
		break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCode)
{
	WNDCLASS window = {};

	ResizeDibSection(backBuffer, 1280, 720);

	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = MainWindowCallback;
	window.hInstance = instance;
	//window.hIcon;
	window.lpszClassName = "Handmade Hero Take 1";
	DefineControllerFuntions();

	if (RegisterClass(&window))
	{
		HWND handle = CreateWindowEx(0, window.lpszClassName,
			"Handmade Hero", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

		if (handle)
		{
			int blueOffset = 0, greenOffset = 0;
			MSG message;
			BOOL messageResult;
			while (isRunning) {

				// GetMessageA blocks. PeekMessage does not
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {

					if (message.message == WM_QUIT) {
						isRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {
					XINPUT_STATE controllerState;
					if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
						// The controller is plugged in
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
						{
							greenOffset++;
						}
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
						{
							greenOffset--;
						}
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
						{
							blueOffset -= 2;
						}
						if ((pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) || pad->sThumbLX < 0)
						{
							blueOffset++;
						}
						bool leftShoulderPressed = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulderPressed = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool aButtonPressed = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool bButtonPressed = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool xButtonPressed = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool yButtonPressed = (pad->wButtons & XINPUT_GAMEPAD_Y);

					}
					else {
						// Controller not available.
					}

					XINPUT_VIBRATION goodVibrations;
					goodVibrations.wLeftMotorSpeed = 6000;
					goodVibrations.wRightMotorSpeed = 6000;
					XInputSetState(0, &goodVibrations);

				}

				renderGradient(backBuffer, blueOffset++, greenOffset);

				HDC deviceContext = GetDC(handle);
				window_dimensions updateScreen = GetWindowDimensions(handle);
				DisplayBufferInWindow(deviceContext, updateScreen, backBuffer);
				ReleaseDC(handle, deviceContext);
			}
		}
	}

	return 0;
}
