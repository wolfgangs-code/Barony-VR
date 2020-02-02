/*-------------------------------------------------------------------------------

	BARONY
	File: actgeneral.cpp
	Desc: very small and general behavior functions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "sound.hpp"
#include "entity.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "menu.hpp"
#include "magic/magic.hpp"
#include "interface/interface.hpp"

/*-------------------------------------------------------------------------------

	act*

	The following functions describe various entity behaviors. All functions
	take a pointer to the entities that use them as an argument.

-------------------------------------------------------------------------------*/

void actAnimator(Entity* my)
{
	if ( my->skill[4] == 0 )
	{
		my->skill[4] = 1;
		map.tiles[my->skill[0] + (int)my->y * MAPLAYERS + (int)my->x * MAPLAYERS * map.height] -= my->skill[1] - 1;
	}

	if ( (int)floor(my->x) < 0 || (int)floor(my->x) >= map.width || (int)floor(my->y) < 0 || (int)floor(my->y) >= map.height )
	{
		list_RemoveNode(my->mynode);
		return;
	}

	my->skill[3]++;
	if ( my->skill[3] >= 10 )
	{
		my->skill[3] = 0;
		map.tiles[my->skill[0] + (int)floor(my->y)*MAPLAYERS + (int)floor(my->x)*MAPLAYERS * map.height]++;
		my->skill[5]++;
		if (my->skill[5] == my->skill[1])
		{
			my->skill[5] = 0;
			map.tiles[my->skill[0] + (int)floor(my->y)*MAPLAYERS + (int)floor(my->x)*MAPLAYERS * map.height] -= my->skill[1];
		}
	}
}

#define TESTSPRITES

void actRotate(Entity* my)
{
	my->yaw += 0.1;
	my->flags[PASSABLE] = true; // this entity should always be passable

#ifdef TESTSPRITES
	if ( keystatus[SDL_SCANCODE_HOME] )
	{
		keystatus[SDL_SCANCODE_HOME] = 0;
		my->sprite++;
		if ( my->sprite >= nummodels )
		{
			my->sprite = 0;
		}
		messagePlayer(clientnum, "test sprite: %d", my->sprite);
	}
	if ( keystatus[SDL_SCANCODE_END] )
	{
		keystatus[SDL_SCANCODE_END] = 0;
		my->sprite += 10;
		if ( my->sprite >= nummodels )
		{
			my->sprite = 0;
		}
		messagePlayer(clientnum, "test sprite: %d", my->sprite);
	}
#endif
}

#define LIQUID_TIMER my->skill[0]
#define LIQUID_INIT my->skill[1]
#define LIQUID_LAVA my->flags[USERFLAG1]
#define LIQUID_LAVANOBUBBLE my->skill[4]

void actLiquid(Entity* my)
{
	// as of 1.0.7 this function is DEPRECATED

	list_RemoveNode(my->mynode);
	return;

	if ( !LIQUID_INIT )
	{
		LIQUID_INIT = 1;
		LIQUID_TIMER = 60 * (rand() % 20);
		if ( LIQUID_LAVA )
		{
			my->light = lightSphereShadow(my->x / 16, my->y / 16, 2, 128);
		}
	}
	LIQUID_TIMER--;
	if ( LIQUID_TIMER <= 0 )
	{
		LIQUID_TIMER = 60 * 20 + 60 * (rand() % 20);
		if ( !LIQUID_LAVA )
		{
			playSoundEntityLocal( my, 135, 32 );
		}
		else
		{
			playSoundEntityLocal( my, 155, 100 );
		}
	}
	if ( LIQUID_LAVA && !LIQUID_LAVANOBUBBLE )
	{
		if ( ticks % 40 == my->getUID() % 40 && rand() % 3 == 0 )
		{
			int c, j = 1 + rand() % 2;
			for ( c = 0; c < j; c++ )
			{
				Entity* entity = spawnGib( my );
				entity->x += rand() % 16 - 8;
				entity->y += rand() % 16 - 8;
				entity->flags[SPRITE] = true;
				entity->sprite = 42;
				entity->fskill[3] = 0.01;
				double vel = (rand() % 10) / 20.f;
				entity->vel_x = vel * cos(entity->yaw);
				entity->vel_y = vel * sin(entity->yaw);
				entity->vel_z = -.15 - (rand() % 15) / 100.f;
				entity->z = 7.5;
			}
		}
	}
}

void actEmpty(Entity* my)
{
	// an empty action
	// used on clients to permit dead reckoning and other interpolation
}

void actFurniture(Entity* my)
{
	if ( !my )
	{
		return;
	}

	if ( !my->flags[BURNABLE] )
	{
		my->flags[BURNABLE] = true;
	}

	my->actFurniture();
}

void Entity::actFurniture()
{
	if ( !furnitureInit )
	{
		furnitureInit = 1;
		if ( furnitureType == FURNITURE_TABLE || FURNITURE_BUNKBED || FURNITURE_BED || FURNITURE_PODIUM )
		{
			furnitureHealth = 15 + rand() % 5;
		}
		else
		{
			furnitureHealth = 4 + rand() % 4;
		}
		furnitureMaxHealth = furnitureHealth;
		flags[BURNABLE] = true;
	}
	else
	{
		if ( multiplayer != CLIENT )
		{
			// burning
			if ( flags[BURNING] )
			{
				if ( ticks % 15 == 0 )
				{
					furnitureHealth--;
				}
			}

			// furniture mortality :p
			if ( furnitureHealth <= 0 )
			{
				int c;
				for ( c = 0; c < 5; c++ )
				{
					Entity* entity = spawnGib(this);
					entity->flags[INVISIBLE] = false;
					entity->sprite = 187; // Splinter.vox
					entity->x = floor(x / 16) * 16 + 8;
					entity->y = floor(y / 16) * 16 + 8;
					entity->y += -3 + rand() % 6;
					entity->x += -3 + rand() % 6;
					entity->z = -5 + rand() % 10;
					entity->yaw = (rand() % 360) * PI / 180.0;
					entity->pitch = (rand() % 360) * PI / 180.0;
					entity->roll = (rand() % 360) * PI / 180.0;
					entity->vel_x = (rand() % 10 - 5) / 10.0;
					entity->vel_y = (rand() % 10 - 5) / 10.0;
					entity->vel_z = -.5;
					entity->fskill[3] = 0.04;
					serverSpawnGibForClient(entity);
				}
				playSoundEntity(this, 176, 128);
				Entity* entity = uidToEntity(parent);
				if ( entity != NULL )
				{
					entity->itemNotMoving = 0; // drop the item that was on the table
					entity->itemNotMovingClient = 0; // clear the client item gravity flag
					serverUpdateEntitySkill(entity, 18); //update both the above flags.
					serverUpdateEntitySkill(entity, 19);
				}
				list_RemoveNode(mynode);
				return;
			}

			// using
			int i;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if ( (i == 0 && selectedEntity == this) || (client_selected[i] == this) )
				{
					if (inrange[i])
					{
						switch ( furnitureType )
						{
							case FURNITURE_CHAIR:
								messagePlayer(i, language[476]);
								break;
							case FURNITURE_TABLE:
								messagePlayer(i, language[477]);
								break;
							case FURNITURE_BED:
								messagePlayer(i, language[2493]);
								break;
							case FURNITURE_BUNKBED:
								if ( i == 0 || i == 2 )
								{
									messagePlayer(i, language[2494]);
								}
								else
								{
									messagePlayer(i, language[2495]);
								}
								break;
							case FURNITURE_PODIUM:
								messagePlayer(i, language[2496]);
								break;
							default:
								messagePlayer(i, language[477]);
								break;
						}
					}
				}
			}
		}
	}
}

// an easter egg
#define MCAXE_USED my->skill[0]

void actMCaxe(Entity* my)
{
	my->yaw += .05;
	if ( my->yaw > PI * 2 )
	{
		my->yaw -= PI * 2;
	}
	if ( !MCAXE_USED )
	{
		if ( multiplayer != CLIENT )
		{
			// use
			int i;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if ( (i == 0 && selectedEntity == my) || (client_selected[i] == my) )
				{
					if (inrange[i])
					{
						messagePlayer(i, language[478 + rand() % 5]);
						MCAXE_USED = 1;
						serverUpdateEntitySkill(my, 0);
					}
				}
			}
		}

		// bob
		my->z -= sin(my->fskill[0] * PI / 180.f);
		my->fskill[0] += 6;
		if ( my->fskill[0] >= 360 )
		{
			my->fskill[0] -= 360;
		}
		my->z += sin(my->fskill[0] * PI / 180.f);
	}
	else
	{
		my->z += 1;
		if ( my->z > 64 )
		{
			list_RemoveNode(my->mynode);
			return;
		}
	}
}

void actStalagFloor(Entity* my)
{
	//TODO: something?
	if ( !my )
	{
		return;
	}

	my->actStalagFloor();
}

void Entity::actStalagFloor()
{

}

void actStalagCeiling(Entity* my)
{
	//TODO: something?
	if ( !my )
	{
		return;
	}
	my->actStalagCeiling();
}

void Entity::actStalagCeiling()
{

}

void actStalagColumn(Entity* my)
{
	//TODO: something?
	if ( !my )
	{
		return;
	}

	my->actStalagColumn();
}

void Entity::actStalagColumn()
{

}

void actColumn(Entity* my)
{
	//TODO: something?
	if ( !my )
	{
		return;
	}
	if ( my->flags[BLOCKSIGHT] ) // stop the compiler optimising into a different entity.
	{
		my->flags[BLOCKSIGHT] = false;
	}
	my->actColumn();
}

void Entity::actColumn()
{

}

void actCeilingTile(Entity* my)
{
	if ( !my )
	{
		return;
	}
	if ( !my->flags[PASSABLE] )
	{
		my->flags[PASSABLE] = true;
	}
	if ( my->flags[BLOCKSIGHT] )
	{
		my->flags[BLOCKSIGHT] = false;
	}
}

void actPistonBase(Entity* my)
{
	if ( !my )
	{
		return;
	}
}

void actPistonCam(Entity* my)
{
	if ( !my )
	{
		return;
	}
	my->actPistonCam();
}

void Entity::actPistonCam()
{
	yaw += pistonCamRotateSpeed;
	while ( yaw > 2 * PI )
	{
		yaw -= 2 * PI;
	}
	while ( yaw < 0 )
	{
		yaw += 2 * PI;
	}
	if ( (pistonCamDir == 0 || pistonCamDir == 2) && pistonCamRotateSpeed > 0 )
	{
		if ( yaw <= PI && yaw >= -pistonCamRotateSpeed + PI )
		{
			yaw = PI;
			pistonCamRotateSpeed = 0;
		}
	}
	--pistonCamTimer;

	if ( pistonCamDir == 0 ) // bottom
	{
		if ( pistonCamTimer <= 0 )
		{
			pistonCamDir = 1; // up
			pistonCamRotateSpeed = 0.2;
			pistonCamTimer = rand() % 5 * TICKS_PER_SECOND;
		}
	}
	if ( pistonCamDir == 1 ) // up
	{
		z -= 0.1;
		if ( z < -1.75 )
		{
			z = -1.75;
			pistonCamRotateSpeed *= rand() % 2 == 0 ? -1 : 1;
			pistonCamDir = 2; // top
		}
	}
	else if ( pistonCamDir == 2 ) // top
	{
		if ( pistonCamTimer <= 0 )
		{
			pistonCamDir = 3; // down
			pistonCamRotateSpeed = -0.2;
			pistonCamTimer = rand() % 5 * TICKS_PER_SECOND;
		}
	}
	else if ( pistonCamDir == 3 ) // down
	{
		z += 0.1;
		if ( z > 1.75 )
		{
			z = 1.75;
			pistonCamRotateSpeed *= rand() % 2 == 0 ? -1 : 1;
			pistonCamDir = 0; // down
		}
	}
}

void actFloorDecoration(Entity* my)
{
	if ( !my )
	{
		return;
	}
	if ( !my->flags[PASSABLE] )
	{
		my->flags[PASSABLE] = true;
	}

	if ( multiplayer == CLIENT )
	{
		return;
	}

	if ( my->floorDecorationInteractText1 == 0 )
	{
		// no text.
		return;
	}

	// using
	int i;
	for ( i = 0; i < MAXPLAYERS; i++ )
	{
		if ( (i == 0 && selectedEntity == my) || (client_selected[i] == my) )
		{
			if ( inrange[i] )
			{
				// assemble the string.
				char buf[256] = "";
				int totalChars = 0;
				for ( int i = 8; i < 60; ++i )
				{
					if ( i == 28 ) // circuit_status
					{
						continue;
					}
					if ( my->skill[i] != 0 )
					{
						for ( int c = 0; c < 4; ++c )
						{
							buf[totalChars] = static_cast<char>((my->skill[i] >> (c * 8)) & 0xFF);
							++totalChars;
						}
					}
				}
				if ( buf[totalChars] != '\0' )
				{
					buf[totalChars] = '\0';
				}
				std::string output = buf;
				size_t found = output.find("\\n");
				while ( found != std::string::npos )
				{
					output.erase(found, 2);
					output.insert(found, 1, '\n');
					found = output.find("\\n");
				}
				strcpy(buf, output.c_str());
				messagePlayer(i, buf);
			}
		}
	}
}

void actTextSource(Entity* my)
{
	if ( !my )
	{
		return;
	}
	my->actTextSource();
}

TextSourceScript textSourceScript;

int textSourceProcessScriptTag(std::string& input, std::string findTag)
{
	size_t foundScriptTag = input.find(findTag);
	if ( foundScriptTag != std::string::npos )
	{
		if ( !textSourceScript.containsOperator(findTag.back()) )
		{
			textSourceScript.eraseTag(input, findTag, foundScriptTag);
			// end the function here, return non-error as we found the tag.
			return 0;
		}
		input.erase(foundScriptTag, strlen(findTag.c_str()));
		if ( input.empty() )
		{
			return 0;
		}

		std::string tagValue;

		while ( foundScriptTag < input.length()
			&& input.at(foundScriptTag) != ' '
			&& input.at(foundScriptTag) != '@'
			&& input.at(foundScriptTag) != '\0'
			)
		{
			// usage: Hello @p @d 123 will send to distance 123 units away and send message "Hello player "
			tagValue = tagValue + input.at(foundScriptTag);
			//messagePlayer(clientnum, "%c", output.at(foundDistanceRequirement));
			++foundScriptTag;
		}
		if ( foundScriptTag < input.length() && input.at(foundScriptTag) == ' ' )
		{
			input.erase(input.find(tagValue), tagValue.length() + 1); // clear trailing space
		}
		else
		{
			input.erase(input.find(tagValue), tagValue.length());
		}
		return std::stoi(tagValue);
	}
	return textSourceScript.k_ScriptError;
}

void handleTextSourceScript(std::string input)
{
	bool statOnlyUpdateNeeded = false;

	std::vector<std::string> tokens;
	std::string searchString = input;
	size_t findToken = searchString.find("@");
	while ( findToken != std::string::npos )
	{
		std::string token = "@";
		++findToken;
		while ( findToken < searchString.length()
			&& searchString.at(findToken) != ' '
			&& searchString.at(findToken) != '@'
			&& searchString.at(findToken) != '\0'
			)
		{
			token = token + searchString.at(findToken);
			++findToken;
		}
		searchString.erase(searchString.find(token), token.length());
		tokens.push_back(token);
		findToken = searchString.find("@");
	}

	for ( auto it = tokens.begin(); it != tokens.end(); ++it )
	{
		if ( (*it).find("@clrplayer") != std::string::npos )
		{
			int result = textSourceProcessScriptTag(input, "@clrplayer");
			if ( result != textSourceScript.k_ScriptError )
			{
				textSourceScript.playerClearInventory(true);
				if ( multiplayer == SERVER )
				{
					for ( int c = 1; c < MAXPLAYERS; ++c )
					{
						if ( stats[c] && !client_disconnected[c] )
						{
							stats[c]->freePlayerEquipment();
							stats[c]->clearStats();
							textSourceScript.updateClientInformation(c, true, true, TextSourceScript::CLIENT_UPDATE_ALL);
						}
					}
				}
			}
		}
		else if ( (*it).find("@class=") != std::string::npos )
		{
			int result = textSourceProcessScriptTag(input, "@class=");
			if ( result != textSourceScript.k_ScriptError && result >= CLASS_BARBARIAN && result < NUMCLASSES )
			{
				if ( !enabledDLCPack1 && result >= CLASS_CONJURER && result <= CLASS_BREWER )
				{
					result = CLASS_BARBARIAN;
				}
				if ( !enabledDLCPack2 && result >= CLASS_MACHINIST && result <= CLASS_HUNTER )
				{
					result = CLASS_BARBARIAN;
				}
				for ( int c = 0; c < MAXPLAYERS; ++c )
				{
					if ( stats[c] && !client_disconnected[c] )
					{
						client_classes[c] = result;
						initClass(c);
					}
				}
				for ( int c = 1; c < MAXPLAYERS; ++c )
				{
					textSourceScript.updateClientInformation(c, false, false, TextSourceScript::CLIENT_UPDATE_CLASS);
				}
			}
		}
		else if ( (*it).find("@clrstats") != std::string::npos )
		{
			int result = textSourceProcessScriptTag(input, "@clrstats");
			if ( result != textSourceScript.k_ScriptError )
			{
				for ( int c = 0; c < MAXPLAYERS; ++c )
				{
					if ( stats[c] && !client_disconnected[c] )
					{
						stats[c]->HP = DEFAULT_HP;
						stats[c]->MAXHP = DEFAULT_HP;
						stats[c]->OLDHP = stats[c]->HP;
						stats[c]->MP = DEFAULT_MP;
						stats[c]->MAXMP = DEFAULT_MP;
						stats[c]->STR = 0;
						stats[c]->DEX = 0;
						stats[c]->CON = 0;
						stats[c]->INT = 0;
						stats[c]->PER = 0;
						stats[c]->CHR = 0;
						stats[c]->LVL = 1;
						stats[c]->GOLD = 0;
						stats[c]->EXP = 0;
					}
				}
				statOnlyUpdateNeeded = true;
			}
		}
		else if ( (*it).find("@copyNPC=") != std::string::npos )
		{
			std::string profTag = "@copyNPC=";
			int result = textSourceProcessScriptTag(input, profTag);
			if ( result != textSourceScript.k_ScriptError )
			{
				for ( node_t* node = map.entities->first; node; node = node->next )
				{
					Entity* scriptEntity = (Entity*)node->element;
					if ( scriptEntity && scriptEntity->behavior == &actMonster )
					{
						Stat* scriptStats = scriptEntity->getStats();
						if ( scriptStats && !strcmp(scriptStats->name, "scriptNPC") && (scriptStats->MISC_FLAGS[STAT_FLAG_NPC] & 0xFF) == result )
						{
							// copy stats.
							for ( int c = 0; c < MAXPLAYERS && !client_disconnected[c]; ++c )
							{
								stats[c]->copyNPCStatsAndInventoryFrom(*scriptStats);
							}
							statOnlyUpdateNeeded = true;
						}
					}
				}
			}
		}
		else
		{
			for ( int i = 0; i < NUMSTATS; ++i )
			{
				std::string profTag = "@st";
				switch ( i )
				{
					case STAT_STR:
						profTag.append("STR");
						break;
					case STAT_DEX:
						profTag.append("DEX");
						break;
					case STAT_CON:
						profTag.append("INT");
						break;
					case STAT_INT:
						profTag.append("CON");
						break;
					case STAT_PER:
						profTag.append("PER");
						break;
					case STAT_CHR:
						profTag.append("CHR");
						break;
					default:
						break;
				}
				profTag.append("=");
				if ( (*it).find(profTag) != std::string::npos )
				{
					int result = textSourceProcessScriptTag(input, profTag);
					if ( result != textSourceScript.k_ScriptError )
					{
						for ( int c = 0; c < MAXPLAYERS; ++c )
						{
							if ( stats[c] && !client_disconnected[c] )
							{
								switch ( i )
								{
									case STAT_STR:
										stats[c]->STR = result;
										break;
									case STAT_DEX:
										stats[c]->DEX = result;
										break;
									case STAT_CON:
										stats[c]->CON = result;
										break;
									case STAT_INT:
										stats[c]->INT = result;
										break;
									case STAT_PER:
										stats[c]->PER = result;
										break;
									case STAT_CHR:
										stats[c]->CHR = result;
										break;
									default:
										break;
								}
							}
						}
						statOnlyUpdateNeeded = true;
					}
				}
			}
			for ( int i = 0; i < NUMSTATS; ++i )
			{
				std::string profTag = "@st";
				switch ( i )
				{
					case STAT_STR:
						profTag.append("STR");
						break;
					case STAT_DEX:
						profTag.append("DEX");
						break;
					case STAT_CON:
						profTag.append("INT");
						break;
					case STAT_INT:
						profTag.append("CON");
						break;
					case STAT_PER:
						profTag.append("PER");
						break;
					case STAT_CHR:
						profTag.append("CHR");
						break;
					default:
						break;
				}
				profTag.append("+");
				if ( (*it).find(profTag) != std::string::npos )
				{
					int result = textSourceProcessScriptTag(input, profTag);
					if ( result != textSourceScript.k_ScriptError )
					{
						for ( int c = 0; c < MAXPLAYERS; ++c )
						{
							if ( stats[c] && !client_disconnected[c] )
							{
								switch ( i )
								{
									case STAT_STR:
										stats[c]->STR += result;
										break;
									case STAT_DEX:
										stats[c]->DEX += result;
										break;
									case STAT_CON:
										stats[c]->CON += result;
										break;
									case STAT_INT:
										stats[c]->INT += result;
										break;
									case STAT_PER:
										stats[c]->PER += result;
										break;
									case STAT_CHR:
										stats[c]->CHR += result;
										break;
									default:
										break;
								}
							}
						}
						statOnlyUpdateNeeded = true;
					}
				}
			}

			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				std::string profTag = "@pro";
				profTag.append(std::to_string(i).append("="));
				if ( (*it).find(profTag) != std::string::npos )
				{
					int result = textSourceProcessScriptTag(input, profTag);
					if ( result != textSourceScript.k_ScriptError )
					{
						for ( int c = 0; c < MAXPLAYERS; ++c )
						{
							if ( stats[c] && !client_disconnected[c] )
							{
								stats[c]->PROFICIENCIES[i] = result;
							}
						}
						statOnlyUpdateNeeded = true;
					}
				}
			}
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				std::string profTag = "@pro";
				profTag.append(std::to_string(i).append("+"));
				if ( (*it).find(profTag) != std::string::npos )
				{
					int result = textSourceProcessScriptTag(input, profTag);
					if ( result != textSourceScript.k_ScriptError )
					{
						for ( int c = 0; c < MAXPLAYERS; ++c )
						{
							if ( stats[c] && !client_disconnected[c] )
							{
								stats[c]->PROFICIENCIES[i] += result;
							}
						}
						statOnlyUpdateNeeded = true;
					}
				}
			}
		}
	}

	if ( statOnlyUpdateNeeded )
	{
		for ( int c = 1; c < MAXPLAYERS; ++c )
		{
			textSourceScript.updateClientInformation(c, false, false, TextSourceScript::CLIENT_UPDATE_ALL);
		}
	}
}

void Entity::actTextSource()
{
	if ( multiplayer == CLIENT )
	{
		return;
	}

	if ( ((textSourceVariables4W >> 16) & 0xFFFF) == 0 ) // store the delay in the 16 leftmost bits.
	{
		textSourceVariables4W |= (textSourceDelay << 16);
	}

	if ( circuit_status == CIRCUIT_ON )
	{
		// received power
		if ( textSourceDelay > 0 )
		{
			--textSourceDelay;
			return;
		}
		else
		{
			textSourceDelay = (textSourceVariables4W >> 16) & 0xFFFF;
		}
		if ( (textSourceVariables4W & 0xFF) == 0 )
		{
			textSourceVariables4W |= 1;
			// assemble the string.
			char buf[256] = "";
			int totalChars = 0;
			for ( int i = 4; i < 60; ++i )
			{
				if ( i == 28 ) // circuit_status
				{
					continue;
				}
				if ( skill[i] != 0 )
				{
					for ( int c = 0; c < 4; ++c )
					{
						buf[totalChars] = static_cast<char>((skill[i] >> (c * 8)) & 0xFF);
						++totalChars;
					}
				}
			}
			if ( buf[totalChars] != '\0' )
			{
				buf[totalChars] = '\0';
			}
			Uint32 color = SDL_MapRGB(mainsurface->format, (textSourceColorRGB >> 16) & 0xFF, (textSourceColorRGB >> 8) & 0xFF,
				(textSourceColorRGB >> 0) & 0xFF);
			std::string output = buf;

			size_t foundScriptTag = output.find("@script");
			if ( foundScriptTag != std::string::npos )
			{
				if ( (foundScriptTag + strlen("@script")) < output.length()
					&& output.at(foundScriptTag + strlen("@script")) == ' ' )
				{
					output.erase(foundScriptTag, strlen("@script") + 1); // trailing space.
				}
				else
				{
					output.erase(foundScriptTag, strlen("@script"));
				}

				handleTextSourceScript(output);
				return;
			}

			size_t foundPlayerRef = output.find("@p");
			if ( foundPlayerRef != std::string::npos )
			{
				output.erase(foundPlayerRef, 2);
				output.insert(foundPlayerRef, "%s");
			}

			size_t foundDistanceRequirement = output.find("@d");
			int distanceRequirement = -1;
			if ( foundDistanceRequirement != std::string::npos )
			{
				output.erase(foundDistanceRequirement, 2);
				std::string distance;
				while ( foundDistanceRequirement < output.length() 
					&& output.at(foundDistanceRequirement) != ' '
					&& output.at(foundDistanceRequirement) != '\0'
					)
				{
					// usage: Hello @p @d 123 will send to distance 123 units away and send message "Hello player "
					distance = distance + output.at(foundDistanceRequirement);
					//messagePlayer(clientnum, "%c", output.at(foundDistanceRequirement));
					++foundDistanceRequirement;
				}
				distanceRequirement = std::stoi(distance);
				output.erase(output.find(distance), distance.length());
				//messagePlayer(clientnum, "%d", distanceRequirement);
			}

			size_t found = output.find("\\n");
			while ( found != std::string::npos )
			{
				output.erase(found, 2);
				output.insert(found, 1, '\n');
				found = output.find("\\n");
			}
			strcpy(buf, output.c_str());

			for ( int c = 0; c < MAXPLAYERS; ++c )
			{
				if ( !client_disconnected[c] )
				{
					if ( distanceRequirement != -1 && !(players[c] && players[c]->entity && entityDist(this, players[c]->entity) <= distanceRequirement) )
					{
						// not in range.
					}
					else
					{
						if ( foundPlayerRef != std::string::npos && stats[c] )
						{
							messagePlayerColor(c, color, buf, stats[c]->name);
						}
						else
						{
							messagePlayerColor(c, color, buf);
						}
					}
				}
			}
		}
	}
	else if ( circuit_status == CIRCUIT_OFF )
	{
		textSourceDelay = (textSourceVariables4W >> 16) & 0xFFFF;
		if ( (textSourceVariables4W & 0xFF) == 1 && ((textSourceVariables4W >> 8) & 0xFF) == 0 )
		{
			textSourceVariables4W -= 1;
		}
	}
}

void TextSourceScript::updateClientInformation(int player, bool clearInventory, bool clearStats, ClientInformationType updateType)
{
	if ( multiplayer != SERVER )
	{
		return;
	}
	if ( !stats[player] || client_disconnected[player] )
	{
		return;
	}

	if ( updateType == CLIENT_UPDATE_ALL )
	{
		// update client attributes
		strcpy((char*)net_packet->data, "SCRU");
		net_packet->data[4] = clientnum;
		net_packet->data[5] = (Sint8)stats[player]->STR;
		net_packet->data[6] = (Sint8)stats[player]->DEX;
		net_packet->data[7] = (Sint8)stats[player]->CON;
		net_packet->data[8] = (Sint8)stats[player]->INT;
		net_packet->data[9] = (Sint8)stats[player]->PER;
		net_packet->data[10] = (Sint8)stats[player]->CHR;
		net_packet->data[11] = (Sint8)stats[player]->EXP;
		net_packet->data[12] = (Sint8)stats[player]->LVL;
		SDLNet_Write16((Sint16)stats[player]->HP, &net_packet->data[13]);
		SDLNet_Write16((Sint16)stats[player]->MAXHP, &net_packet->data[15]);
		SDLNet_Write16((Sint16)stats[player]->MP, &net_packet->data[17]);
		SDLNet_Write16((Sint16)stats[player]->MAXMP, &net_packet->data[19]);
		SDLNet_Write32((Sint32)stats[player]->GOLD, &net_packet->data[21]);
		if ( clearInventory )
		{
			net_packet->data[25] = 1;
		}
		else
		{
			net_packet->data[25] = 0;
		}
		if ( clearStats )
		{
			net_packet->data[26] = 1;
		}
		else
		{
			net_packet->data[26] = 0;
		}

		for ( int i = 0; i < NUMPROFICIENCIES; ++i )
		{
			net_packet->data[27 + i] = (Uint8)stats[player]->PROFICIENCIES[i];
		}
		net_packet->address.host = net_clients[player - 1].host;
		net_packet->address.port = net_clients[player - 1].port;
		net_packet->len = 27 + NUMPROFICIENCIES;
		sendPacketSafe(net_sock, -1, net_packet, player - 1);

		serverUpdatePlayerLVL();
	}
	else if ( updateType == CLIENT_UPDATE_CLASS )
	{
		strcpy((char*)net_packet->data, "SCRC");
		for ( int i = 0; i < MAXPLAYERS; ++i )
		{
			net_packet->data[4 + i] = client_classes[i];
		}
		net_packet->address.host = net_clients[player - 1].host;
		net_packet->address.port = net_clients[player - 1].port;
		net_packet->len = 5 + MAXPLAYERS;
		sendPacketSafe(net_sock, -1, net_packet, player - 1);
	}
}

void TextSourceScript::playerClearInventory(bool clearStats)
{
	deinitShapeshiftHotbar();
	for ( int c = 0; c < NUM_HOTBAR_ALTERNATES; ++c )
	{
		selected_spell_alternate[c] = NULL;
		hotbarShapeshiftInit[c] = false;
	}
	selected_spell = NULL; //So you don't start off with a spell when the game restarts.
	selected_spell_last_appearance = -1;
	spellcastingAnimationManager_deactivate(&cast_animation);
	stats[clientnum]->freePlayerEquipment();
	list_FreeAll(&stats[clientnum]->inventory);
	shootmode = true;
	appraisal_timer = 0;
	appraisal_item = 0;

	if ( clearStats )
	{
		stats[clientnum]->clearStats();
	}

	this->hasClearedInventory = true;
}