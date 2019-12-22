#include <SFML/Graphics.hpp>

#define MOVT 2500

class Platform : public sf::RectangleShape {
public:
	explicit Platform(const sf::Vector2f& dims) :
		m_dims(dims), RectangleShape(dims)
	{
		//		setSize(m_dims);
		setPosition(sf::Vector2f(0.f, 550.f));
		update();
	}
	void setTex(const sf::Texture* texture)
	{
		setTexture(texture);
		setTextureRect(sf::IntRect(0, 0, 4000, 1000));
		update();
	}
	sf::Vector2f& getDims()
	{
		return m_dims;
	}

private:
	sf::Vector2f m_dims;
};


class Character : public sf::RectangleShape {
public:
	explicit Character(const sf::Vector2f& dims) :
		m_dims(dims), RectangleShape(dims)
	{
		setPosition(sf::Vector2f(0.f, 450.f));
		setFillColor(sf::Color::Blue);
		update();
	}
	sf::Vector2f& getDims()
	{
		return m_dims;
	}

private:
	sf::Vector2f m_dims;
};

class MovingPlatform : public sf::RectangleShape {
public:
	explicit MovingPlatform(const sf::Vector2f& dims) :
		m_dims(dims), RectangleShape(dims)
	{
		setPosition(sf::Vector2f(300.f, 250.f));
		update();
	}
	void setTex(const sf::Texture* texture)
	{
		setTexture(texture);
		setTextureRect(sf::IntRect(0, 0, 4000, 1000));
		update();
	}
	sf::Vector2f& getDims()
	{
		return m_dims;
	}

private:
	sf::Vector2f m_dims;
};



int main()
{
	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	int ticker = 0, flagCheckResize = 0;
	Platform platform(sf::Vector2f(800.f, 50.f));
	Character character(sf::Vector2f(50.f, 100.f));
	MovingPlatform mplatform(sf::Vector2f(150.f, 30.f));
	sf::Texture texture;

	if (!texture.loadFromFile("brick.jpg"))
	{
		cout<<"Error";
		exit(0);
	}
	sf::FloatRect boundingBoxPlatform = platform.getGlobalBounds();
	sf::FloatRect boundingBoxCharacter = character.getGlobalBounds();
	sf::FloatRect boundingBoxmPlatform = mplatform.getGlobalBounds();

	// run the program as long as the window is open
	while (window.isOpen())
	{
		ticker++;
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();

			// catch the resize events
			if (flagCheckResize == 1 && event.type == sf::Event::Resized)
			{
				// update the view to the new size of the window
				sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
			}

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::R) {
					//Toggle between constant size and proportional
					flagCheckResize = (++flagCheckResize) % 2;
				}
			}

		}


		// clear the window with red color
		window.clear(sf::Color::Red);

		//Setting textures for platform and moving platform
		platform.setTex(&texture);
		mplatform.setTex(&texture);

		//Defining the movement of mplatform
		if (ticker < MOVT) {
			mplatform.move(sf::Vector2f(0.1f, 0.f));
		}
		else if (ticker < MOVT * 2) {
			mplatform.move(sf::Vector2f(-0.1f, 0.f));
		}
		ticker %= MOVT * 2;

		//Forming the bounding boxes
		boundingBoxPlatform = platform.getGlobalBounds();
		boundingBoxCharacter = character.getGlobalBounds();
		boundingBoxmPlatform = mplatform.getGlobalBounds();
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
			//Toggle between constant and proportional
			flagCheckResize = (++flagCheckResize) % 2;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			// left key is pressed: move our character
			character.move(-1.f, 0.f);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			// right key is pressed: move our character
			character.move(1.f, 0.f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			// up key is pressed: move our character
			character.move(0.f, -1.f);
		}
		else {
			if (!boundingBoxCharacter.intersects(boundingBoxPlatform) && !boundingBoxCharacter.intersects(boundingBoxmPlatform)) {
				// no key is pressed, emulate falling if character is not on a platform
				character.move(0.f, 1.f);
			}
		}

		window.draw(platform);
		window.draw(character);
		window.draw(mplatform);

		window.display();
	}

	return 0;
}