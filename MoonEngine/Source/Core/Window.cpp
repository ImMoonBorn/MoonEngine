#include "mpch.h"
#include "Core/Window.h"

#include "Core/Application.h"
#include "Core/Input.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace MoonEngine
{
	bool Window::Init()
	{
		if (!glfwInit())
			return false;

		WindowPrefs& prefs = Application::GetApp()->GetPrefs().Window;

		m_Window = glfwCreateWindow(prefs.Resolution.Width, prefs.Resolution.Height, Application::GetPrefs().AppName, prefs.Fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
		if (!m_Window)
		{
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(m_Window);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		float xscale, yscale;
		glfwGetWindowContentScale(m_Window, &xscale, &yscale);
		prefs.Dpi = xscale;

		BindWindowCallbacks();

		if (prefs.MaximizeOnStart)
			glfwMaximizeWindow(m_Window);

		glfwSwapInterval(prefs.VsyncOn);

		return true;
	}

	void Window::BindWindowCallbacks()
	{
		glfwSetWindowCloseCallback(m_Window, [](auto window)
		{
			Application* app = Application::GetApp();
			app->Quit();
		});

		glfwSetWindowSizeCallback(m_Window, [](auto window, int width, int height)
		{
			WindowPrefs& prefs = Application::GetWindowPrefs();
			prefs.Resolution.Width = (uint32_t)width;
			prefs.Resolution.Height = (uint32_t)height;
		});

		glfwSetFramebufferSizeCallback(m_Window, [](auto window, int width, int height)
		{
			glViewport(0, 0, width, height);
		});

		glfwSetWindowContentScaleCallback(m_Window, [](GLFWwindow* window, float xscale, float yscale)
		{
			OnDpiChange.Invoke(xscale);
			WindowPrefs& prefs = Application::GetWindowPrefs();
			float scale = yscale > xscale ? yscale : xscale;
			prefs.Dpi = scale;
		});

		glfwSetWindowMaximizeCallback(m_Window, [](auto window, int maximized)
		{
			WindowPrefs& prefs = Application::GetWindowPrefs();
			prefs.MaximizeOnStart = maximized;
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::OnMouseScroll.Invoke(static_cast<float>(yoffset));
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressEvent e((Keycode)key, false);
					Input::OnKeyPress.Invoke(e);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressEvent e((Keycode)key, true);
					Input::OnKeyPress.Invoke(e);
					break;
				}
			}
		});
	}

	void Window::Update()
	{
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	void Window::Terminate()
	{
		glfwTerminate();
	}

	void Window::SetVsync(bool state)
	{
		glfwSwapInterval(state);
	}
}