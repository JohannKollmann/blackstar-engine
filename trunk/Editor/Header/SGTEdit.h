

#ifndef __SGTEdit_H__
#define __SGTEdit_H__

#include <list>
#include "wxEdit.h"
#include "EDTIncludes.h"
#include "SGTMain.h"
#include "SGTMessageSystem.h"
#include "SGTGameObject.h"
#include "OgreAxisObject.h"
#include "SGTEntityMaterialInspector.h"
#include "GUISystem.h"

struct SGTEditorSelection
{
	SGTGameObject *mObject;
};

struct SGTMaterialSelection
{
	Ogre::SubEntity *mSubEntity;
	Ogre::BillboardSet *mBillboardSet;
	Ogre::String mOriginalMaterialName;
};

class SGTEdit : public SGTMessageListener
{
	friend class wxOgre;

public:

	enum EEditModes
	{
		NORMAL,
		BRUSH,
		MATERIAL
	};
	enum SGTAxisLock
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

	SGTGameObject* OnInsertObject(SGTGameObject *parent = 0, bool align = false, bool create_only = false);
	SGTGameObject* OnInsertWaypoint(bool align = false, bool create_only = false);
	void OnInsertObjectAsChild();
	void OnDeleteObject();
	void OnCreateObjectGroup();
	void OnSaveObjectGroup();
	void OnCreateChain();
	void OnConnectWaypoints();
	void OnSaveBones();

	void OnSelectObject(float MouseX, float MouseY);
	void OnBrush();
	void OnSelectMaterial(float MouseX, float MouseY);

	void OnSelectResource();
	void ClearPreviewObject();
	void CreatePreviewObject();

	void DeselectMaterial();
	void SelectObject(SGTGameObject *object);
	void DeselectObject(SGTGameObject *object);
	void DeselectAllObjects();
	bool ObjectIsSelected(SGTGameObject *object);

	void ReceiveMessage(SGTMsg &msg);

	void SetCameraMoveSpeed(float factor);
	void SetCameraRotationSpeed(float factor);
	void SetObjectMoveSpeed(float factor);
	void SetObjectRotationSpeed(float factor);

	static void OnToolbarEvent(int toolID, Ogre::String toolname);

	SGTAxisLock mXAxisLock;
	SGTAxisLock mYAxisLock;
	SGTAxisLock mZAxisLock;

	SGTEdit();
	~SGTEdit() { };


	bool mIsActive;		//Wird für Steuerung gebraucht und wird von wxOgre gesetzt.
	bool mBrushMode;	//Public, da es aus Main Menu gesetzt wird!
	bool mMaterialMode;

private:
	std::list<SGTEditorSelection> mSelectedObjects;	//Liste der angeklickten Objekte

	std::list<SGTGameObject*> mPreviewObjects;

	SGTMaterialSelection mCurrentMaterialSelection;

	SGTMain *mRoot;
	wxEdit *mEdit;

	Ogre::SceneNode *mPivotNode;
	Ogre::SceneNode *mPivotOffsetNode;

	Ogre::Vector2 mMousePosition;

	bool mPerformingObjRot;
	bool mPerformingObjMov;

	bool mPerformedLDClick;

	float mObjectMovSpeed;
	float mObjectRotSpeed;
	float mObjectScaleSpeed;

	float mMovSpeed;
	float mRotSpeed;

	bool mCamRotating;
	bool mLeftDown;
	bool mRightDown;
	bool mAltIsDown;

	bool mMultiSelect;
	bool mInsertAsChild;
	bool mStrgPressed;

	void SelectChildren(SGTGameObject *object);
	void DeselectChildren(SGTGameObject *object);

	void AlignObjectWithMesh(SGTGameObject *object);

	void AttachAxisObject(SGTGameObject *object);
	void DetachAxisObject(SGTGameObject *object);

};

#endif