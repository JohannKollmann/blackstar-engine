
#include "Edit.h"
#include "IceInput.h"
#include "IceCameraController.h"
#include "IceScenemanager.h"
#include "IceObjectLevelRayCaster.h"
#include "IceWeatherController.h"
#include "IceMainLoop.h"
#include "IceGOCEditorInterface.h"
#include "IceGOCWaypoint.h"
#include "IceGOCAnimatedCharacter.h"
#include "OgrePhysX.h"
#include "NxScene.h"
#include "NavMeshEditorNode.h"
#include "IceAIManager.h"
#include "IceGOCMover.h"
#include "IceGOCOgreNode.h"

IMPLEMENT_CLASS(Edit, wxOgre)

enum
{
	EVT_InsertObject,
	EVT_InsertObjectAsChild,
	EVT_DeleteObject,
	EVT_CreateObjectgroup,
	EVT_SaveObjectgroup,
	EVT_SaveBones,
	EVT_CreateWayTriangle,
	EVT_InsertAnimKey,
	EVT_MoverInserKey,
	EVT_TriggerMover,
	EVT_PauseMover,
	EVT_StopMover,
	EVT_SetLookAtObject,
	EVT_SetNormalLookAtObject
};

BEGIN_EVENT_TABLE(Edit, wxOgre)
	EVT_KEY_DOWN(Edit::OnKeyDown)
	EVT_KEY_UP(Edit::OnKeyUp)

	EVT_SET_FOCUS(Edit::OnSetFocus)
	EVT_KILL_FOCUS(Edit::OnKillFocus)

	EVT_MOUSE_EVENTS(Edit::OnMouseEvent)

	EVT_MENU(EVT_InsertObject, Edit::OnInsertObject)
	EVT_MENU(EVT_InsertObjectAsChild, Edit::OnInsertObjectAsChild)
	EVT_MENU(EVT_DeleteObject, Edit::OnDeleteObject)
	EVT_MENU(EVT_CreateObjectgroup, Edit::OnCreateObjectGroup)
	EVT_MENU(EVT_SaveObjectgroup, Edit::OnSaveObjectGroup)
	EVT_MENU(EVT_SaveBones, Edit::OnSaveBones)
	EVT_MENU(EVT_CreateWayTriangle, Edit::OnCreateWayTriangle)
	EVT_MENU(EVT_InsertAnimKey, Edit::OnInsertAnimKey)
	EVT_MENU(EVT_MoverInserKey, Edit::OnMoverInsertKey)
	EVT_MENU(EVT_TriggerMover, Edit::OnTriggerMover)
	EVT_MENU(EVT_PauseMover, Edit::OnPauseMover)
	EVT_MENU(EVT_StopMover, Edit::OnStopMover)
	EVT_MENU(EVT_SetLookAtObject, Edit::OnSetLookAtObject)
	EVT_MENU(EVT_SetNormalLookAtObject, Edit::OnSetNormalLookAtObject)

	END_EVENT_TABLE()

Edit::Edit(wxWindow* parent) : wxOgre(parent, -1)
{
	mEdit = &wxEdit::Instance();

	mCamRotating = false;
	mLeftDown = false;
	mRightDown = false;
	mMultiSelect = false;
	mInsertAsChild = false;
	mStrgPressed = false;
	mAltIsDown = false;

	mMouseLocked = false;
	mPlaying = false;

	mMaterialMode = false;
	mBrushMode = false;

	mFreezeCamera = false;

	mAlignObjectWithMesh = false;

	mXAxisLock = AxisLock::LOCKED;
	mYAxisLock = AxisLock::LOCKED;
	mZAxisLock = AxisLock::LOCKED;

	mPerformingObjRot = false;
	mPerformingObjMov = false;

	mPerformedLDClick = false;

	mObjectMovSpeed = 1.0f;
	mObjectRotSpeed = 0.3f;

	mObjectScaleSpeed = 0.0002f;

	mMovSpeed = 10.0f;
	mRotSpeed = 0.2f;

	mCurrentMaterialSelection.mSubEntity = NULL;
	mCurrentMaterialSelection.mBillboardSet = NULL;
	mCurrentMaterialSelection.mOriginalMaterialName = "";

	mMoverReset.mover = nullptr;
};

void Edit::PostInit()
{
	wxEdit::Instance().GetMainToolbar()->RegisterTool("Physics", "WorldSettings", "Data/Editor/Intern/Editor_gewicht_04.png", Edit::OnToolbarEvent, "Enables/Disables Physics simulation", true, true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("TimeCycle", "WorldSettings", "Data/Editor/Intern/editor_clock.png", Edit::OnToolbarEvent, "Enables/Disables game clock", true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("WorldSettings", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("XAxisLock", "Transform", "Data/Editor/Intern/xAxis.png", Edit::OnToolbarEvent, "Enables/Disables X Axis lock for object movements/rotations", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("YAxisLock", "Transform", "Data/Editor/Intern/yAxis.png", Edit::OnToolbarEvent, "Enables/Disables Y Axis lock for object movements/rotations", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("ZAxisLock", "Transform", "Data/Editor/Intern/zAxis.png", Edit::OnToolbarEvent, "Enables/Disables Z Axis lock for object movements/rotations", true);
	//wxEdit::Instance().GetMainToolbar()->RegisterTool("MoveRelative", "Transform", "Data/Editor/Intern/BrushMode.png", Edit::OnToolbarEvent, true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Transform", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("BrushMode", "EditMode", "Data/Editor/Intern/BrushMode.png", Edit::OnToolbarEvent, "Enables/Disables the Brush mode for object placement", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("MaterialMode", "EditMode", "Data/Editor/Intern/materialmode.png", Edit::OnToolbarEvent, "Enables/Disables the Material mode for material selection", true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("EditMode", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("InsertObject", "ObjectMgr", "Data/Editor/Intern/Engine_Icon02.png", Edit::OnToolbarEvent, "Inserts an object");
	//wxEdit::Instance().GetMainToolbar()->RegisterTool("InsertObjectAsChild", "ObjectMgr", "Data/Editor/Intern/BrushMode.png", Edit::OnToolbarEvent);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("DeleteObject", "ObjectMgr", "Data/Editor/Intern/Engine_Icon04.png", Edit::OnToolbarEvent, "Removes an object");
	wxEdit::Instance().GetMainToolbar()->RegisterTool("SaveObjectGroup", "ObjectMgr", "Data/Editor/Intern/Engine_Icon06.png", Edit::OnToolbarEvent, "Save Object group");
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ObjectMgr", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("PlayGame", "Game", "Data/Editor/Intern/Icon_Play.png", Edit::OnToolbarEvent, "Play!");
	wxEdit::Instance().GetMainToolbar()->RegisterTool("StopGame", "Game", "Data/Editor/Intern/Icon_Stop.png", Edit::OnToolbarEvent, "Reset everything to the state before you started playing.");
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", false);

	Ice::AIManager::Instance().SetWayMeshLoadingMode(true);

	SetCameraMoveSpeed(mMovSpeed);
	SetCameraRotationSpeed(mRotSpeed);

	//OIS Maus Input ist im embedded Mode unzuverlässig. Wir übernehmen.
	Ice::Main::Instance().GetCameraController()->mMove = true;
	Ice::Main::Instance().GetCameraController()->mYRot = false;
	Ice::Main::Instance().GetCameraController()->mXRot = false;

	mPivotNode = Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mPivotOffsetNode = mPivotNode->createChildSceneNode();

	Ice::MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	Ice::Main::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode("EditorPreview");
}

void Edit::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	Edit *edit = wxEdit::Instance().GetOgrePane();
	bool checked = wxEdit::Instance().GetMainToolbar()->GetToolState(toolID);
	if (toolname == "XAxisLock")
	{
		if (checked) edit->mXAxisLock = AxisLock::UNLOCKED;
		else edit->mXAxisLock = AxisLock::LOCKED;
	}
	if (toolname == "YAxisLock")
	{
		if (checked) edit->mYAxisLock = AxisLock::UNLOCKED;
		else edit->mYAxisLock = AxisLock::LOCKED;
	}
	if (toolname == "ZAxisLock")
	{
		if (checked) edit->mZAxisLock = AxisLock::UNLOCKED;
		else edit->mZAxisLock = AxisLock::LOCKED;
	}

	if (toolname == "Physics")
	{
		Ice::MainLoop::Instance().SetPhysics(checked);
	}
	if (toolname == "TimeCycle")
	{
		Ice::SceneManager::Instance().EnableClock(checked);
	}
	if (toolname == "BrushMode")
	{
		edit->mBrushMode = checked;
		edit->ClearPreviewObject();
		if (checked) edit->CreatePreviewObject();
	}
	if (toolname == "MaterialMode")
	{
		edit->mMaterialMode = checked;
	}
	if (toolname == "InsertObject")
	{
		edit->OnInsertObject();
	}
	if (toolname == "DeleteObject")
	{
		edit->OnDeleteObject();
	}
	if (toolname == "SaveObjectGroup")
	{
		edit->OnSaveObjectGroup();
	}
	if (toolname == "PlayGame")
	{
		edit->PlayGame();
	}
}

void Edit::PauseGame()
{
	if (mPlaying)
	{
		mPlayerCamera->DetachCamera();
		Ice::Main::Instance().GetCamera()->setPosition(mPlayerCamera->GetOwner()->GetGlobalPosition() + Ogre::Vector3(0, 2, -4));
		Ice::Main::Instance().GetCamera()->setDirection(0, 0, 1);
		mPlayerInput->SetActive(false);
		mPlayerCamera = 0;
		mPlayerInput = 0;
		mPlaying = false;
		Ice::Main::Instance().GetCameraController()->mMove = true;
		FreeAndShowMouse();
	}
}

void Edit::PlayGame()
{
	SetFocus();

	if (mSelectedObjects.size () != 1) return;

	Ice::GameObject* obj = (*mSelectedObjects.begin()).mObject;
	Ice::GOCSimpleCameraController *cam = (Ice::GOCSimpleCameraController*)obj->GetComponent("Camera");
	Ice::GOCPlayerInput *input = (Ice::GOCPlayerInput*)obj->GetComponent("CharacterInput", "PlayerInput");
	if (cam && input)
	{
		mPlayerCamera = cam;
		mPlayerInput = input;
		POINT p;
		GetCursorPos(&p);
		SetCursorPos(p.x, p.y + 100);
		LockAndHideMouse();
		cam->AttachCamera(Ice::Main::Instance().GetCamera());
		input->SetActive(true);
		Ice::Main::Instance().GetCameraController()->mMove = false;
		mPlaying = true;
		DeselectAllObjects();
	}
}

void Edit::LockAndHideMouse()
{
	if (mShowMouse) ShowCursor(false);
	POINT p;
	GetCursorPos(&p);
	mWinMousePosition.x = p.x;
	mWinMousePosition.y = p.y;
	mMouseLocked = true;
	mShowMouse = false;
}
void Edit::FreeAndShowMouse()
{
	if (!mShowMouse) ShowCursor(true);
	SetCursorPos(mWinMousePosition.x, mWinMousePosition.y);
	mMouseLocked = false;
	mShowMouse = true;
}

void Edit::OnCreateWayTriangle( wxCommandEvent& WXUNUSED(event) )
{
	if (mSelectedObjects.size() == 2)
	{
		Ice::GameObject* obj1 = (*mSelectedObjects.begin()).mObject;
		Ice::GameObject* obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;
		if (obj1->GetComponent<NavMeshEditorNode>() && obj2->GetComponent<NavMeshEditorNode>())
		{
			NavMeshEditorNode *n1 = obj1->GetComponent<NavMeshEditorNode>();
			NavMeshEditorNode *n2 = obj2->GetComponent<NavMeshEditorNode>();
			if (n1->GetType() == NavMeshEditorNode::NODE && n2->GetType() == NavMeshEditorNode::EDGE)
			{
				NavMeshEditorNode *n3 = n2->GetTriangles()[0].n1.neighbour;
				NavMeshEditorNode *n4 = n2->GetTriangles()[0].n2.neighbour;
				DeselectAllObjects();
				n1->AddTriangle(n3, n4);
				n1->UpdatePosition(n1->GetOwner()->GetGlobalPosition());
			}
			if (n2->GetType() == NavMeshEditorNode::NODE && n1->GetType() == NavMeshEditorNode::EDGE)
			{
				NavMeshEditorNode *n3 = n1->GetTriangles()[0].n1.neighbour;
				NavMeshEditorNode *n4 = n1->GetTriangles()[0].n2.neighbour;
				DeselectAllObjects();
				n2->AddTriangle(n3, n4);
				n2->UpdatePosition(n2->GetOwner()->GetGlobalPosition());
			}
		}
	}
}

void Edit::OnMouseEvent(wxMouseEvent &ev)
{
	mLeftDown = ev.LeftIsDown();
	mRightDown = ev.RightIsDown();

	if (mMouseLocked)
	{
		SetCursorPos(mWinMousePosition.x, mWinMousePosition.y);
		if (mPlaying) return;
	}

	if (ev.LeftIsDown() || ev.RightIsDown())
	{
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
	/*
	Sicherstellen, dass keine Kamerarotation vorliegt oder gerade abgeschlossen wird (sonst Abbruch)
	*/
	if (ev.LeftIsDown() && ev.RightIsDown() && mCamRotating == false)
	{
		mCamRotating = true;
		LockAndHideMouse();
	}
	else if (mCamRotating == true && mLeftDown == false && mRightDown == false)
	{
		mCamRotating = false;
		FreeAndShowMouse();
		return;
	}
	if (mCamRotating == true) return;

	mMousePosition.x = (float)(ev.GetX()) / Ice::Main::Instance().GetWindow()->getWidth();
	mMousePosition.y = (float)(ev.GetY()) / Ice::Main::Instance().GetWindow()->getHeight();

	if (ev.RightUp())
	{
		if (mPerformingObjRot == false)
		{
			wxPoint clientpt = ev.GetPosition();
			wxMenu menu("");

			if (true)
			{
				if (mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() != "None"
					&& mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find("Joint") == Ogre::String::npos)
				{
					menu.Append(EVT_InsertObject, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
					if (mSelectedObjects.size() > 0)
					{
						menu.Append(EVT_InsertObjectAsChild, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() + " as Child of " +  mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject->GetName());
					}
				}

				if (mSelectedObjects.size() > 1) menu.Append(EVT_CreateObjectgroup, "Merge");
				if (mSelectedObjects.size() == 2)		//Dyadic operators
				{
					Ice::GameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					Ice::GameObject* obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;

					if (obj1->GetComponent<Ice::GOCMover>())
					{
						menu.Append(EVT_SetLookAtObject, "Look At");
						menu.Append(EVT_SetNormalLookAtObject, "Look At (Normal)");
					}

					if (obj1->GetComponent<NavMeshEditorNode>() && obj2->GetComponent<NavMeshEditorNode>())
					{
						NavMeshEditorNode *n1 = obj1->GetComponent<NavMeshEditorNode>();
						NavMeshEditorNode *n2 = obj2->GetComponent<NavMeshEditorNode>();
						if (n1->GetType() != n2->GetType()) menu.Append(EVT_CreateWayTriangle, "Create Triangle");
					}
				}
				if (mSelectedObjects.size() == 1)
				{
					menu.Append(EVT_SaveObjectgroup, "Save Object Group");

					Ice::GameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					if (obj1->GetComponent<Ice::GOCMover>())
					{
						menu.Append(EVT_SetLookAtObject, "Clear Look At");
						menu.Append(EVT_SetNormalLookAtObject, "Clear Look At (Normal)");
					}

					if (obj1->GetComponent("View", "Skeleton"))
					{
						menu.Append(EVT_SaveBones, "Save Bone Config");
					}
					if (obj1->GetComponent<Ice::AnimKey>())
						menu.Append(EVT_InsertAnimKey, "New Anim Key");
					if (obj1->GetComponent<Ice::GOCMover>())
						menu.Append(EVT_MoverInserKey, "New Anim Key");
					if (obj1->GetComponent<Ice::GOCMover>())
						menu.Append(EVT_TriggerMover, "Test Mover");
					if (obj1->GetComponent<Ice::GOCMover>())
						menu.Append(EVT_PauseMover, "Pause Mover");
					if (obj1->GetComponent<Ice::GOCMover>())
						menu.Append(EVT_StopMover, "Stop Mover");
				}

				if (mSelectedObjects.size() >= 1) menu.Append(EVT_DeleteObject, "Delete");
			}

			mEdit->GetOgrePane()->PopupMenu(&menu, clientpt);

		}
		else
		{
			mPerformingObjRot = false;
			FreeAndShowMouse();
		}

	}
	if (ev.LeftUp())
	{
		if (mPerformingObjMov == false)
		{
			if (!mPerformedLDClick)
			{
				if (mBrushMode)
				{
					OnBrush();
				}
				else
				{
					DeselectMaterial();
					OnSelectObject((float)(ev.GetX()) / Ice::Main::Instance().GetWindow()->getWidth(), (float)(ev.GetY()) / Ice::Main::Instance().GetWindow()->getHeight());
				}
			}
		}
		else
		{
			mPerformingObjMov = false;
			FreeAndShowMouse();
		}
		mPerformedLDClick = false;
	}
	if (ev.LeftDClick())
	{
		if (mMaterialMode)
		{
			mPerformedLDClick = true;
			OnSelectMaterial((float)(ev.GetX()) / Ice::Main::Instance().GetWindow()->getWidth(), (float)(ev.GetY()) / Ice::Main::Instance().GetWindow()->getHeight());
		}
		if (mBrushMode)
		{
			//OnBrush();
			mPerformedLDClick = true;
		}
	}
	if (ev.m_wheelRotation != 0 && mSelectedObjects.size() > 0)
	{
		if (mXAxisLock == AxisLock::UNLOCKED || mYAxisLock == AxisLock::UNLOCKED || mZAxisLock == AxisLock::UNLOCKED)
		{
			for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				/*Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)(*i).mObject->GetComponent("View");
				if (visuals != 0)
				{
					if (visuals->GetComponentID() == "ViewContainer")
					{
						Ice::MeshRenderable *gocmesh = (Ice::MeshRenderable*)((Ice::GOCViewContainer*)visuals)->GetItem("MeshRenderable");
						if (gocmesh != 0)
						{
							mPerformingObjMov = true;
							Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mYAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mZAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f);	
							(*i).mObject->Rescale(scaleaxis * ev.m_wheelRotation * mObjectScaleSpeed);
							visuals->GetNode()->showBoundingBox(true);
							(*i).mObject->Freeze(true);
							AttachAxisObject((*i).mObject);
						}
					}
					else if (visuals->GetComponentID() == "MeshDebugRenderable")
					{
						visuals->GetNode()->showBoundingBox(true);
					}*/
				mPerformingObjMov = true;
				Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mYAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mZAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f);	
				(*i).mObject->Rescale(scaleaxis * ev.m_wheelRotation * mObjectScaleSpeed);
				(*i).mObject->Freeze(true);
				AttachAxisObject((*i).mObject);
			}
		}
	}

	ev.Skip();
};

void Edit::AttachAxisObject(Ice::GameObject *object)
{
	object->RemoveComponent("AxisObject");
	AxisComponent* caxis = new AxisComponent();
	object->AddComponent(Ice::GOComponentPtr(caxis));
}
void Edit::DetachAxisObject(Ice::GameObject *object)
{
	object->RemoveComponent("AxisObject");
}

void Edit::OnMouseMove(Ogre::Radian RotX,Ogre::Radian RotY)
{
	if (mMouseLocked)
	{
		SetCursorPos(mWinMousePosition.x, mWinMousePosition.y);
		if (mPlaying) return;
	}
	if (mFreezeCamera) return;
	if (mLeftDown && mRightDown && !mAltIsDown)
	{
		Ice::Main::Instance().GetCamera()->yaw(-RotX * mRotSpeed);
		Ice::Main::Instance().GetCamera()->pitch(-RotY * mRotSpeed);
	}
	else if (mLeftDown && mRightDown && mAltIsDown)
	{
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(0,1,0), -RotX * mRotSpeed);
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(1,0,0), -RotY * mRotSpeed);
		mPivotOffsetNode->needUpdate(true);
		Ice::Main::Instance().GetCamera()->setPosition(mPivotOffsetNode->_getDerivedPosition());
		Ice::Main::Instance().GetCamera()->lookAt(mPivotNode->getPosition());
	}

	else if (mLeftDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == AxisLock::UNLOCKED || mXAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(RotX.valueRadians() * mObjectMovSpeed,0,0), mXAxisLock == AxisLock::UNLOCKED);
				}
			}
			if (mYAxisLock == AxisLock::UNLOCKED || mYAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(Ogre::Vector3(0,-RotY.valueRadians() * mObjectMovSpeed,0), mYAxisLock == AxisLock::UNLOCKED);
				}
			}
			if (mZAxisLock == AxisLock::UNLOCKED || mZAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					Ogre::Vector3 movaxis = Ice::Main::Instance().GetCamera()->getDerivedDirection() * -1.0f;
					movaxis.y = 0;
					movaxis.normalise();
					(*i).mObject->Translate(movaxis * RotY.valueRadians() * mObjectMovSpeed , mZAxisLock == AxisLock::UNLOCKED);
				}
			}
			if (mAlignObjectWithMesh)
			{
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					AlignObjectWithMesh(i->mObject);
				}
			}
		}
	}
	else if (mRightDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == AxisLock::UNLOCKED || mXAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * Ice::Main::Instance().GetCamera()->getDerivedOrientation().xAxis(), RotY * mObjectRotSpeed, mXAxisLock == AxisLock::UNLOCKED);
				}
			}
			if (mYAxisLock == AxisLock::UNLOCKED || mYAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * Ogre::Vector3::UNIT_Y, RotX * mObjectRotSpeed, mYAxisLock == AxisLock::UNLOCKED);
				}
			}
			if (mZAxisLock == AxisLock::UNLOCKED || mZAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				LockAndHideMouse();
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					Ogre::Vector3 rotaxis = Ice::Main::Instance().GetCamera()->getDerivedDirection() * -1.0f;
					rotaxis.y = 0;
					rotaxis.normalise();
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * rotaxis, RotX * mObjectRotSpeed, mZAxisLock == AxisLock::UNLOCKED);
				}
			}
		}
	}
	else
	{
		for (std::list<Ice::GameObject*>::iterator i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
		{
			(*i)->SetGlobalOrientation(Ogre::Quaternion());
			AlignObjectWithMesh(*i);
		}
	}
};

void Edit::OnKeyDown(wxKeyEvent& key)
{
	if (key.GetKeyCode() == wxKeyCode::WXK_SHIFT) mMultiSelect = true;
	if (key.GetKeyCode() == wxKeyCode::WXK_SPACE)
	{
		if (!mBrushMode && !mPlaying)
		{
			ClearPreviewObject();
			CreatePreviewObject();
			mBrushMode = true;
			wxEdit::Instance().GetMainToolbar()->CheckTool("BrushMode", true);
		}		
	}
	if (key.GetKeyCode() == wxKeyCode::WXK_CONTROL)
	{
		mStrgPressed = true;
	}
	if (key.GetKeyCode() == wxKeyCode::WXK_ALT && !mAltIsDown)
	{
		mPivotNode->setOrientation(Ogre::Quaternion());
		if (mSelectedObjects.size() > 0)
		{
			mPivotNode->setPosition(mSelectedObjects.begin()._Mynode()->_Myval.mObject->GetGlobalPosition());
		}
		mPivotOffsetNode->setPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() - mPivotNode->getPosition());
		mAltIsDown = true;
	}

	if (key.GetKeyCode() == 49) mXAxisLock = AxisLock::UNLOCKED;
	if (key.GetKeyCode() == 50) mYAxisLock = AxisLock::UNLOCKED;
	if (key.GetKeyCode() == 51) mZAxisLock = AxisLock::UNLOCKED;
	if (key.GetKeyCode() == 52) mXAxisLock = AxisLock::UNLOCKED_SKIPCHILDREN;
	if (key.GetKeyCode() == 53) mYAxisLock = AxisLock::UNLOCKED_SKIPCHILDREN;
	if (key.GetKeyCode() == 54) mZAxisLock = AxisLock::UNLOCKED_SKIPCHILDREN;
	if (key.GetKeyCode() == 55) mAlignObjectWithMesh = true;
	wxEdit::Instance().GetMainToolbar()->CheckTool("XAxisLock", mXAxisLock != AxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("YAxisLock", mYAxisLock != AxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("ZAxisLock", mZAxisLock != AxisLock::LOCKED);
};

void Edit::OnKeyUp(wxKeyEvent& key)
{
	if (key.GetKeyCode() == wxKeyCode::WXK_SHIFT) mMultiSelect = false;
	if (key.GetKeyCode() == wxKeyCode::WXK_SPACE)
	{
		mBrushMode = false;
		ClearPreviewObject();
		wxEdit::Instance().GetMainToolbar()->CheckTool("BrushMode", false);
	}
	if (key.GetKeyCode() == wxKeyCode::WXK_CONTROL)
	{
		mStrgPressed = false;
	}
	if (key.GetKeyCode() == wxKeyCode::WXK_ALT) mAltIsDown = false;

	if (key.GetKeyCode() == wxKeyCode::WXK_DELETE)
	{
		if (mSelectedObjects.size() >= 1)
		{
			OnDeleteObject();
		}
	}

	if (key.GetKeyCode() == 73 && mStrgPressed == true)
	{
		OnInsertObject();
	}

	if (key.GetKeyCode() == 27)		//Esc
	{
		PauseGame();
	}

	if (key.GetKeyCode() == 49) mXAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 50) mYAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 51) mZAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 52) mXAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 53) mYAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 54) mZAxisLock = AxisLock::LOCKED;
	if (key.GetKeyCode() == 55) mAlignObjectWithMesh = false;
	wxEdit::Instance().GetMainToolbar()->CheckTool("XAxisLock", mXAxisLock != AxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("YAxisLock", mYAxisLock != AxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("ZAxisLock", mZAxisLock != AxisLock::LOCKED);
};

void Edit::OnSelectResource()
{
	if (Ice::Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_RCONTROL) || mBrushMode)
	{
		ClearPreviewObject();
		CreatePreviewObject();
	}
}

void Edit::ClearPreviewObject()
{
	for (std::list<Ice::GameObject*>::iterator i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
	{
		delete (*i);
	}
	mPreviewObjects.clear();
}
void Edit::CreatePreviewObject()
{
	Ice::GameObject *preview = InsertObject(0, true, true);
	if (preview)
	{
		if (preview->GetComponent("Physics")) preview->RemoveComponent("Physics");
		mPreviewObjects.push_back(preview);
	}
}

void Edit::OnLoadWorld(Ogre::String fileName)
{
	STOP_MAINLOOP
	DeselectAllObjects();
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");
	mMoverReset.mover = nullptr;
	Ice::SceneManager::Instance().LoadLevel(fileName);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetOgrePane()->SetFocus();

	NavMeshEditorNode::FromMesh(Ice::AIManager::Instance().GetNavigationMesh());
	RESUME_MAINLOOP
};

void Edit::OnSaveWorld(Ogre::String fileName)
{
	Ice::SceneManager::Instance().SaveLevel(fileName);
};

Ice::GameObject* Edit::InsertWaypoint(bool align, bool create_only)
{
	Ice::GameObject *waypoint = Ice::SceneManager::Instance().CreateGameObject();
	waypoint->AddComponent(Ice::GOComponentPtr(new Ice::GOCWaypoint()));
	waypoint->ShowEditorVisuals(true);
	waypoint->SetGlobalOrientation(Ogre::Quaternion(Ice::Main::Instance().GetCamera()->getDerivedOrientation().getYaw(), Ogre::Vector3(0,1,0)));
	if (align) AlignObjectWithMesh(waypoint);
	else waypoint->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	if (!create_only)
	{
		SelectObject(waypoint);
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(waypoint);
	}
	wxEdit::Instance().GetOgrePane()->SetFocus();
	return waypoint;
}

Ice::GameObject* Edit::InsertWayTriangle(bool align, bool create_only)
{
	Ice::GameObject *oNode1 = new Ice::GameObject();
	Ice::GameObject *oNode2 = new Ice::GameObject();
	Ice::GameObject *oNode3 = new Ice::GameObject();

	NavMeshEditorNode *n1 = new NavMeshEditorNode();
	oNode1->AddComponent(Ice::GOComponentPtr(n1));
	NavMeshEditorNode *n2 = new NavMeshEditorNode();
	oNode2->AddComponent(Ice::GOComponentPtr(n2));
	NavMeshEditorNode *n3 = new NavMeshEditorNode(oNode3, NavMeshEditorNode::NODE, n1, n2);

	if (align)
		AlignObjectWithMesh(oNode1);
	else
		oNode1->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 
	oNode2->SetGlobalPosition(oNode1->GetGlobalPosition() + Ogre::Vector3(1,0,2));
	oNode3->SetGlobalPosition(oNode1->GetGlobalPosition() + Ogre::Vector3(2,0,0));

	if (!create_only)
	{
		SelectObject(oNode1);

		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

		wxEdit::Instance().GetOgrePane()->SetFocus();
	}

	return oNode1;
}

Ice::GameObject* Edit::InsertObject(Ice::GameObject *parent, bool align, bool create_only)
{
	Ice::GameObject *object = 0;
	Ogre::String sResource = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().c_str();
	if (sResource == "None") return nullptr;
	if (sResource.find("Waypoint.static") != Ogre::String::npos)
	{
		return InsertWaypoint(align, create_only);
	}
	else if (sResource.find("WayTriangle.static") != Ogre::String::npos)
	{
		return InsertWayTriangle(align, create_only);
	}
	else if (sResource.find(".") != Ogre::String::npos)
	{
		LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(sResource);
		if (sResource.find(".ot") != Ogre::String::npos)
		{
			object = (Ice::GameObject*)ls->LoadObject();

			if (parent != NULL) object->SetParent(parent);
		}
		else
		{
			object = Ice::SceneManager::Instance().CreateGameObject();
			object->SetParent(parent);

			std::vector<ComponentSection> sections;
			Ogre::Vector3 offset;
			Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
			Ogre::Quaternion orientation;
			ls->LoadAtom("std::vector<ComponentSection>", (void*)(&sections));
			for (auto i = sections.begin(); i != sections.end(); i++)
			{
				if ((*i).mSectionName == "GameObject")
				{
					offset = (*i).mSectionData.GetOgreVec3("Position");
					orientation = (*i).mSectionData.GetOgreQuat("Orientation");
					scale = (*i).mSectionData.GetOgreVec3("Scale");
					object->SetGlobalOrientation(orientation);
					object->SetGlobalScale(scale);
					continue;
				} 
				(*i).mSectionData.AddOgreVec3("Scale", scale);

				Ice::GOCEditorInterface *component = Ice::SceneManager::Instance().NewGOC((*i).mSectionName);
				component->SetParameters(&(*i).mSectionData);
				object->AddComponent(Ice::GOComponentPtr(component->GetGOComponent()));
			}

		}

		ls->CloseFile();
		delete ls;
	}

	if (align) AlignObjectWithMesh(object);
	else object->SetGlobalPosition(GetInsertPosition()); 

	object->ShowEditorVisuals(true);

	if (!create_only)
	{
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(object);
		SelectObject(object);

		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

		wxEdit::Instance().GetOgrePane()->SetFocus();
	}

	return object;
}

void Edit::OnInsertObject( wxCommandEvent& WXUNUSED(event) )
{
	InsertObject();
}

void Edit::OnKillFocus( wxFocusEvent& event )
{
	if (mPlaying)
		PauseGame();
	Ice::Main::Instance().GetInputManager()->SetEnabled(false);
	if (wxEdit::Instance().GetpropertyWindow()->GetCurrentPage())
		wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()->OnUpdate();
}

void Edit::OnSetFocus( wxFocusEvent& event )
{
	Ice::Main::Instance().GetInputManager()->SetEnabled(true);
}

void Edit::OnInsertObjectAsChild( wxCommandEvent& WXUNUSED(event) )
{
	if (mSelectedObjects.size() > 0) InsertObject(mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject);
}

void Edit::OnDeleteObject( wxCommandEvent& WXUNUSED(event) )
{
	wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(true);

	wxEditIceGameObject *propgrid_page = dynamic_cast<wxEditIceGameObject*>(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage());
	if (propgrid_page)
	{
		propgrid_page->SetObject(nullptr);
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");

	bool skip = false;
	bool one = false;
	if (mSelectedObjects.size() == 1)
	{
		one = true;
		Ice::GameObject *object = (*mSelectedObjects.begin()).mObject;

		//check if the object has managed children
		bool hasChildren = false;
		for (unsigned short i = 0; i < object->GetNumChildren(); i++)
		{
			if (object->GetChild(i)->IsManagedByParent())
			{
				hasChildren = true;
				break;
			}
		}
		if (hasChildren)
		{
			skip = true;
			wxMessageDialog dialog( NULL, _T("Delete Children?"),
				_T("Deleting root object..."), wxYES_DEFAULT|wxYES_NO|wxCANCEL|wxICON_QUESTION);
			int input = dialog.ShowModal();
			if (input == wxID_YES)
			{
				if (object == mMoverReset.mover) mMoverReset.mover = nullptr;
				delete object;
			}
			else if (input == wxID_NO)
			{
				while (object->GetNumChildren() > 0)
				{
					object->GetChild(0)->SetParent(object->GetParent());
				}
				if (object == mMoverReset.mover) mMoverReset.mover = nullptr;
				delete object;
			}
			else if (input == wxID_CANCEL) return;
		}
	}
	if (!skip)
	{
		for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
		{
			if (NavMeshEditorNode::IsEdge(i->mObject)) return;
			if (i->mObject->GetComponent<NavMeshEditorNode>() && !one) return;	//Hack - only possible to remove one triangle node
			delete (*i).mObject;
			(*i).mObject = 0;
		}
	}
	mSelectedObjects.clear();
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(false);
}

void Edit::OnCreateObjectGroup( wxCommandEvent& WXUNUSED(event) )
{
	Ice::GameObject *parent = Ice::SceneManager::Instance().CreateGameObject();
	parent->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		(*i).mObject->SetParent(parent);
	}
	DeselectAllObjects();
	SelectObject(parent);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
}

void Edit::OnSaveObjectGroup( wxCommandEvent& WXUNUSED(event) )
{
	if (mSelectedObjects.size() == 1)
	{
		wxFileDialog dialog
			(
			&wxEdit::Instance(),
			"Save Object Tree",
			wxEmptyString,
			wxEmptyString,
			"Object Trees (*.ot)|*.ot",
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT
			);

		dialog.CentreOnParent();
		dialog.SetPath("Data/Editor/Objects/");

		dialog.SetFilterIndex(1);

		wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(true);
		if (dialog.ShowModal() == wxID_OK)
		{
			LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(dialog.GetPath().c_str(), Ogre::String(dialog.GetPath().c_str()) + ".xml");
			Ice::GameObject *go = (*mSelectedObjects.begin()).mObject;
			DeselectAllObjects();
			Ogre::Vector3 position = go->GetGlobalPosition();
			go->SetGlobalPosition(Ogre::Vector3(0,0,0));
			ss->SaveObject(go, "Root");
			ss->CloseFiles();
			go->SetGlobalPosition(position);
			SelectObject(go);
			delete ss;
		}
		wxEdit::Instance().GetMainNotebook()->GetOgreWindow()->SetPaused(false);
	}
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects");
}

void Edit::OnSaveBones( wxCommandEvent& WXUNUSED(event) )
{
	Ice::GOCAnimatedCharacter *ragdoll = (Ice::GOCAnimatedCharacter*)(*mSelectedObjects.begin()).mObject->GetComponent("View", "Skeleton");
	ragdoll->SerialiseBoneObjects("Data/Scripts/Animation/" + ragdoll->GetRagdoll()->getEntity()->getMesh()->getName() + ".bones");
}

void Edit::OnSelectMaterial(float MouseX, float MouseY)
{
	DeselectMaterial();

	//Potenziell unsicher!
	wxEdit::Instance().GetWorldExplorer()->SetSelection(3);

	Ogre::Ray ray = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY);
	Ice::ObjectLevelRayCaster rc(ray);
	Ice::GameObject *object = rc.GetFirstHit(true);
	if (object)
	{
		Ice::GOCMeshRenderable *gocmesh = object->GetComponent<Ice::GOCMeshRenderable>();
		if (gocmesh != 0)
		{
			EntityMaterialInspector emi(gocmesh->GetEntity());
			Ogre::SubEntity *ent = emi.GetSubEntity(ray);
			if (ent == 0) return;
			mCurrentMaterialSelection.mSubEntity = ent;
			mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
			ent->setMaterialName("Editor_Submesh_Selected");
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(((Ogre::Entity*)gocmesh->GetEntity())->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
			wxEdit::Instance().GetOgrePane()->SetFocus();
			return;
		}
	}

	if (Ice::SceneManager::Instance().HasLevelMesh())
	{
		EntityMaterialInspector emi(Ice::Main::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity"));
		Ogre::SubEntity *ent = emi.GetSubEntity(ray);
		if (ent)
		{
			//Ogre::LogManager::getSingleton().logMessage("change material: " + ent->getMaterialName());
			mCurrentMaterialSelection.mSubEntity = ent;
			mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
			ent->setMaterialName("Editor_Submesh_Selected");
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(Ice::Main::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity")->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
			wxEdit::Instance().GetOgrePane()->SetFocus();
		}
	}
}

void Edit::DeselectMaterial()
{
	if (mCurrentMaterialSelection.mSubEntity != NULL)
	{
		mCurrentMaterialSelection.mSubEntity->setMaterialName(mCurrentMaterialSelection.mOriginalMaterialName);
		mCurrentMaterialSelection.mSubEntity = NULL;
		mCurrentMaterialSelection.mBillboardSet = NULL;
		mCurrentMaterialSelection.mOriginalMaterialName = "";
	}
	if (mCurrentMaterialSelection.mBillboardSet != NULL)
	{
		mCurrentMaterialSelection.mBillboardSet->setMaterialName(mCurrentMaterialSelection.mOriginalMaterialName);
		mCurrentMaterialSelection.mBillboardSet = NULL;
		mCurrentMaterialSelection.mSubEntity = NULL;
		mCurrentMaterialSelection.mOriginalMaterialName = "";
	}
}

void Edit::OnSelectObject(float MouseX, float MouseY)
{
	//Ogre::LogManager::getSingleton().logMessage("OnSelectObject");
	Ice::ObjectLevelRayCaster rc(Ice::Main::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY));
	Ice::GameObject *object = rc.GetFirstHit();
	//the user is capable to iterate through the ray direction array
	bool found = false;
	while (object)
	{
		for (auto i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
		{
			if (i->mObject == object)
			{
				found = true;
				break;
			}
		}
		object = rc.GetNextHit();
		if (found) break;
	}
	if (!found || !object) object = rc.GetFirstHit();
	
	if (!mMultiSelect) DeselectAllObjects();
	if (object)
	{
		SelectObject(object);
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
}

void Edit::OnBrush()
{
	InsertObject(0, true);
}

void Edit::SelectObject(Ice::GameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Select Object " + object->GetName());
	if (!mMultiSelect) DeselectAllObjects();
	else DeselectObject(object);

	object->ShowEditorVisuals(true);

	wxEdit::Instance().GetWorldExplorer()->SetSelection(4);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->ExpandToObject(object);
	wxEditIceGameObject *page = (wxEditIceGameObject*)wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	page->SetObject(object);

	if (object->GetComponent("CharacterInput", "PlayerInput") && object->GetComponent("Camera"))
	{
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", true);
		wxEdit::Instance().GetMainToolbar()->SetToolStatus("PlayGame", true);
		wxEdit::Instance().GetMainToolbar()->SetToolStatus("StopGame", false);
	}

	object->Freeze(true);
	Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
	if (visuals != nullptr)
	{
		visuals->GetNode()->showBoundingBox(true);
	}
	EditorSelection sel;
	sel.mObject = object;
	AttachAxisObject(object);
	mSelectedObjects.push_back(sel);
	SelectChildren(object);
	//Ogre::LogManager::getSingleton().logMessage("SelectObject " + object->GetName());
}

void Edit::SelectChildren(Ice::GameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("select Children " + object->GetName());
	for (unsigned short i = 0; i < object->GetNumChildren(); i++)
	{
		Ice::GameObject *child = object->GetChild(i);
		child->Freeze(true);
		Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(true);
		}
		SelectChildren(child);
	}
}

void Edit::DeselectChildren(Ice::GameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Deselect Children " + object->GetName());
	for (unsigned short i = 0; i < object->GetNumChildren(); i++)
	{
		Ice::GameObject *child = object->GetChild(i);
		child->Freeze(false);
		Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(false);
		}
		DeselectChildren(child);
	}
}

bool Edit::ObjectIsSelected(Ice::GameObject *object)
{
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		if ((*i).mObject == object)
		{
			return true;
		}
	}
	return false;
}

void Edit::DeselectObject(Ice::GameObject *object)
{
	if (object->GetComponent("CharacterInput", "PlayerInput") && object->GetComponent("Camera"))
	{
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", false);
	}
	//Ogre::LogManager::getSingleton().logMessage("Deselect Object " + object->GetName());
	DeselectChildren(object);
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		if ((*i).mObject == object)
		{
			DetachAxisObject((*i).mObject);
			Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
			if (visuals != 0)
			{
				visuals->GetNode()->showBoundingBox(false);
			}
			(*i).mObject->Freeze(false);
			mSelectedObjects.erase(i);
			return;
		}
	}
}

void Edit::DeselectAllObjects()
{
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", false);
	//Ogre::LogManager::getSingleton().logMessage("Deselect all Objects");
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		DetachAxisObject((*i).mObject);
		Ice::GOCOgreNode *visuals = (*i).mObject->GetComponent<Ice::GOCOgreNode>();
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(false);
		}
		(*i).mObject->Freeze(false);
		DeselectChildren((*i).mObject);
	}
	mSelectedObjects.clear();
}

void Edit::AlignObjectWithMesh(Ice::GameObject *object, bool rotate)
{
	Ogre::Ray mouseRay = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(mMousePosition.x, mMousePosition.y);
	OgrePhysX::Scene::QueryHit report;
	std::vector<OgrePhysX::Scene::QueryHit> lReport;
	bool found = false;
	float shortest = 999999;
	Ice::Main::Instance().GetPhysXScene()->raycastAllShapes(lReport, mouseRay);
	for (std::vector<OgrePhysX::Scene::QueryHit>::iterator i = lReport.begin(); i != lReport.end(); i++)
	{
		if ((*i).hitActor->userData != object && i->distance < shortest)
		{
			report = *i;
			found = true;
			shortest = i->distance;
		}
	}
  	if (found)
	{
		Ogre::Vector3 position = report.point;
		Ogre::Vector3 normal = report.normal;
		if (rotate)
		{
			object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(normal.z * 90)));
			object->Rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-normal.x * 90)));
			if (normal.y < 0)
			{
				object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(-normal.y * 180)));
			}
		}
		Ogre::Vector3 offset = Ogre::Vector3(0,0,0);
		Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
		if (visuals != 0)
		{
			if (visuals->GetNode()->getAttachedObject(0))
			{
  				offset = ((visuals->GetNode()->getScale() * visuals->GetNode()->getAttachedObject(0)->getBoundingBox().getSize()) * 0.475f) * normal;
			}
			else offset = (visuals->GetNode()->getScale() * 0.5f) * normal;//object->GetVisual()->getBoundingBox().getSize()
		}
		object->SetGlobalPosition(position + offset);
	}

}


void Edit::SetCameraMoveSpeed(float factor)
{
	Ice::Main::Instance().GetCameraController()->mMoveSpeed = factor;
}

void Edit::SetCameraRotationSpeed(float factor)
{
	mRotSpeed = factor;
}

void Edit::SetObjectMoveSpeed(float factor)
{
	mObjectMovSpeed = factor;
}

void Edit::SetObjectRotationSpeed(float factor)
{
	mObjectRotSpeed = factor;
}



void Edit::ReceiveMessage(Ice::Msg &msg)
{
	if (msg.type == "MOUSE_MOVE")
	{
		OnMouseMove(Ogre::Degree(msg.params.GetInt("ROT_X_REL")), Ogre::Degree(msg.params.GetInt("ROT_Y_REL")));
	}
	if (msg.type == "UPDATE_PER_FRAME")
	{
		if (mMoverReset.mover)
		{
			if (!mMoverReset.mover->GetComponent<Ice::GOCMover>()->IsMoving())
			{
				if (mFreezeCamera)
				{
					mFreezeCamera = false;
					mSelectedObjects.front().mObject->GetComponent<Ice::GOCOgreNode>()->GetNode()->showBoundingBox(true);
				}
				mMoverReset.mover->GetComponent<Ice::GOCMover>()->Reset();
				mMoverReset.mover = nullptr;
			}
		}
		//Die Preview node, wenn vorhanden, rotieren
		if (wxEdit::Instance().GetAuiManager().GetPane("preview").IsShown())
		{
			wxEdit::Instance().GetPreviewWindow()->Update(msg.params.GetFloat("TIME"));
		}
		/*if (wxEdit::Instance().GetpropertyWindow()->GetCurrentPageName() == "EditGameObject")
		{
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->Update();
			if (((wxEditIceGameObject*)wxEdit::Instance().GetpropertyWindow()->GetCurrentPage())->GetGameObject() != NULL)
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}*/
	}
};

void Edit::OnInsertAnimKey( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	STOP_MAINLOOP
	if (mSelectedObjects.size() != 1) return;

	Ice::AnimKey *key = mSelectedObjects.front().mObject->GetComponent<Ice::AnimKey>();
	if (!key) return;

	Ice::GameObject *animKey = key->CreateSuccessor();
	animKey->SetGlobalPosition(GetInsertPosition());

	SelectObject(animKey);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(animKey);
	RESUME_MAINLOOP
}

void Edit::OnMoverInsertKey( wxCommandEvent& WXUNUSED(event)) /*= wxCommandEvent()*/
{

	if (mSelectedObjects.size() != 1) return;
	
	Ice::GOCMover *mover = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();
	if (!mover) return;
		
	/*mMoverReset.mover = mover->GetOwner();
	mMoverReset.resetPos = mover->GetOwner()->GetGlobalPosition();
	mMoverReset.resetQuat = mover->GetOwner()->GetGlobalOrientation();*/

	Ice::GameObject *go = new Ice::GameObject();
	Ice::GOCAnimKey *key = new Ice::GOCAnimKey();
	go->AddComponent(Ice::GOComponentPtr(key));
	go->SetParent(mover->GetOwner());

	go->SetGlobalPosition(mover->GetOwner()->GetGlobalPosition());
	SelectObject(go);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(go);

	//mover->InsertKey(go, nullptr);	is called in OnAddChild handler
}

#define CALL_MOVER_FUNCTION(fct)\
if (mSelectedObjects.size() != 1) return;\
	\
	Ice::GOCMover *mover = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();\
	if (!mover) return;\
	\
	if (mSelectedObjects.front().mObject->GetComponent<Ice::GOCSimpleCameraController>())\
	{\
		mFreezeCamera = true;\
		mSelectedObjects.front().mObject->GetComponent<Ice::GOCOgreNode>()->GetNode()->showBoundingBox(false);\
	}\
	\
	mover->fct();

void Edit::OnTriggerMover( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	Ice::GOCMover *m = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();
	if (m) mMoverReset.mover = m->GetOwner();
	CALL_MOVER_FUNCTION(Trigger)
}

void Edit::OnPauseMover( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	CALL_MOVER_FUNCTION(Pause)
}

void Edit::OnStopMover( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	CALL_MOVER_FUNCTION(Stop)
}

void Edit::OnSetLookAtObject( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	if (mSelectedObjects.size() < 1) return;
	Ice::GOCMover *mover = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();
	IceAssert(mover);
	if (mSelectedObjects.size() == 1) mover->SetLookAtObject(nullptr);
	else if (mSelectedObjects.size() == 2) mover->SetLookAtObject(mSelectedObjects.back().mObject);
}
void Edit::OnSetNormalLookAtObject( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	if (mSelectedObjects.size() < 1) return;
	Ice::GOCMover *mover = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();
	IceAssert(mover);
	if (mSelectedObjects.size() == 1) mover->SetNormalLookAtObject(nullptr);
	else if (mSelectedObjects.size() == 2) mover->SetNormalLookAtObject(mSelectedObjects.back().mObject);
}

Ogre::Vector3 Edit::GetInsertPosition()
{
	return Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5));
}

void Edit::OnRender()
{
	Ice::MainLoop::Instance().doLoop();
}