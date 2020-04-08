//
//	Game Object Model definition
//	Written by karanrak
//
#pragma once
using namespace std;

#define TRANSLATE_UNITS 700

enum GameEvents {
	C_SPAWN,
	C_DEATH,
	C_COLLIDE,
	C_MOVE,
	C_SIDEB,
	C_RECSTART,
	C_RECSTOP,
};


class Event {

	int type;
	int timeStamp;
	sf::Vector2f pos;
	int id;
	bool flagPosReq;
	int screenno;

public:

	explicit Event() {

	}
	explicit Event(int Type, int Id, int Timestamp, sf::Vector2f Pos, int Screenno) :
		type(Type), timeStamp(Timestamp), pos(Pos), id(Id), screenno(Screenno) {
		flagPosReq = 1;
	}
	explicit Event(int Type, int Id, int Timestamp, int Screenno) :
		type(Type), timeStamp(Timestamp), id(Id), screenno(Screenno) {
		flagPosReq = 0;
		pos = sf::Vector2f(0.f, 0.f);
	}
	int getType() {
		return type;
	}
	void set_Event(int Type, int Id, int Timestamp, sf::Vector2f Pos, int Screenno) {
		type = Type;
		timeStamp = Timestamp;
		pos = Pos;
		id = Id;
		screenno = Screenno;
		flagPosReq = 1;
	}
	void set_Event(int Type, int Id, int Timestamp, int Screenno) {
		type = Type;
		timeStamp = Timestamp;
		id = Id;
		screenno = Screenno;
		flagPosReq = 0;
	}
	int e_getScreenno() {
		return screenno;
	}
	void e_setScreenno(int mscreenno) {
		screenno = mscreenno;
	}
	sf::Vector2f e_getPos() {
		return pos;
	}
	int e_getId() {
		return id;
	}
	bool e_getFlag() {
		return flagPosReq;
	}
	int e_getTimeStamp() {
		return timeStamp;
	}
};



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
	explicit Character(int id = 0, const sf::Vector2f dims = sf::Vector2f(50.f, 100.f), sf::Vector2f pos = sf::Vector2f(50.f, 50.f), bool visible = true, sf::Color color = sf::Color::Yellow) :
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
			move(sf::Vector2f(-(TRANSLATE_UNITS - 80) * 1.f, 0.f));
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
		m_screenno(0), Scrollable(dims, pos) {
		setFillColor(sf::Color::Blue);
	}


private:
	int m_screenno;
};

class event_handler {
public:
	explicit event_handler() {
		onEventCE.clear();
	}

	void onEvent(Character* c, Event e) {
		for (int i = 0; i < onEventCE.size(); i++) {
			onEventCE[i]->onEvent(c, e);
		}
	}

	vector <event_handler*> onEventCE;
};

class Character_eventhandler : public event_handler {
public:
	void onEvent(Character* c, Event e) {
		switch (e.getType()) {
		case C_COLLIDE:
			//cout << "Handling Collision" << endl;
			break;
		case C_MOVE:
			//cout << "Handling Up" << endl;
			c->move(e.e_getPos());
			break;
		case C_DEATH:
		case C_SPAWN: c->setPosition(e.e_getPos());
			break;
		case C_SIDEB: c->setScreenno(e.e_getScreenno());
			break;
		case C_RECSTART: 
			break;
		case C_RECSTOP:
			break;
		}
	}
};

class RecObject {
public:
	vector<Event> recqueue;
	map<int, sf::Vector2f> r_startpos;// , r_endpos;
	map<int, Character> r_startchar;// , r_endchar;
	int start_time;
	int end_time;
	bool flag;
	bool recPlay;


	void clear() {
		r_startpos.clear();
		//r_endchar.clear();
		r_startchar.clear();
		//r_endpos.clear();
		recqueue.clear();
		start_time = 0;
		end_time = 0;
		flag = 0;
		recPlay = 0;
	}
};