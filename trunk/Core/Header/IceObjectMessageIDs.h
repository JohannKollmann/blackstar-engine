#pragma once
#include "IceTextIDs.h"

namespace Ice
{
	enum ObjectMessageIDs
	{
		MOVER_START = 0,
		MOVER_END,
		MOVER_PASSING_KEY,
		ENTER_FPS_MODE,
		LEAVE_FPS_MODE,
		START_JUMP,
		END_JUMP,
		TRIGGER_LEAVE,
		TRIGGER_ENTER,
		INPUT_START_JUMP,
		UPDATE_CHARACTER_MOVEMENTSTATE,
		ENTER_MOVEMENT_STATE,
		LEAVE_MOVEMENT_STATE,
		CHARACTER_KILL,
		CHARACTER_COLLISION,
		INTERN_RESET,
		OBJECT_MESSAGE_ID_SIZE
	};

	TextID& getObjectMessageIDs();
}