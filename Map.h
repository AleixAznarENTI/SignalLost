#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Room.h"
#include "Battery.h"

enum class TileType {
	Wall,
	Floor
};

class Map
{

public:
	Map(int width, int height);
	void generate(int roomAttempts = 30);

	TileType	 getTile(int x, int y) const;
	int			 getWidth()			   const { return m_width; }
	int			 getHeight()		   const { return m_height; }
	sf::Vector2i getStartPosition()    const { return m_startPosition; }
	sf::Vector2i getSignalPosition()   const { return m_signalPosition; }
	
	const std::vector<Battery>& getBatteries() const { return m_batteries; }
	const std::vector<Room>&    getRooms()	   const { return m_rooms; }

	RoomType getRoomTypeAt(int tx, int ty) const;

private:
	int m_width;
	int m_height;


	std::vector<std::vector<TileType>> m_grid;
	std::vector<Room>				   m_rooms;
	std::vector<Battery>			   m_batteries;

	sf::Vector2i m_startPosition;
	sf::Vector2i m_signalPosition;

	void fillWithWalls();
	void carveRoom(const Room& room);
	void carveCorridor(sf::Vector2i a, sf::Vector2i b);
	void assignRoomTypes();
	void placeBatteries(float tileSize);
	void placeSignal();
};