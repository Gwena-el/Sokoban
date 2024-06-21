#include <string>
#include <fstream>

std::string get_level(int level_tracker) 
{
	std::string level = "";
	std::string temp;

	std::ifstream file_data{ "data/level" + std::to_string(level_tracker) + ".dat" };
	if(!file_data)
		return "error";
			   
	while(file_data)
	{
		//std::getline(file_data, temp);
		file_data >> temp;
		level += temp;
	}
	return level;
}

int get_max_level()
{
	int i{1};
	bool check_exist{true};		

	while(check_exist)
	{
		std::ifstream file{"data/level" + std::to_string(i) + ".dat"};
		if(!file)
			check_exist = false;
		else{
			i++;
			file.close();
		}
	}
	return i - 1;
}
