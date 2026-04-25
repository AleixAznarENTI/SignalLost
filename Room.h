#pragma once
#include <SFML/System/Vector2.hpp>

enum class RoomType {
	Normal,
	Storage,
	Control,
	Danger
};

enum class RoomShape {
	Rectangle,
	LShape,
	Pillars
};

struct Room {
	int x, y, w, h;
	RoomType type = RoomType::Normal;
	RoomShape shape = RoomShape::Rectangle;

	// For L-shape: additional extension
	int extX = 0, extY = 0;
	int extW = 0, extH = 0;

	sf::Vector2i center() const { return { x + w / 2, y + h / 2 }; }

	bool overlaps(const Room& other) const {
		return x - 1 < other.x + other.w &&
			x + w + 1 > other.x &&
			y - 1 < other.y + other.h &&
			y + h + 1 > other.y;
	}

	bool contains(int tx, int ty) const {
		bool inMain = tx >= x && tx < x + w && ty >= y && ty < y + h;
		bool inExt = extW > 0 &&
			tx >= extX && tx < extX + extW &&
			ty >= extY && ty < extY + extH;
		return inMain || inExt;
	}

};