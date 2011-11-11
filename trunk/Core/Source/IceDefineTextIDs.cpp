#include "IceObjectMessageIDs.h"
#include "IceIncludes.h"

namespace Ice
{
	TEXTID_BEGIN_DEF(aObjectMessageIDs)
	TEXTID_DEFINE_CONSTANT(MOVER_START)
	TEXTID_DEFINE_CONSTANT(MOVER_END)
	TEXTID_DEFINE_CONSTANT(MOVER_PASSING_KEY)
	TEXTID_DEFINE_CONSTANT(ENTER_FPS_MODE)
	TEXTID_DEFINE_CONSTANT(LEAVE_FPS_MODE)
	TEXTID_DEFINE_CONSTANT(START_JUMP)
	TEXTID_DEFINE_CONSTANT(END_JUMP)
	TEXTID_DEFINE_CONSTANT(TRIGGER_LEAVE)
	TEXTID_DEFINE_CONSTANT(TRIGGER_ENTER)
	TEXTID_DEFINE_CONSTANT(INPUT_START_JUMP)
	TEXTID_DEFINE_CONSTANT(UPDATE_CHARACTER_MOVEMENTSTATE)
	TEXTID_DEFINE_CONSTANT(ENTER_MOVEMENT_STATE)
	TEXTID_DEFINE_CONSTANT(LEAVE_MOVEMENT_STATE)
	TEXTID_DEFINE_CONSTANT(CHARACTER_KILL)
	TEXTID_DEFINE_CONSTANT(CHARACTER_COLLISION)
	TEXTID_DEFINE_CONSTANT(INTERN_RESET)
	TEXTID_DEFINE_CONSTANT(AI_SEE)
	TEXTID_DEFINE_CONSTANT(AI_HEAR)
	TEXTID_END_DEF
	
	TextID& getObjectMessageIDs()
	{
		static TextID theOneAndOnly(aObjectMessageIDs, OBJECT_MESSAGE_ID_FIRST, OBJECT_MESSAGE_ID_LAST, TEXTID_DEFS_SIZE(aObjectMessageIDs));
		return theOneAndOnly;
	}

	TEXTID_BEGIN_DEF(aGlobalMessageIDs)
	TEXTID_DEFINE_CONSTANT(UPDATE_PER_FRAME)
	TEXTID_DEFINE_CONSTANT(RENDERING_BEGIN)
	TEXTID_DEFINE_CONSTANT(PHYSICS_BEGIN)
	TEXTID_DEFINE_CONSTANT(PHYSICS_SUBSTEP)
	TEXTID_DEFINE_CONSTANT(PHYSICS_END)
	TEXTID_DEFINE_CONSTANT(KEY_DOWN)
	TEXTID_DEFINE_CONSTANT(KEY_UP)
	TEXTID_DEFINE_CONSTANT(MOUSE_DOWN)
	TEXTID_DEFINE_CONSTANT(MOUSE_UP)
	TEXTID_DEFINE_CONSTANT(MOUSE_MOVE)
	TEXTID_DEFINE_CONSTANT(ACTOR_ONSLEEP)
	TEXTID_DEFINE_CONSTANT(ACTOR_ONWAKE)
	TEXTID_DEFINE_CONSTANT(MATERIAL_ONCONTACT)
	TEXTID_DEFINE_CONSTANT(REPARSE_SCRIPTS_PRE)
	TEXTID_DEFINE_CONSTANT(REPARSE_SCRIPTS_POST)
	TEXTID_DEFINE_CONSTANT(LOADLEVEL_BEGIN)
	TEXTID_DEFINE_CONSTANT(LOADLEVEL_END)
	TEXTID_DEFINE_CONSTANT(SAVELEVEL_BEGIN)
	TEXTID_DEFINE_CONSTANT(SAVELEVEL_END)
	TEXTID_DEFINE_CONSTANT(ENABLE_GAME_CLOCK)
	TEXTID_DEFINE_CONSTANT(GAMESTATE_ENTER)
	TEXTID_DEFINE_CONSTANT(GAMESTATE_LEAVE)
	TEXTID_DEFINE_CONSTANT(CONSOLE_INGAME)
	TEXTID_DEFINE_CONSTANT(UPDATE_INDEPENDANT)
	TEXTID_DEFINE_CONSTANT(DAYNIGHT_CHANGE)
	TEXTID_END_DEF

	TextID& getGlobalMessageIDs()
	{
		static TextID theOneAndOnly(aGlobalMessageIDs, GLOBAL_MESSAGE_ID_FIRST, GLOBAL_MESSAGE_ID_LAST, TEXTID_DEFS_SIZE(aGlobalMessageIDs));
		return theOneAndOnly;
	}
}