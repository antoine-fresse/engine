#include <precompiled_header.h>

#include <entity/entity.h>
#include <entity/serializer.h>
#include <thread/multitasker.h>
#include <entity/system.h>

#include <imgui_impl.h>

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

/*struct Sprite2D
{
	Sprite2D() : texture(nullptr), texture_coordinates{{0.0f,0.0f},{ 32.0f,0.0f },{ 0.0f,32.0f },{ 32.0f,32.0f }}, size(0.0f, 0.0f), zOrder(0) {}

	shared_ptr<sf::Texture> texture;
	glm::vec2 texture_coordinates[4];
	glm::vec2 size;
	int zOrder;
	
};

class SpriteRenderer : public kth::System<Transform, Sprite2D>
{
public:
	SpriteRenderer(kth::EntityManager& manager, shared_ptr<sf::RenderTarget> target) : System<Transform, Sprite2D>(manager) , _render_target(target) {}
	~SpriteRenderer() override {};
	void update(chrono::duration<double> dt) override;
private:

	vector<pair<Transform*,Sprite2D*>> _draw_list;
	shared_ptr<sf::RenderTarget> _render_target;

	
};

void SpriteRenderer::update(chrono::duration<double> dt)
{
	if (!_render_target) return;

	_manager.for_each<Transform, Sprite2D>([&](kth::Entity entity, Transform* transform, Sprite2D* sprite)
	{
		_draw_list.push_back({ transform, sprite });
	});
	
	// Sort by zOrder then by texture (then by pointer address to avoid flickering)
	sort(_draw_list.begin(), _draw_list.end(), [](const pair<Transform*, Sprite2D*>& a, const pair<Transform*, Sprite2D*>& b)
	{
		if (a.second->zOrder == b.second->zOrder)
		{
			if (a.second->texture == b.second->texture) return a < b;
			return a.second->texture < b.second->texture;
		}	
		return a.second->zOrder < b.second->zOrder;
	});

	sf::VertexArray quad(sf::TrianglesStrip, 4);
	
	for(auto&& sprite : _draw_list)
	{
		quad[0].position = sprite.first->position;
		quad[1].position = sprite.first->position + glm::vec2(sprite.second->size.x,0.0f);
		quad[2].position = sprite.first->position + glm::vec2(0.0f, sprite.second->size.y);
		quad[3].position = sprite.first->position + sprite.second->size;
		

		for (int i = 0; i < 4; ++i)
			quad[i].texCoords = sprite.second->texture_coordinates[i];

		_render_target->draw(quad, sprite.second->texture.get());
	}

}*/

kth::Multitasker gTasker(4, 25);
kth::EntityManager gEM;

int super_func(int val)
{
	return val * 10;
}

template<typename ReturnType, typename ... ArgsTypes>
ReturnType call_func(void* f, ArgsTypes&& ... args)
{
	std::function<ReturnType(ArgsTypes...)> wrap((ReturnType(*)(ArgsTypes...))f);
	return wrap(std::forward<ArgsTypes>(args)...);
}

using GenericEventCallback = std::function<int(void*)>;

void main_loop()
{
	using namespace std;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
	glfwMakeContextCurrent(window);
	gl3wInit();

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);
	
	map<string, GenericEventCallback> funcs;
	funcs["test"] = [](void * param) { return super_func((int)param); };
	
	cout << "Return :" << funcs["test"]((void*)5) << endl;
	
	
	const int N = 10;
	kth::Entity ents[N];
	for (int i = 0; i < N; ++i)
	{
		ents[i] = gEM.create_empty_entity();
		ents[i].add_component<Transform>(20 + i*10.0f, 20 + i*10.0f);
	}
	auto lt = chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		auto t = chrono::high_resolution_clock::now();
		chrono::duration<double> dt = t - lt;
		lt = t;
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		

		ImGui::Begin("Test");

		ImGui::Value("dt", (float)(1.0/dt.count()));

		ImGui::End();


		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		glfwSwapBuffers(window);
	}
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	gTasker.stop();
}

int main(int argc, char* argv[])
{
	gTasker.enqueue(main_loop);
	// Process tasks on the "main" thread
	gTasker.process_tasks(); 
	return 0;
}
