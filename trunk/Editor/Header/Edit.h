

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
#include "wxOgre.h"
#include "wx/msw/winundef.h"
#include "boost/thread.hpp"

#define STOP_MAINLOOP wxEdit::Instance().GetOgrePane()->IncBlockEngineLoop();

#define RESUME_MAINLOOP wxEdit::Instance().GetOgrePane()->DecBlockEngineLoop();

class IEditorSelection
{
	virtual void SetGlobalPosition(Ogre::Vector3 pos) = 0;
	virtual void SetGlobalOrientation(Ogre::Quaternion quat) = 0;
};
struct EditorSelection
{
	Ice::GameObjectPtr mObject;
};

struct MaterialSelection
{
	Ogre::SubEntity *mSubEntity;
	Ogre::BillboardSet *mBillboardSet;
	Ogre::String mOriginalMaterialName;
};

class Edit : public wxOgre, public Ice::SynchronizedMessageListener
{
	DECLARE_CLASS(Edit)

protected:
	DECLARE_EVENT_TABLE()

	void OnRender();

	void OnSize(wxSizeEvent& event);

	boost::mutex mBlockEngineLoopCond;
	int mEngineLoopBlockers;

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

	struct MoverReset
	{
		std::weak_ptr<Ice::GameObject> Mover;
		Ogre::Vector3 ResetPos;
		Ogre::Quaternion ResetQuat;
	};

	void IncBlockEngineLoop();
	void DecBlockEngineLoop();

	void PostInit();

	void OnMouseEvent(wxMouseEvent& ev);

	void OnKeyDown(wxKeyEvent& key);
	void OnKeyUp(wxKeyEvent& key);

	void OnSetFocus(wxFocusEvent& event);
	void OnKillFocus(wxFocusEvent& event);

	void OnInsertObject(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnInsertObjectAsChild(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnDeleteObject(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnCreateObjectGroup(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnSaveObjectGroup(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnSaveBones(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnInsertAnimKey(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnMoverInsertKey(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnTriggerMover(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnPauseMover(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnStopMover(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnSetLookAtObject(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnSetNormalLookAtObject(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnComputeAO(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());
	void OnCreateJoint(wxCommandEvent& WXUNUSED(event) = wxCommandEvent());


	/*
	Wird von OIS (aus main) aufgerufen und hat einige Vorteile gegenbüber dem wxWidgets Mouse Event System.
	*/
	void OnMouseMove(Ogre::Radian RotX,Ogre::Radian RotY);

	void OnLoadWorld(Ogre::String fileName);
	void OnSaveWorld(Ogre::String fileName);

	Ice::GameObjectPtr InsertObject(Ice::GameObjectPtr parent = Ice::GameObjectPtr(), bool align = false, bool create_only = false);
	Ice::GameObjectPtr InsertWaypoint(bool align = false, bool create_only = false);

	void OnSelectObject(float MouseX, float MouseY);
	void OnBrush();
	void OnSelectMaterial(float MouseX, float MouseY);

	void OnSelectResource();
	void ClearPreviewObject();
	void CreatePreviewObject();

	Ice::GameObjectPtr CreateGameObject(std::vector<ComponentSectionPtr> componentParameters);

	void DeselectMaterial();
	void SelectObject(Ice::GameObjectPtr object);
	void DeselectObject(Ice::GameObjectPtr object);
	void DeselectAllObjects();
	bool ObjectIsSelected(Ice::GameObjectPtr object);

	void BrushMultitexture(unsigned int textureLayer);

	void ReceiveMessage(Ice::Msg &msg);

	void SetCameraMoveSpeed(float factor);
	void SetCameraRotationSpeed(float factor);
	void SetObjectMoveSpeed(float factor);
	void SetObjectRotationSpeed(float factor);

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	AxisLock mXAxisLock;
	AxisLock mYAxisLock;
	AxisLock mZAxisLock;

	Edit(wxWindow* parent);
	~Edit() { };


	bool mIsActive;		//Wird für Steuerung gebraucht und wird von wxOgre gesetzt.
	bool mBrushMode;	//Public, da es aus Main Menu gesetzt wird!
	bool mMaterialMode;
	bool mMultitextureBrushMode;

private:
	std::list<EditorSelection> mSelectedObjects;	//Liste der angeklickten Objekte

	std::list<Ice::GameObjectPtr> mPreviewObjects;

	MaterialSelection mCurrentMaterialSelection;

	Ice::Main *mRoot;
	wxEdit *mEdit;

	Ogre::SceneNode *mPivotNode;
	Ogre::SceneNode *mPivotOffsetNode;

	MoverReset mMoverReset;

	//Winapi Hack!
	Ogre::Vector2 mWinMousePosition;
	bool mMouseLocked;
	bool mShowMouse;
	void LockAndHideMouse();
	void FreeAndShowMouse();

	bool mPlaying;
	bool mFreezeCamera;
	Ice::GOCSimpleCameraController *mPlayerCamera;
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

	void SelectChildren(Ice::GameObjectPtr object);
	void DeselectChildren(Ice::GameObjectPtr object);

	//This is called per frame, for performance reasons GameObjectPtr is passed by reference.
	void AlignObjectWithMesh(const Ice::GameObjectPtr &object, bool rotate = false);

	void AttachAxisObject(Ice::GameObjectPtr object);
	void DetachAxisObject(Ice::GameObjectPtr object);

	Ogre::Vector3 GetInsertPosition();

};
