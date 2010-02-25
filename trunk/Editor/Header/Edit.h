

#pragma once

#include <list>
#include "wxEdit.h"
#include "EDTIncludes.h"
#include "IceMain.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "OgreAxisObject.h"
#include "EntityMaterialInspector.h"
#include "IceGOCPlayerInput.h"
#include "IceGOCCameraController.h"

class IEditorSelection
{
	virtual void SetGlobalPosition(Ogre::Vector3 pos) = 0;
	virtual void SetGlobalOrientation(Ogre::Quaternion quat) = 0;
};
struct EditorSelection
{
	Ice::GameObject *mObject;
};

struct MaterialSelection
{
	Ogre::SubEntity *mSubEntity;
	Ogre::BillboardSet *mBillboardSet;
	Ogre::String mOriginalMaterialName;
};

class Edit : public Ice::MessageListener
{
	friend class wxOgre;

public:

	enum EEditModes
	{
		NORMAL,
		BRUSH,
		MATERIAL
	};
	enum AxisLock
	{
		LOCKED,
		UNLOCKED,
		UNLOCKED_SKIPCHILDREN
	};
	void OnMouseEvent(wxMouseEvent& ev);

	/*
	Wird von OIS (aus main) aufgerufen und hat einige Vorteile gegenbüber dem wxWidgets Mouse Event System.
	*/
	void OnMouseMove(Ogre::Radian RotX,Ogre::Radian RotY);

	void OnKeyDown(wxKeyEvent& key);
	void OnKeyUp(wxKeyEvent& key);

	void OnLoadWorld(Ogre::String fileName);
	void OnSaveWorld(Ogre::String fileName);

	Ice::GameObject* OnInsertObject(Ice::GameObject *parent = 0, bool align = false, bool create_only = false);
	Ice::GameObject* OnInsertWaypoint(bool align = false, bool create_only = false);
	Ice::GameObject* InsertWayTriangle(bool align = false, bool create_only = false);
	void OnInsertObjectAsChild();
	void OnDeleteObject();
	void OnCreateObjectGroup();
	void OnSaveObjectGroup();
	void OnCreateChain();
	void OnConnectWaypoints();
	void OnSaveBones();

	void OnSetFocus(bool focus);

	void OnSelectObject(float MouseX, float MouseY);
	void OnBrush();
	void OnSelectMaterial(float MouseX, float MouseY);

	void OnSelectResource();
	void ClearPreviewObject();
	void CreatePreviewObject();

	void DeselectMaterial();
	void SelectObject(Ice::GameObject *object);
	void DeselectObject(Ice::GameObject *object);
	void DeselectAllObjects();
	bool ObjectIsSelected(Ice::GameObject *object);

	void ReceiveMessage(Ice::Msg &msg);

	void SetCameraMoveSpeed(float factor);
	void SetCameraRotationSpeed(float factor);
	void SetObjectMoveSpeed(float factor);
	void SetObjectRotationSpeed(float factor);

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	AxisLock mXAxisLock;
	AxisLock mYAxisLock;
	AxisLock mZAxisLock;

	Edit();
	~Edit() { };


	bool mIsActive;		//Wird für Steuerung gebraucht und wird von wxOgre gesetzt.
	bool mBrushMode;	//Public, da es aus Main Menu gesetzt wird!
	bool mMaterialMode;

private:
	std::list<EditorSelection> mSelectedObjects;	//Liste der angeklickten Objekte

	std::list<Ice::GameObject*> mPreviewObjects;

	MaterialSelection mCurrentMaterialSelection;

	Ice::Main *mRoot;
	wxEdit *mEdit;

	Ogre::SceneNode *mPivotNode;
	Ogre::SceneNode *mPivotOffsetNode;

	//Winapi Hack!
	Ogre::Vector2 mWinMousePosition;
	bool mMouseLocked;
	bool mShowMouse;
	void LockAndHideMouse();
	void FreeAndShowMouse();

	bool mPlaying;
	Ice::GOCCameraController *mPlayerCamera;
	Ice::GOCPlayerInput *mPlayerInput;
	void PlayGame();
	void PauseGame();

	Ogre::Vector2 mMousePosition;

	bool mPerformingObjRot;
	bool mPerformingObjMov;

	bool mPerformedLDClick;

	float mObjectMovSpeed;
	float mObjectRotSpeed;
	float mObjectScaleSpeed;

	bool mAlignObjectWithMesh;

	float mMovSpeed;
	float mRotSpeed;

	bool mCamRotating;
	bool mLeftDown;
	bool mRightDown;
	bool mAltIsDown;

	bool mMultiSelect;
	bool mInsertAsChild;
	bool mStrgPressed;

	void SelectChildren(Ice::GameObject *object);
	void DeselectChildren(Ice::GameObject *object);

	void AlignObjectWithMesh(Ice::GameObject *object, bool rotate = false);

	void AttachAxisObject(Ice::GameObject *object);
	void DetachAxisObject(Ice::GameObject *object);

};
