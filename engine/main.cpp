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

struct Renderable
{
	Renderable() : texture(nullptr){}
	Renderable(void* ptr) : texture(ptr) {}
	void* texture;
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
		if(i<7500)
			ents[i].add_component<Transform>(i*1.0f, 2.0f, 3.0f);

		if(i>=2500)
			ents[i].add_component<Renderable>((void*)i);
	}

	

	auto start = std::chrono::high_resolution_clock::now();
	
	int cnt = 0;
	gEM.for_each<Transform, Renderable>([&](kth::Entity entity, Transform* t, Renderable* r)
	{
		//std::cout << r->texture << " " << t->x << " " << t->y << " " << t->z << std::endl;
		++cnt;
	});
	
	ASSERT(cnt == 5000);
	
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - start;

	std::cout << "count : " << cnt <<" // time : " << diff.count() << "s" << std::endl;

	

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
	RegisterComponent(Renderable, &gEM);

	
	

	//kth::Multitasker tasker(1, 25);
	gTasker.enqueue(main_loop);
	gTasker.process_tasks();

	/*std::ofstream file("blueprint.json");
	cereal::JSONOutputArchive ar(file);
	ar(e);

	file.close();*/

	
	return 0;
}
