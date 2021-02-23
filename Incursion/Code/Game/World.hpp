#pragma once

#include <vector>

class Map;
class Game;

class World
{
public:
	World(Game* game);
	~World();

	void StartLevel();
	void LoadNextLevel();

	void Update( float deltaSeconds );
	void Render()const;

	Map* GetCurrentMap()const { return m_currentMap; }
	
private:
	Map* m_currentMap = nullptr;
	Game* m_game = nullptr;
	std::vector<Map*> m_maps;
};