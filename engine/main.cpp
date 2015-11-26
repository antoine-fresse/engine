#include <SFML/Graphics.hpp>
#include <entity/entity.h>
#include <fstream>
#include <entity/serializer.h>

struct Transform
{
	Transform() : x(0.0f),y(0.0f),z(0.0f){}
	Transform(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;
};

int main(int argc, char* argv[])
{

	EntityManager EM;
	RegisterComponent(Transform, &EM);


	Entity e = EM.create_empty_entity();
	Transform* test = e.add_component<Transform>(1.0f,2.0f,3.0f);

	std::ofstream file("blueprint.json");
	cereal::JSONOutputArchive ar(file);
	ar(e);

	file.close();

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

		shape.setPosition(test->x, test->y);

		window.clear();
		window.draw(shape);
		window.display();
	}
	return 0;
}