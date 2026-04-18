#pragma once
#include <SFML/System/Vector2.hpp>

enum class RoomType {
	Normal,
	Storage,
	Control,
	Danger
};

struct Room {
	int x, y;
	int w, h;

	RoomType type = RoomType::Normal;

	sf::Vector2i center() const {
		return { x + w / 2, y + h / 2 };
	}

	bool overlaps(const Room& other) const {
		return x - 1 < other.x + other.w &&
			   x + w + 1 > other.x		 &&
			   y - 1 < other.y + other.h &&
			   y + h + 1 > other.y;
	}

	bool contains(int tx, int ty) const {
		return tx >= x && tx < x + w &&
			   ty >= y && ty < y + h;
	}

};