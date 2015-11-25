#include <SFML/Graphics.hpp>
#include <entity/entity.h>

struct Transform
{
	float x;
	float y;
	float z;
};

int main(int argc, char* argv[])
{

	EntityManager EM;
	RegisterComponent(Transform, &EM);


	Entity e = EM.create_entity();
	e.add_component<Transform>();

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
	return 0;
}