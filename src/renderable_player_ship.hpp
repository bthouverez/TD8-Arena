#ifndef RENDERABLE_PLAYER_SHIP_HEADER
#define RENDERABLE_PLAYER_SHIP_HEADER

#include "renderable_entity.hpp"

#include <string>


class RenderablePlayerShip : public RenderableEntity 
{
public:
	RenderablePlayerShip() : RenderableEntity() {}
	virtual ~RenderablePlayerShip() {}

	virtual bool init()
	{		
		return loadOBJ(std::string("data/mesh/playership.obj"));
	}	
};

#endif