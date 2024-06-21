#define OLC_PGE_APPLICATION

// #define SCREEN_WIDTH 256
// #define SCREEN_HEIGHT 240

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 960

#include "src/editor.h"
#include "src/entity.h"


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <bitset>

struct Sokoban : public olc::PixelGameEngine
{
	Sokoban()
	{
		// Name you application
		sAppName = "Sokoban";
	}
	
	// flags for ground byte
	const std::uint8_t is_solid { 0x1 };
	const std::uint8_t is_winning_tile { 0x2 };
	// flags for entity byte	
	const std::uint8_t is_player { 0x1 };
	const std::uint8_t is_bloc { 0x2 };
	const std::uint8_t is_wall { 0x4 };

	const std::uint8_t clear { 0x0 };

	//olc::Sprite* sokoban_sprite = new olc::Sprite("data/SokobanClone.png");
	olc::Sprite* sokoban_clone = nullptr;
	olc::Decal* sokoban_sprite = nullptr;

	
	int game_state;
	int editor_state;
	// @TODO We could use a static variable inside a function to increment the level.
	// Must think about decrement too.
	int level_tracker{1};
	
	// Grid grid {
	//     { 16, 15},
	//     { 16, 16}
	// };
	
	Grid grid {
	    { 16, 15},
	    { 64, 64}
	};   
	
	bool pause{ false };	
	int push;
	float holding = 0.0f;

	Player player;	
	Map_Manager *elements;
	//std::vector<Map_Manager> elements;
	Entity_Manager manager;
	std::vector<Winning_Tile> winning_tiles;
	std::vector<Bloc> bloc_to_update;
	
	std::vector<Entity_Manager> previous_states;
	int number_state{-1};
	
	const int number_entity {grid.size.w * grid.size.h};

	// EDITOR
	Selector selector;
	Selector selector_menu;	
	int selector_menu_pos;
	int selector_level_pos;

	std::string save_message;
	float display_save_message_timer = 0.0f;

	Screen editor_menu {
						{ 0, 0 },
						{ SCREEN_WIDTH, 100 }
	};

	olc::Pixel editor_menu_color{ 0, 0, 0, 230 };

	std::array<Selection, MAX_ELEMENT_TO_SELECT> selection;

	// -------------------------------------------------------------
	int get_index(int x, int y)
	{
		return y * grid.size.w + x;
	}
	
	// -------------------------------------------------------------
	void load_level(std::string lvl)
	{
		memset(elements, 0, number_entity * sizeof(Map_Manager));
		//elements.clear();
		manager.reset_all();
		winning_tiles.clear();

		for(int y{0}; y < grid.size.h; ++y)
			for(int x{0}; x < grid.size.w; ++x) {
				std::string temp = "";
				int i = get_index(x, y);
				int starting_field_bit = i * 16;
				
				//read throught the 16 bits field
				for(int b{starting_field_bit}; b < starting_field_bit + 16; ++b)
				{
					temp += lvl[b];
				}
				
				std::uint8_t ground_type = stol(temp.substr(0, 8));
				std::uint8_t entity_type = stol(temp.substr(8, 8));
				elements[i].ground = ground_type;
				elements[i].entity = entity_type;
				
				if(entity_type & is_player)
				{
					manager.player.is_alive = true;
					manager.player.x = x;
					manager.player.y = y;
				}
				if(entity_type & is_bloc)
				{
					elements[i].entity_index = manager.bloc++;
					manager.blocs.push_back(Bloc(x, y));
				}
				if(entity_type & is_wall)
				{
					elements[i].entity_index = manager.wall++;
					manager.walls.push_back(Wall(x, y));
				}
				if(ground_type & is_winning_tile)
				    winning_tiles.push_back(Winning_Tile(x, y));
			}
	}
	
	// -------------------------------------------------------------
	bool test_next(int x, int y, int direction, int player_strength)
	{
		bool add_to_update { false };
		int i = get_index(x, y);
		
		if(!(elements[i].ground | clear))
			return false;
		if(elements[i].entity & is_wall)
			return false;
		if(elements[i].ground & is_solid)
			if(!(elements[i].entity & is_bloc))
				return true;

		if(player_strength > 0)
		{
			--player_strength;
			switch(direction)
			{
			case NORTH:
				add_to_update = test_next(x, y - 1, NORTH, player_strength);
				break;
			case EAST:
				add_to_update = test_next(x + 1, y, EAST, player_strength);
				break;			
			case SOUTH:
				add_to_update = test_next(x, y + 1, SOUTH, player_strength);
				break;
			case WEST:
				add_to_update = test_next(x - 1, y, WEST, player_strength);
				break;
			}
		}
		
		if(add_to_update)
			bloc_to_update.push_back(Bloc(x, y));
		else
			return false;
	}

   	// -------------------------------------------------------------
	void update(int ux, int uy)
	{
		store_state();
		if(bloc_to_update.size() != 0)
		{
			for(int i{0}; i < bloc_to_update.size(); i--)
			{
				int index = get_index(bloc_to_update[i].x, bloc_to_update[i].y);
  				int bloc_index = elements[index].entity_index;

				manager.blocs[bloc_index].x += ux;
				manager.blocs[bloc_index].y += uy;
			}
		}
		manager.player.x += ux;
		manager.player.y += uy;

		bloc_to_update.clear();
		update_map();
		if(check_win())
		{
			previous_states.clear();
			number_state = -1;

			if(level_tracker < get_max_level())
				++level_tracker;
			std::string load_next_level = get_level(level_tracker);
			
			if(load_next_level != "error") 
				load_level(load_next_level);
			else
				pause = true;		  
		}
	}

	// -------------------------------------------------------------
	void update_map()
	{
		for(int i{0}; i < number_entity; ++i)
		{
			elements[i].entity &= clear;
			elements[i].entity_index = 0;
		}
		
		for(int w{0}; w < manager.walls.size(); ++w)
		{
			int wall_index = get_index(manager.walls[w].x, manager.walls[w].y);
			elements[wall_index].entity |= is_wall;
			elements[wall_index].entity_index = w;
		}
		for(int b{0}; b < manager.blocs.size(); ++b)
		{
			int bloc_index = get_index(manager.blocs[b].x, manager.blocs[b].y);
			elements[bloc_index].entity |= is_bloc;
			elements[bloc_index].entity_index = b;
		}

                
                
		if(manager.player.is_alive)
		{
			int player_index = get_index(manager.player.x, manager.player.y);
			elements[player_index].entity |= is_player;
		}
	}

 	// -------------------------------------------------------------
	void store_state()
	{
		number_state++;
		previous_states.emplace_back(Entity_Manager());
		previous_states[number_state].copy(manager);
	}

	// -------------------------------------------------------------
	void rewind() 
	{
		manager.reset_all();
		manager.copy(previous_states[number_state]);
		previous_states.pop_back();
		--number_state;
		update_map();
	}

	void restart()
	{
		manager.reset_all();
		manager.copy(previous_states[0]);
		previous_states.clear();
		number_state = -1;
		update_map();
	}

	// -------------------------------------------------------------
	bool check_win()
	{
		int correct = 0;
		for(int i{0}; i < manager.bloc; ++i)
		{
			for(int j{0}; j < winning_tiles.size(); ++j)
			{
				if(manager.blocs[i].x == winning_tiles[j].x &&
				   manager.blocs[i].y == winning_tiles[j].y)
					++correct;
			}
		}
		if(correct == winning_tiles.size())
			return true;
		else
			return false;
	}

	// -------------------------------------------------------------	
	void drawing_routine()
	{			
	    for(int y{0}; y < grid.size.h; ++y)
			for(int x{0}; x < grid.size.w; ++x)
			{
				int i = get_index(x, y);
				if(elements[i].ground & clear)
					FillRect(x * grid.unit_size.w, y * grid.unit_size.h,
							 grid.unit_size.w, grid.unit_size.h, olc::BLACK);
				
				if(elements[i].ground & is_solid)
					FillRect(x * grid.unit_size.w, y * grid.unit_size.h,
							 grid.unit_size.h, grid.unit_size.h, olc::GREY);

				if(elements[i].ground & is_winning_tile)
				{
					int tile_index;
					for(int w{0}; w < winning_tiles.size(); ++w)
					{
						if(get_index(winning_tiles[w].x, winning_tiles[w].y) == i)
							tile_index = w;
					}
					// DrawRect(x * grid.unit_size.w, y * grid.unit_size.h,
					// 		 grid.unit_size.w - 1, grid.unit_size.h - 1, olc::RED);
					winning_tiles[tile_index].DrawSelf(this, grid.unit_size, sokoban_sprite);
				}

				if(elements[i].entity & is_wall)
					manager.walls[elements[i].entity_index].DrawSelf(this, grid.unit_size, sokoban_sprite);

				if(elements[i].entity & is_bloc)
					manager.blocs[elements[i].entity_index].DrawSelf(this, grid.unit_size, sokoban_sprite);

				if(elements[i].entity & is_player)
					if(manager.player.is_alive)
						manager.player.DrawSelf(this, grid.unit_size, push, sokoban_sprite);
			}

		if(number_state < 0)
			DrawString(SCREEN_WIDTH - 30, 20, "0", olc::YELLOW, 3);
		else
			DrawString(SCREEN_WIDTH - 30, 20, std::to_string(number_state + 1), olc::YELLOW, 3);
		
		if(game_state == EDITOR)
			draw_editor();
	}

	void draw_editor()
	{
		for(int y{0}; y < grid.size.h; ++y)
			for(int x{0}; x < grid.size.w; ++x) 
			{
				DrawLine(x * grid.unit_size.w, 0,
						 x * grid.unit_size.w, grid.size.h * grid.unit_size.h, olc::WHITE); 
				DrawLine(0, y * grid.unit_size.h,
						 grid.size.w * grid.unit_size.w, y * grid.unit_size.h, olc::WHITE);
			}

		for(int i{0}; i < winning_tiles.size(); ++i)
		{
			DrawRect(winning_tiles[i].x * grid.unit_size.w,
					 winning_tiles[i].y * grid.unit_size.h,
					 grid.unit_size.w - 1, grid.unit_size.h - 1, olc::RED);
		}

		DrawRect(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, olc::WHITE);
		DrawRect(selector.x * grid.unit_size.w, selector.y * grid.unit_size.h,
				 grid.unit_size.w, grid.unit_size.h, olc::WHITE);

		if(display_save_message_timer > 0.0f)
			DrawString(10, 10, save_message, olc::YELLOW);


		// @TODO: This is very bare bone...
		// if more than a few level will need to go inline
		if(editor_state == EDITOR_MENU)
		{
			SetPixelMode(olc::Pixel::ALPHA);
			FillRect(editor_menu.pos.x * grid.unit_size.w, editor_menu.pos.y * grid.unit_size.h,
					  editor_menu.size.w, editor_menu.size.h, editor_menu_color);
			SetPixelMode(olc::Pixel::NORMAL);

			DrawString(64, 16, "Load level:");
			int max_level = get_max_level();
			int n = 2;
			for(int i{1}; i < max_level + 1; ++i)
			{
				if(i == level_tracker)
					DrawString((n * grid.size.w) + 4, 52, std::to_string(i), olc::RED);
				else
					DrawString((n * grid.size.w) + 4, 52, std::to_string(i), olc::WHITE);
				
				if(i == selector_level_pos)
					DrawRect(n * grid.size.w, 48, grid.unit_size.w, grid.unit_size.h, olc::WHITE);
				n += 2;
			}
		}

		if(editor_state == SELECTION_MENU)
		{
			SetPixelMode(olc::Pixel::ALPHA);
			FillRect(editor_menu.pos.x * grid.unit_size.w, editor_menu.pos.y * grid.unit_size.h,
					 editor_menu.size.w, editor_menu.size.h, editor_menu_color);

			// placeholder for solid 
			FillRect(2 * grid.unit_size.w, 2 * grid.unit_size.h,
					 grid.unit_size.h, grid.unit_size.h, olc::GREY);

			// placeholder for wall
			FillRect(4 * grid.unit_size.w, 2 * grid.unit_size.h,
					 grid.unit_size.h, grid.unit_size.h, olc::VERY_DARK_BLUE);
				
			// placeholder for bloc
			FillRect(6 * grid.unit_size.w, 2 * grid.unit_size.h,
					 grid.unit_size.h, grid.unit_size.h, olc::BLUE);

			//placeholder for winning tile
			DrawRect(8 * grid.unit_size.w, 2 * grid.unit_size.h,
					 grid.unit_size.w - 1, grid.unit_size.h - 1, olc::RED);

			// placeholder for player
		   FillRect(10 * grid.unit_size.w, 2 * grid.unit_size.h,
					 grid.unit_size.w - 1, grid.unit_size.h - 1, olc::GREEN);


			// selector in menu
			DrawRect(selection[selector_menu_pos].pos.x * grid.unit_size.w,
					 selection[selector_menu_pos].pos.y * grid.unit_size.h,
					 grid.unit_size.w, grid.unit_size.h, olc::WHITE);
			
			SetPixelMode(olc::Pixel::NORMAL);
		}
	}

	void select_next()
	{
		if(selector_menu_pos < MAX_ELEMENT_TO_SELECT - 1)
			++selector_menu_pos;
	}

	void select_previous()
	{
		if(selector_menu_pos > SOLID)
			--selector_menu_pos;
	}

	void remove_winning_tile()
	{
		int selector_index = get_index(selector.x, selector.y);
		int tile_index;
		for(int i{0}; i < winning_tiles.size(); ++i)
		{
			if(get_index(winning_tiles[i].x, winning_tiles[i].y) == selector_index)
				tile_index = i;
		}
		std::swap(winning_tiles[tile_index], winning_tiles[winning_tiles.size() - 1]);
	    winning_tiles.pop_back();		
		elements[selector_index].ground ^= is_winning_tile; // rm winning_tile		
	}

	void remove_wall(Map_Manager &element)
	{
		std::swap(manager.walls[element.entity_index], manager.walls[manager.wall - 1]);
		manager.walls.pop_back();
		--manager.wall;
	}

	void remove_bloc(Map_Manager &element)
	{
		std::swap(manager.blocs[element.entity_index], manager.blocs[manager.bloc - 1]);
		manager.blocs.pop_back();
		--manager.bloc;
	}

	void remove_player(Map_Manager &element)
	{
		element.entity ^= is_player;
		manager.player.is_alive = false;
	}

	
	void editor_delete(Map_Manager &element)
	{
		if(element.entity | clear)
		{
			if(element.entity & is_wall)
				remove_wall(element);

			if(element.entity & is_bloc)
				remove_bloc(element);

			if(element.entity & is_player)
				remove_player(element);
		}
		else
		{			
			if(element.ground & is_winning_tile)
				remove_winning_tile();
			else
				element.ground &= clear;
		}
		
		update_map();
	}
 

	void editor_insert()
	{
		int selector_index = get_index(selector.x, selector.y);
		editor_delete(elements[selector_index]);
		
		switch(selector_menu_pos)
		{
		case SOLID:
			if(elements[selector_index].ground & is_winning_tile)
				remove_winning_tile();

			elements[selector_index].ground |= is_solid;
			break;
				
		case WALL:
			elements[selector_index].ground |= is_solid;
			elements[selector_index].entity |= is_wall;
			elements[selector_index].entity_index = manager.wall++;
			manager.walls.push_back(Wall(selector.x, selector.y));
			break;
			
		case BLOC:
			elements[selector_index].ground |= is_solid;
			elements[selector_index].entity |= is_bloc;
			elements[selector_index].entity_index = manager.bloc++;
			manager.blocs.push_back(Bloc(selector.x, selector.y));
			break;
			
		case WINNING_TILE:
			elements[selector_index].ground |= is_solid;
			elements[selector_index].ground |= is_winning_tile;
			winning_tiles.push_back(Winning_Tile(selector.x, selector.y));
			break;
			
		case PLAYER:
			elements[selector_index].ground |= is_solid;
			elements[selector_index].entity |= is_player;
			manager.player.x = selector.x;
			manager.player.y = selector.y;
			manager.player.is_alive = true;
			break;
		}	   
	}

	void editor_save()
	{
		std::string buffer = "";
		for(int i{0}; i < number_entity; ++i)
		{
			buffer += std::bitset<8>(elements[i].ground).to_string();
			buffer += " ";
			buffer += std::bitset<8>(elements[i].entity).to_string();
			buffer += "  ";
		}
	
		std::ofstream level_data{"data/level" + std::to_string(level_tracker) + ".dat"};
		if(!level_data)
			save_message = "error";
		else
		{
			level_data << buffer;
			level_data.close();
			save_message = "level saved";
		}
		display_save_message_timer = 2.0f;
	}
	
	void editor_new()
	{
		int last_level = get_max_level();
		std::ofstream new_level{"data/level" + std::to_string(last_level + 1) + ".dat"};
		if(!new_level)
			printf("couldn't open new file");
		else{
			std::string buffer;
			for(int i{0}; i < number_entity; ++i)
			{
				buffer += std::bitset<8>(clear).to_string();
				buffer += " ";
				buffer += std::bitset<8>(clear).to_string();
				buffer += "  ";
			}
			new_level << buffer;
			new_level.close();
			level_tracker = last_level + 1;
			manager.player.is_alive = false;
			load_level(get_level(level_tracker));
		}
	}
	
	// -------------------------------------------------------------
	bool OnUserCreate() override
	{
		game_state = GAME;
		editor_state = DEFAULT;
		push = SOUTH;

		sokoban_clone = new olc::Sprite("./data/SokobanClone.png");
		sokoban_sprite = new olc::Decal(sokoban_clone);

		
		elements = new Map_Manager[number_entity];

		std::string level = get_level(level_tracker);
		load_level(level);

		Pos origin = { 2, 2 };
		for(int e{0}; e < MAX_ELEMENT_TO_SELECT; ++e)
		{
			selection[e].pos.x = origin.x;
			selection[e].pos.y = origin.y;
			origin.x += 2;
		}

		selector_menu_pos = SOLID;
		selector_menu.place(selection[selector_menu_pos].pos.x, selection[selector_menu_pos].pos.y);
		selector_level_pos = 1;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame, draws random coloured pixels
		Clear(olc::BLACK);
		drawing_routine();
		
		bool pushing{false};
		if(display_save_message_timer > 0.0f)
			display_save_message_timer -= fElapsedTime;
    
		if(!pause)
		{
			switch (game_state)
			{

				// MAIN GAME
			case GAME:
				if(manager.player.is_alive)
				{
					if(GetKey(olc::UP).bPressed)
					{
						push = NORTH;
						pushing = true;
					}
					if(GetKey(olc::RIGHT).bPressed)
					{
						push = EAST;
						pushing = true;
					}
					if(GetKey(olc::DOWN).bPressed)
					{
						push = SOUTH;
						pushing = true;
					}
					if(GetKey(olc::LEFT).bPressed)
					{
						push = WEST;
						pushing = true;
					}
				}
				
				if(GetKey(olc::Z).bPressed)					
					if(!previous_states.empty())
						rewind();


				
				if(GetKey(olc::R).bPressed)
					restart();

				
				if(GetKey(olc::TAB).bPressed)
					game_state = EDITOR;
				
				break;

				// EDITOR INTERFACE
			case EDITOR:
				if(GetKey(olc::TAB).bPressed)
					game_state = GAME;
				
				switch(editor_state)
				{
				case DEFAULT:					
				if(GetKey(olc::UP).bPressed) selector.move(0, -1);
				if(GetKey(olc::RIGHT).bPressed) selector.move(1, 0);
				if(GetKey(olc::DOWN).bPressed) selector.move(0, 1);
				if(GetKey(olc::LEFT).bPressed) selector.move(-1, 0);

				if(GetKey(olc::M).bPressed)
					editor_state = EDITOR_MENU;

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::E).bPressed)
					editor_state = SELECTION_MENU;

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::D).bPressed) {
					int i = get_index(selector.x, selector.y);
					editor_delete(elements[i]);
				}

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::F).bPressed)
				    editor_insert();

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::S).bPressed)
				    editor_save();

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::N).bPressed)
				    editor_new();
				break;
				
				case EDITOR_MENU:
					if(GetKey(olc::M).bPressed)
						editor_state = DEFAULT;

					if(GetKey(olc::UP).bPressed)
						if(selector_level_pos < get_max_level())
							++selector_level_pos;
					if(GetKey(olc::RIGHT).bPressed)
						if(selector_level_pos < get_max_level())
							++selector_level_pos;
					if(GetKey(olc::DOWN).bPressed)
						if(selector_level_pos > 1)
							--selector_level_pos;
					if(GetKey(olc::LEFT).bPressed)
 						if(selector_level_pos > 1)
							--selector_level_pos;

					if(GetKey(olc::ENTER).bPressed)
					{
						level_tracker = selector_level_pos;
						load_level(get_level(level_tracker));
					}

				    break;
					
				case SELECTION_MENU:
					if(GetKey(olc::UP).bPressed) select_next();				
					if(GetKey(olc::RIGHT).bPressed) select_next();				
					if(GetKey(olc::DOWN).bPressed) select_previous();
					if(GetKey(olc::LEFT).bPressed) select_previous();

				if(GetKey(olc::CTRL).bHeld && GetKey(olc::E).bPressed)
					editor_state = DEFAULT;					
					
					break;
				}				
			}
		}
		
		if(pushing)
		{
			bool need_update;		
			switch(push)
			{
			case NORTH:
				need_update = test_next(manager.player.x, manager.player.y - 1, NORTH, manager.player.strength);
				if(need_update)
					update(0, -1);	
				break;
			case EAST:
				need_update = test_next(manager.player.x + 1, manager.player.y, EAST, manager.player.strength);
				if(need_update)
					update(1, 0);
				break;			
			case SOUTH:
				need_update = test_next(manager.player.x, manager.player.y + 1, SOUTH, manager.player.strength);
				if(need_update){
					update(0, 1);
				}
				break;
			case WEST:
				need_update = test_next(manager.player.x - 1, manager.player.y, WEST, manager.player.strength);
				if(need_update)
					update(-1, 0);
				break;
			}		   
		}

		return true;
	}
};

int main()
{
	Sokoban demo;
	if (demo.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 1))
		demo.Start();
		
	return 0;
}
