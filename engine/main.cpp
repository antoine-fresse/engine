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

kth::Multitasker gTasker(4, 25);
kth::EntityManager gEM;

void main_loop()
{
	sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	
	std::function<void()> tasks[10000];
	kth::Entity ents[10000];
	for (int i = 0; i < 10000; ++i)
	{
		ents[i] = gEM.create_empty_entity();
		Transform* test = ents[i].add_component<Transform>(i*1.0f, 2.0f, 3.0f);
	}

	

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		Transform* test = ents[i].get_component<Transform>();
		tasks[i] = [=]()
		{
			float len = sqrt(test->x*test->x + test->y + test->y + test->z + test->z);
			if (len != 0.0f) 
			{
				test->x /= len;
				test->y /= len;
				test->z /= len;
			}
		};
	}
	auto counter = gTasker.enqueue(tasks);
	gTasker.wait_for(counter, 0, true);
	
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - start;

	std::cout << "multitasker : " << diff.count() << "s" << std::endl;

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		Transform* test = ents[i].get_component<Transform>();
		float len = sqrt(test->x*test->x + test->y + test->y + test->z + test->z);
		if (len != 0.0f)
		{
			test->x /= len;
			test->y /= len;
			test->z /= len;
		}
	}
	end = std::chrono::high_resolution_clock::now();
	diff = end - start;

	std::cout << "regular : " << diff.count() << "s" << std::endl;

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

	gTasker.stop();
}

int main(int argc, char* argv[])
{
	
	RegisterComponent(Transform, &gEM);

	
	

	//kth::Multitasker tasker(1, 25);
	gTasker.enqueue(main_loop);
	gTasker.process_tasks();

	/*std::ofstream file("blueprint.json");
	cereal::JSONOutputArchive ar(file);
	ar(e);

	file.close();*/

	
	return 0;
}
