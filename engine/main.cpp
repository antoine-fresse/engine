#include <SFML/Graphics.hpp>
#include <entity/entity.h>
#include <fstream>
#include <entity/serializer.h>
#include <thread/multitasker.h>

struct Transform
{
	Transform() : x(0.0f),y(0.0f),z(0.0f){}
	Transform(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;
};


void main_loop()
{
	sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
	}
}

int main(int argc, char* argv[])
{
	kth::EntityManager EM;
	RegisterComponent(Transform, &EM);


	kth::Entity e = EM.create_empty_entity();
	Transform* test = e.add_component<Transform>(1.0f,2.0f,3.0f);

	kth::Multitasker tasker(4,25);
	tasker.enqueue(main_loop);
	tasker.process_tasks();

	/*std::ofstream file("blueprint.json");
	cereal::JSONOutputArchive ar(file);
	ar(e);

	file.close();*/

	
	return 0;
}
