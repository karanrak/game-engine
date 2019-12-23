
class Platform : public sf::RectangleShape {
public:
	explicit Platform(const sf::Vector2f& dims) :
		m_dims(dims), RectangleShape(dims)
	{
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
	explicit Character(const sf::Vector2f& dims = sf::Vector2f(50.f, 100.f), int id = 0, bool visible = false) :
		m_id(id), isVisible(visible), m_dims(dims), RectangleShape(dims)
	{
		setPosition(sf::Vector2f(10.f, 0.f));
		setFillColor(sf::Color::Yellow);
		update();
	}

	bool getVisibility()
	{
		return isVisible;
	}
	int getId() {
		return m_id;
	}

	void setVisibility(bool s) {
		isVisible = s;
	}
	void setId(int id) {
		m_id = id;
	}

private:
	bool isVisible;
	int m_id;
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
	void resize(const sf::Vector2f& dims)
	{
		m_dims = dims;
		setSize(m_dims);
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