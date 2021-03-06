#include "Editor/EditorLayer.h"
using namespace MoonEngine;

#ifdef ENGINE_DEBUG
int main()
{
	Application* app = new Application();
	app->PushLayer(new EditorLayer());
	app->Run();
	delete app;
}
#endif // ENGINE_DEBUG

#ifdef  ENGINE_RELEASE
#include <wtypes.h>
int main()
{
	Application* app = new Application();
	app->PushLayer(new EditorLayer());
	app->Run();
	delete app;
}
#endif //  ENGINE_RELEASE

#ifdef ENGINE_FINAL

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	Application* app = new Application();
	app->PushLayer(new EditorLayer());
	app->Run();
	delete app;
}
#endif //  ENGINE_FINAL
