//
//	Game Object Model definition
//	Written by karanrak
//

#define TRANSLATE_UNITS 700

class GameObject : public sf::RectangleShape {
public:
	explicit GameObject(const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = true) :
		m_dims(dims), m_pos(pos), RectangleShape(dims) {
		setPosition(m_pos);

	}
	sf::Vector2f getPos() {
		m_pos = getPosition();
		return m_pos;
	}
	void setPos(sf::Vector2f pos) {
		m_pos = pos;
		setPosition(m_pos);
	}
	void setTex(const sf::Texture* texture)
	{
		setTexture(texture);
		setTextureRect(sf::IntRect(0, 0, 4000, 1000));
	}
private:
	sf::Vector2f m_dims;
	sf::Vector2f m_pos;
};

class Scrollable : public GameObject {
public:
	explicit Scrollable(const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = true) :
		GameObject(dims, pos) {
		m_screenno = 0;
	}

	int getScreenno() {
		return m_screenno;
	}

	void setScreenno(int screenno) {
		m_screenno = screenno;
		if (m_screenno) {
			move(sf::Vector2f(-(TRANSLATE_UNITS) * 1.f, 0.f));
		}
		else {
			move(sf::Vector2f((TRANSLATE_UNITS) * 1.f, 0.f));
		}
	}

private:
	int m_screenno;

};


class Visible : public Scrollable {
public:
	explicit Visible(const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = true) :
		m_visible(visible), Scrollable(dims, pos) {

	}
	void setVisible(bool visible) {
		m_visible = visible;
	}
	bool getVisible() {
		return m_visible;
	}

private:
	bool m_visible;
};

class Moveable : public Visible {
public:
	explicit Moveable(const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = true) :
		Visible(dims, pos, visible) {

	}
	void Move(sf::Vector2f vec, int units) {
		move(sf::Vector2f(units * vec.x, units * vec.y));
	}
private:

};

class Colorable : public Moveable {
public:
	explicit Colorable(const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = true, sf::Color color = sf::Color::Yellow) :
		m_color(color), Moveable(dims, pos, visible) {
		setFillColor(m_color);
	}
	sf::Color getColor() {
		return m_color;
	}

	void setColor(sf::Color color) {
		m_color = color;
		setFillColor(m_color);
	}

private:
	sf::Color m_color;
};



class Platform : public Visible {
public:
	explicit Platform(const sf::Vector2f dims = sf::Vector2f(150.f, 20.f), sf::Vector2f pos = sf::Vector2f(0.f, 500.f), bool visible = true) :
		Visible(dims, pos, visible) {

	}

	sf::Vector2f getDims()
	{
		return m_dims;
	}


private:
	sf::Vector2f m_dims;
};


class Character : public Colorable {
public:
	explicit Character(int id = 0, const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(10.f, 0.f), bool visible = false, sf::Color color = sf::Color::Yellow) :
		m_id(id), m_screenno(0), Colorable(dims, pos, visible, color)
	{

	}
	int getId() {
		return m_id;
	}
	void setId(int id) {
		m_id = id;
	}

	int getScreenno() {
		return m_screenno;
	}

	void setScreenno(int screenno) {	
		m_screenno = screenno;
		if (m_screenno) {
			move(sf::Vector2f(-(TRANSLATE_UNITS-80) * 1.f, 0.f));
		}
		else {
			move(sf::Vector2f((TRANSLATE_UNITS - 80) * 1.f, 0.f));
		}
	}

private:
	int m_screenno;
	int m_id;
};

class MovingPlatform : public Moveable {
public:
	explicit MovingPlatform(const sf::Vector2f dims = sf::Vector2f(150.f, 20.f), sf::Vector2f pos = sf::Vector2f(300.f, 300.f), bool visible = true) :
		Moveable(dims, pos, visible) {

	}

private:
};

class SpawnPoint : public Scrollable {
public:
	explicit SpawnPoint(const sf::Vector2f dims = sf::Vector2f(5.f, 5.f), sf::Vector2f pos = sf::Vector2f(100.f, 0.f)) :
		Scrollable(dims, pos) {

	}
private:

};


class DeathZone : public Scrollable {
public:
	explicit DeathZone(const sf::Vector2f dims = sf::Vector2f(5.f, 5.f), sf::Vector2f pos = sf::Vector2f(400.f, 400.f)) :
		Scrollable(dims, pos) {

	}
private:

};

class SideBoundary : public Scrollable {
public:
	explicit SideBoundary(const sf::Vector2f dims = sf::Vector2f(10.f, 600.f), sf::Vector2f pos = sf::Vector2f(700.f, 0.f)) :
		m_screenno(0),Scrollable(dims, pos) {
		setFillColor(sf::Color::Blue);
	}

private:
	int m_screenno;
};