/// A very simple tool to compute the error of distance estimators for sphere tracing.
/// For now, this is going to look very shader-like; in the future I'll build something more extendable
#include "common.h"

Float EstimateDistance(const Vector3& pos)
{
	Vector3 w = pos;
	Float m = glm::dot(w, w);
	Float dz = 1.0;

	Vector3 w1 = w;
	Vector3 w2 = w * w;
	Vector3 w4 = w2 * w2;

	Vector4 k = Vector4(0.0);

	for (int i = 0; i < 5; i++)
	{
		dz = 8.0*glm::sqrt(m)*m*m*m*dz + 1.0;

		w1 = w;
		w2 = w * w;
		w4 = w2 * w2;

		k.x = w2.x + w2.z;
		k.y = 1.0 / (k.x * k.x * k.x * glm::sqrt(k.x));
		k.z = w4.x + w4.y + w4.z - 6.0*w2.y*w2.z - 6.0*w2.x*w2.y + 2.0*w2.z*w2.x;
		k.w = w2.x - w2.y + w2.z;

		w = pos;
		w.x += 64.0*w1.x*w1.y*w1.z*(w2.x - w2.z)*k.w*(w4.x - 6.0*w2.x*w2.z + w4.z)*k.z*k.y;
		w.y += -16.0*w2.y*k.x*k.w*k.w + k.z*k.z;
		w.z += -8.0*w1.y*k.w*(w4.x*w4.x - 28.0*w4.x*w2.x*w2.z + 70.0*w4.x*w4.z - 28.0*w2.x*w2.z*w4.z + w4.z*w4.z)*k.z*k.y;

		m = glm::dot(w, w);

		if (m > 4.0)
			break;
	}

	return 0.25 * glm::log(m) * glm::sqrt(m) / dz;
}

Color Raymarch(const Vector2& uv, pcg32& random)
{
	Float zoom = 2.0;

	// Ortho projection for now
	Vector3 rayOrigin = Vector3((uv.x + 1.0) / zoom, uv.y / zoom, -3.0);
	Vector3 rayDirection = Vector3(0.0, 0.0, 1.0);

	int steps = 0;
	int maxIterations = 50;

	Float distance = 0.0;
	Float totalDistance = 0.0;
	Float eps = .001;

	for (steps = 0; steps < maxIterations; steps++)
	{
		distance = EstimateDistance(rayOrigin + rayDirection * totalDistance);

		if (distance < eps)
			break;

		totalDistance += distance;
	}

	Float r = glm::clamp(steps / (Float)maxIterations, (Float)0.0, (Float)1.0);
	return Color(r);
}

inline
Vector2 RandomVector2(pcg32& random)
{
	return Vector2(random.nextDouble(), random.nextDouble());
}

void RenderBucket(int threadNumber, int bucketHeight, int width, int height, Color * pixels)
{
	int fromY = threadNumber * bucketHeight;
	int toY = glm::min(fromY + bucketHeight, height);

	pcg32 random;
	random.seed(14041956 * threadNumber);

	int aa = 1;
	Float aaFactor = 1.0 / aa;
	Vector2 pixelSize = Vector2(1.0 / (Float)width, 1.0 / (Float)height);

	for (int y = fromY; y < toY; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Vector2 uv = Vector2(x / (Float)width, y / (Float)height) * (Float)2.0 - Vector2(1.0);
			Color result = Color(0.0);

			for (int a = 0; a < aa; a++)
				result += Raymarch(uv + RandomVector2(random) * pixelSize, random) * aaFactor;

			pixels[y * width + x] = result;
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
	int width = 1024;
	int height = 1024;
	sf::RenderWindow window(sf::VideoMode(width, height), "DE Error Estimation Tool");

	sf::Image result;
	result.create(width, height, sf::Color::Black);
	Render(result);

	result.saveToFile("output/output.png");

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