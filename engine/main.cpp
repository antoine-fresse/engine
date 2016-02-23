#include <precompiled_header.h>

#include <entity/entity.h>
#include <entity/serializer.h>
#include <thread/multitasker.h>
#include <entity/system.h>

#include <imgui_impl.h>

#include <assimp/Importer.hpp>

using namespace std;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}


struct Transform
{
	Transform() : position(0.0f,0.0f) {}
	Transform(float x, float y) : position(x,y) {}
	glm::vec2 position;
};


kth::Multitasker gTasker(4, 25);
kth::EntityManager gEM;



void game_render_and_swap(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	glViewport(0, 0, display_w, display_h);
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();
	glfwSwapBuffers(window);
	
	glfwMakeContextCurrent(nullptr);
}
GLFWwindow* init_gl_context()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return nullptr;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
	glfwMakeContextCurrent(window);
	gl3wInit();

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui_ImplGlfwGL3_CreateDeviceObjects();

	glfwMakeContextCurrent(nullptr);
	return window;
}

void main_loop(GLFWwindow* window)
{
	using namespace std;
	
	const int32 N = 10;
	kth::Entity ents[N];
	for (uint32 i = 0; i < N; ++i)
	{
		ents[i] = gEM.create_empty_entity();
		ents[i].add_component<Transform>(20 + i*10.0f, 20 + i*10.0f);
	}
	auto lt = chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(window))
	{
		auto t = chrono::high_resolution_clock::now();
		chrono::duration<float64> dt = t - lt;
		lt = t;
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		cout << this_thread::get_id() << endl;

		ImGui::Begin("Debug");
		ImGui::Value("dt", (float)(1.0/dt.count()));
		ImGui::Text("Mouse position : %f, %f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		ImGui::End();

		
		// Rendering 
		gTasker.wait_for(gTasker.enqueue(game_render_and_swap, window), 0, true);
	}

	glfwMakeContextCurrent(window);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	gTasker.stop();
}
class A
{
public:
	virtual ~A() = default;
	virtual void draw() { printf("A !\n"); }
};

class B : public A
{
public:
	virtual ~B() = default;
	virtual void draw() override { printf("B !\n"); }
};




int main(int argc, char* argv[])
{
	using namespace std;
	unique_ptr<lua_State, void(*)(lua_State*)> lua(luaL_newstate(), lua_close);
	auto L = lua.get();

	luaL_openlibs(L);
	auto n = luabridge::getGlobalNamespace(L)
	
	.beginClass<A>("A")
		.addFunction("draw", &A::draw)
	.endClass()
	.deriveClass<B, A>("B")
	.endClass();

	B b;
	A& a = b;

	luabridge::push<A&>(L, a);
	lua_setglobal(L, "a");
	
	luaL_dostring(L, "a:draw()");

	/*cout << this_thread::get_id() << endl;
	auto window = init_gl_context();
	if (window)
	{
		main_loop(window);
	}*/

	return 0;
}

