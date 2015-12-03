#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <entity/entity.h>
#include <entity/serializer.h>
#include <thread/multitasker.h>

#include "entity/system.h"


struct Transform
{
	Transform() : position(0.0f,0.0f) {}
	Transform(float x, float y) : position(x,y) {}
	sf::Vector2f position;
};

struct Sprite2D
{
	Sprite2D() : texture(nullptr), texture_coordinates{{0.0f,0.0f},{ 32.0f,0.0f },{ 0.0f,32.0f },{ 32.0f,32.0f }}, size(0.0f, 0.0f), zOrder(0) {}

	std::shared_ptr<sf::Texture> texture;
	sf::Vector2f texture_coordinates[4];
	sf::Vector2f size;
	int zOrder;
	
};

class SpriteRenderer : public kth::System<Transform, Sprite2D>
{
public:
	SpriteRenderer(kth::EntityManager& manager, std::shared_ptr<sf::RenderTarget> target) : System<Transform, Sprite2D>(manager) , _render_target(target) {}
	~SpriteRenderer() override {};
	void update(std::chrono::duration<double> dt) override;
private:

	std::vector<std::pair<Transform*,Sprite2D*>> _draw_list;
	std::shared_ptr<sf::RenderTarget> _render_target;

	
};

void SpriteRenderer::update(std::chrono::duration<double> dt)
{
	if (!_render_target) return;

	_manager.for_each<Transform, Sprite2D>([&](kth::Entity entity, Transform* transform, Sprite2D* sprite)
	{
		_draw_list.push_back({ transform, sprite });
	});
	
	// Sort by zOrder then by texture (then by pointer address to avoid flickering)
	std::sort(_draw_list.begin(), _draw_list.end(), [](const std::pair<Transform*, Sprite2D*>& a, const std::pair<Transform*, Sprite2D*>& b)
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
		quad[1].position = sprite.first->position + sf::Vector2f(sprite.second->size.x,0.0f);
		quad[2].position = sprite.first->position + sf::Vector2f(0.0f, sprite.second->size.y);
		quad[3].position = sprite.first->position + sprite.second->size;
		

		for (int i = 0; i < 4; ++i)
			quad[i].texCoords = sprite.second->texture_coordinates[i];

		_render_target->draw(quad, sprite.second->texture.get());
	}

}

kth::Multitasker gTasker(4, 25);
kth::EntityManager gEM;

void main_loop()
{
	auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(200, 200), "SFML works!");

	auto gDebug_texture = std::make_shared<sf::Texture>();
	gDebug_texture->loadFromFile("debug_texture.png");

	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	
	const int N = 10;
	kth::Entity ents[N];
	for (int i = 0; i < N; ++i)
	{
		ents[i] = gEM.create_empty_entity();
		ents[i].add_component<Transform>(20 + i*10.0f, 20 + i*10.0f);
		auto sprite = ents[i].add_component<Sprite2D>();
		sprite->size = { 32,32 };
		sprite->texture = gDebug_texture;
		sprite->zOrder = i % 3;
	}


	
	SpriteRenderer renderer(gEM, window);
	auto lt = std::chrono::high_resolution_clock::now();
	while (window->isOpen())
	{
		auto t = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> dt = t - lt;
		lt = t;
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();
		}

		window->clear();
		
		window->draw(shape);
		renderer.update(dt);

		window->display();
	}

	gTasker.stop();
}

int main(int argc, char* argv[])
{
	//kth::Multitasker tasker(1, 25);
	gTasker.enqueue(main_loop);
	gTasker.process_tasks();

	/*std::ofstream file("blueprint.json");
	cereal::JSONOutputArchive ar(file);
	ar(e);

	file.close();*/

	
	return 0;
}
