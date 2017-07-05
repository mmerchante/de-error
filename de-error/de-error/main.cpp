/// A very simple tool to compute the error of distance estimators for sphere tracing.
/// For now, this is going to look very shader-like; in the future I'll build something more extendable
#include "common.h"

Float EstimateDistance(const Vector3& p)
{
	return 1.0;
}

Color Raymarch(const Vector2& uv, pcg32& random)
{
	return Color(uv.x, uv.y, 0.0);
}

void RenderBucket(int threadNumber, int bucketHeight, int width, int height, Color * pixels)
{
	int fromY = threadNumber * bucketHeight;
	int toY = glm::min(fromY + bucketHeight, height);

	pcg32 random;
	random.seed(14041956 * threadNumber);

	for (int y = fromY; y < toY; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Vector2 uv = Vector2(x / (Float)width, y / (Float)height);
			pixels[y * width + x] = Raymarch(uv, random) * (threadNumber / 8.0f);
		}
	}
}

void Render(sf::Image & result)
{
	std::chrono::steady_clock::time_point clock = std::chrono::high_resolution_clock::now();
	
	int width = result.getSize().x;
	int height = result.getSize().y;
	Color * pixels = new Color[width * height];

	int threadCount = 8;
	std::vector<std::thread> threads;
	int bucketHeight = height / threadCount;

	for (int i = 0; i < threadCount; i++)
		threads.push_back(std::thread(RenderBucket, i, bucketHeight, width, height, pixels));

	for (int i = 0; i < threadCount; i++)
		threads[i].join();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Color c = pixels[y * width + x];
			c = glm::clamp(c, Color(0.0), Color(1.0));

			int r = c.x * 255;
			int g = c.y * 255;
			int b = c.z * 255;

			sf::Color outColor = sf::Color(r, g, b);
			result.setPixel(x, y, outColor);
		}
	}

	double delta = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - clock).count() / 1000.0;
	std::cout << "Render time: " << std::fixed << std::setprecision(3) << delta  << "ms" << std::endl;
}

void main()
{
	int width = 512;
	int height = 512;
	sf::RenderWindow window(sf::VideoMode(width, height), "DE Error Estimation Tool");

	sf::Image result;
	result.create(width, height, sf::Color::Black);
	Render(result);

	sf::Texture tx;
	tx.loadFromImage(result);

	sf::RectangleShape background(sf::Vector2f(width, height));
	background.setTexture(&tx);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(background);
		window.display();
	}

}