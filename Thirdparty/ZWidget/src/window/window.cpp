
#include "window/window.h"
#include <stdexcept>

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner)
{
	return DisplayBackend::Get()->Create(windowHost, popupWindow, owner);
}

void DisplayWindow::ProcessEvents()
{
	DisplayBackend::Get()->ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	DisplayBackend::Get()->RunLoop();
}

void DisplayWindow::ExitLoop()
{
	DisplayBackend::Get()->ExitLoop();
}

void* DisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return DisplayBackend::Get()->StartTimer(timeoutMilliseconds, onTimer);
}

void DisplayWindow::StopTimer(void* timerID)
{
	DisplayBackend::Get()->StopTimer(timerID);
}

Size DisplayWindow::GetScreenSize()
{
	return DisplayBackend::Get()->GetScreenSize();
}

/////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<DisplayBackend>& GetBackendVar()
{
	// In C++, static variables in functions are constructed on first encounter and is destructed in the reverse order when main() ends.
	static std::unique_ptr<DisplayBackend> p;
	return p;
}

DisplayBackend* DisplayBackend::Get()
{
	return GetBackendVar().get();
}

void DisplayBackend::Set(std::unique_ptr<DisplayBackend> instance)
{
	GetBackendVar() = std::move(instance);
}

#ifdef WIN32

#include "win32/win32displaywindow.h"

class Win32DisplayBackend : public DisplayBackend
{
public:
	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;
};

std::unique_ptr<DisplayWindow> Win32DisplayBackend::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner)
{
	return std::make_unique<Win32DisplayWindow>(windowHost, popupWindow, static_cast<Win32DisplayWindow*>(owner));
}

void Win32DisplayBackend::ProcessEvents()
{
	Win32DisplayWindow::ProcessEvents();
}

void Win32DisplayBackend::RunLoop()
{
	Win32DisplayWindow::RunLoop();
}

void Win32DisplayBackend::ExitLoop()
{
	Win32DisplayWindow::ExitLoop();
}

Size Win32DisplayBackend::GetScreenSize()
{
	return Win32DisplayWindow::GetScreenSize();
}

void* Win32DisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return Win32DisplayWindow::StartTimer(timeoutMilliseconds, std::move(onTimer));
}

void Win32DisplayBackend::StopTimer(void* timerID)
{
	Win32DisplayWindow::StopTimer(timerID);
}

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWin32()
{
	return std::make_unique<Win32DisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWin32()
{
	return nullptr;
}

#endif

#ifdef USE_SDL2

#include "sdl2/sdl2displaywindow.h"

class SDL2DisplayBackend : public DisplayBackend
{
public:
	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;
};

std::unique_ptr<DisplayWindow> SDL2DisplayBackend::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner)
{
	return std::make_unique<SDL2DisplayWindow>(windowHost, popupWindow, static_cast<SDL2DisplayWindow*>(owner));
}

void SDL2DisplayBackend::ProcessEvents()
{
	SDL2DisplayWindow::ProcessEvents();
}

void SDL2DisplayBackend::RunLoop()
{
	SDL2DisplayWindow::RunLoop();
}

void SDL2DisplayBackend::ExitLoop()
{
	SDL2DisplayWindow::ExitLoop();
}

Size SDL2DisplayBackend::GetScreenSize()
{
	return SDL2DisplayWindow::GetScreenSize();
}

void* SDL2DisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return SDL2DisplayWindow::StartTimer(timeoutMilliseconds, std::move(onTimer));
}

void SDL2DisplayBackend::StopTimer(void* timerID)
{
	SDL2DisplayWindow::StopTimer(timerID);
}

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateSDL2()
{
	return std::make_unique<SDL2DisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateSDL2()
{
	return nullptr;
}

#endif