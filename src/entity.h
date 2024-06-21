#ifndef ENTITY_H
#define ENTITY_H

#include "olcPixelGameEngine.h"
#include <cstdint>

//olc::Sprite* sokoban_sprite;
//olc::Sprite* sokoban_sprite = new olc::Sprite("data/SokobanClone.png");

enum PUSH_DIRECTION
	{
	 NORTH,
	 EAST,
	 SOUTH,
	 WEST
};

// -------------------------------------------------------------
struct Pos
{
	int x;
	int y;
};

// -------------------------------------------------------------
struct Size
{
	int w;
	int h;
};

struct Selection
{
	Pos pos;
};

// -------------------------------------------------------------
struct Grid
{
	Size size;
	Size unit_size;
};

// -------------------------------------------------------------
struct Screen 
{
	Pos pos;
	Size size;
};

struct Map_Manager 
{
	std::uint8_t ground;
	std::uint8_t entity;

	// @TODO: replace entity_index by entity_ref?
	// using a pointer to point to the entity??
	int entity_index;
};

// -------------------------------------------------------------
struct Player
{
	int x;
	int y;
	int strength { 1 };
	bool is_alive{false};
	olc::vf2d east_sprite { 0.0f, 256.0f };
	olc::vf2d north_sprite { 0.0f, 320.0f };
	olc::vf2d south_sprite { 64.0f, 256.0f };
	olc::vf2d west_sprite { 64.0f, 320.0f };

	// @TODO: no need for menber function here...
	// only color change
	void DrawSelf(olc::PixelGameEngine *pge, Size size, int direction, olc::Decal* sprite);
};

// -------------------------------------------------------------
struct Wall
{
	int x;
	int y;
	olc::vf2d wall_sprite { 384.0f, 0.0f };
	
	Wall(int x, int y);

	
	void DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite);
};

// -------------------------------------------------------------
struct Bloc
{
	int x;
	int y;
	olc::vf2d bloc_sprite { 320.0f, 0.0f };
	
	Bloc(int x, int y);

	void DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite);
};

// -------------------------------------------------------------
struct Winning_Tile
{
	int x;
	int y;
	olc::vf2d winning_tile_sprite { 64.0f, 64.0f };
 
	Winning_Tile(int x, int y);

	void DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite);
};

// -------------------------------------------------------------
struct Entity_Manager
{
	std::vector<Wall> walls;
	std::vector<Bloc> blocs;

	Player player;

	int wall{0};
	int bloc{0};
	
	void reset_all();
	
	void copy(Entity_Manager to_copy);
};	

struct Selector
{
	int x = 0;
	int y = 0;
	bool is_grabbing{false};
	int selected;


	// @TODO: why all member function???
	void place(int nx, int ny);
	
	void move(int nx, int ny);
	//void move(int direction, std::vector<Map_Manager> map_manager);

	void select(int s);
};

enum ELEMENT_TO_SELECT
	{
	 SOLID,
	 WALL,
	 BLOC,
	 WINNING_TILE,
	 PLAYER,

	 MAX_ELEMENT_TO_SELECT
	};

enum GAME_STATE
	{
	 GAME,
	 EDITOR
	};

enum EDITOR_STATE
	{
	 DEFAULT,
	 EDITOR_MENU,
	 SELECTION_MENU,
	};

#endif
