#include "entity.h"
#include "olcPixelGameEngine.h"

// -------------------------------------------------------------
void Player::DrawSelf(olc::PixelGameEngine *pge, Size size, int direction, olc::Decal* sprite)
{
	// pge->FillRect(x * size.w, y * size.h,
	// 			  size.w, size.h, olc::GREEN);

	olc::vf2d temp_sprite;
	switch(direction)
	{
	case NORTH:
		temp_sprite.x =  north_sprite.x;
		temp_sprite.y =  north_sprite.y;
		break;
	case EAST:
		temp_sprite.x =  east_sprite.x;
		temp_sprite.y =  east_sprite.y;
		break;
	case SOUTH:
		temp_sprite.x =  south_sprite.x;
		temp_sprite.y =  south_sprite.y;
		break;
	case WEST:
		temp_sprite.x =  west_sprite.x;
		temp_sprite.y =  west_sprite.y;
		break;
	}

	olc::vf2d pl_pos = { (float)(x * size.w), (float)(y * size.h) };
	olc::vf2d sizef = { (float)size.w, (float)size.h };
	pge->DrawPartialDecal(pl_pos, sprite, temp_sprite, sizef);
}

// -------------------------------------------------------------
Wall::Wall(int x, int y) : x{x}, y{y} {}

void Wall::DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite)
{
	// pge->FillRect(x * size.w, y * size.h,
	// 			  size.w, size.h, olc::VERY_DARK_BLUE);
	olc::vf2d pl_pos = { (float)(x * size.w), (float)(y * size.h) };
	olc::vf2d sizef = { (float)size.w, (float)size.h };
	pge->DrawPartialDecal(pl_pos, sprite, wall_sprite, sizef);
}

// -------------------------------------------------------------
Bloc::Bloc(int x, int y) : x{x}, y{y} {}

void Bloc::DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite)
{
	// pge->FillRect(x * size.w, y * size.h,
	// 			  size.w, size.h, olc::BLUE);
	olc::vf2d pl_pos = { (float)(x * size.w), (float)(y * size.h) };
	olc::vf2d sizef = { (float)size.w, (float)size.h };
	pge->DrawPartialDecal(pl_pos, sprite, bloc_sprite, sizef);
}

// -------------------------------------------------------------
Winning_Tile::Winning_Tile(int x, int y) : x{x}, y{y} {}

void Winning_Tile::DrawSelf(olc::PixelGameEngine *pge, Size size, olc::Decal* sprite)
{
	// pge->DrawRect(x * size.w, y * size.h,
	// 			  size.w, size.h, olc::RED);
	olc::vf2d pl_pos = { (float)(x * size.w), (float)(y * size.h) };
	olc::vf2d sizef = { (float)size.w, (float)size.h };
	pge->DrawPartialDecal(pl_pos, sprite, winning_tile_sprite, sizef);
}

// -------------------------------------------------------------
void Selector::move(int nx, int ny)
{
	x += nx;
	y += ny;
}

void Selector::place(int nx, int ny)
{
	x = nx;
	y = ny;
}


// -------------------------------------------------------------
void Entity_Manager::reset_all()
{
	walls.clear();
	blocs.clear();
	//winning_tiles.clear();
	wall = 0;
	bloc = 0;
	//tile = 0;
	//pos_player = {0, 0};
}

void Entity_Manager::copy(Entity_Manager to_copy) 
{
	for(int w{0}; w < to_copy.wall; ++w)
		walls.push_back(to_copy.walls[w]);
	for(int b{0}; b < to_copy.bloc; ++b)
		blocs.push_back(to_copy.blocs[b]);
	player.x = to_copy.player.x;
	player.y = to_copy.player.y;
	//for(int t{0}; t < to_copy.tile; ++t)
	//	winning_tiles.push_back(to_copy.winning_tiles[t]);
	wall = to_copy.wall;
	bloc = to_copy.bloc;
	//tile = to_copy.bloc;
	//pos_player = to_copy.pos_player;
}
