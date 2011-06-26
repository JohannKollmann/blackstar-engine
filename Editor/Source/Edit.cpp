
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
#include "IceAIManager.h"
#include "IceGOCMover.h"
#include "IceGOCOgreNode.h"
#include "AmbientOcclusionGenerator.h"
#include "IceGOCJoint.h"
#include "VertexMultitextureWeightBrusher.h"

#include "ResourceGroupReloader.h"

IMPLEMENT_CLASS(Edit, wxOgre)

enum
{
	EVT_InsertObject,
	EVT_InsertObjectAsChild,
	EVT_DeleteObject,
	EVT_CreateObjectgroup,
	EVT_SaveObjectgroup,
	EVT_SaveBones,
	EVT_InsertAnimKey,
	EVT_MoverInserKey,
	EVT_TriggerMover,
	EVT_PauseMover,
	EVT_StopMover,
	EVT_SetLookAtObject,
	EVT_SetNormalLookAtObject,
	EVT_ComputeAO,
	EVT_CreateJoint
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
	EVT_MENU(EVT_InsertAnimKey, Edit::OnInsertAnimKey)
	EVT_MENU(EVT_MoverInserKey, Edit::OnMoverInsertKey)
	EVT_MENU(EVT_TriggerMover, Edit::OnTriggerMover)
	EVT_MENU(EVT_PauseMover, Edit::OnPauseMover)
	EVT_MENU(EVT_StopMover, Edit::OnStopMover)
	EVT_MENU(EVT_SetLookAtObject, Edit::OnSetLookAtObject)
	EVT_MENU(EVT_SetNormalLookAtObject, Edit::OnSetNormalLookAtObject)
	EVT_MENU(EVT_ComputeAO, Edit::OnComputeAO)
	EVT_MENU(EVT_CreateJoint, Edit::OnCreateJoint)

	END_EVENT_TABLE()

Edit::Edit(wxWindow* parent) : wxOgre(parent, -1)
{
	mEdit = &wxEdit::Instance();

	mCamRotating = 0;
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
	mMultitextureBrushMode = false;

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

	mEngineLoopBlockers = 0;

	mCurrentMaterialSelection.mSubEntity = NULL;
	mCurrentMaterialSelection.mBillboardSet = NULL;
	mCurrentMaterialSelection.mOriginalMaterialName = "";

	mMoverReset.Mover.reset();
};

void Edit::PostInit()
{
	wxEdit::Instance().GetMainToolbar()->RegisterTool("Physics", "WorldSettings", "Data/Editor/Intern/Editor_gewicht_04.png", Edit::OnToolbarEvent, "Enables/Disables Physics simulation", true, true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("TimeCycle", "WorldSettings", "Data/Editor/Intern/editor_clock.png", Edit::OnToolbarEvent, "Enables/Disables game clock", true, true);
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

	wxEdit::Instance().GetMainToolbar()->RegisterTool("ReloadOgreResources", "Reload", "Data/Editor/Intern/editor_scriptreload_01.png", Edit::OnToolbarEvent, "Reload all Ogre resources");
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Reload", true);

	Ice::AIManager::Instance().SetWayMeshLoadingMode(false);

	SetCameraMoveSpeed(mMovSpeed);
	SetCameraRotationSpeed(mRotSpeed);

	//OIS Maus Input ist im embedded Mode unzuverlässig. Wir übernehmen.
	Ice::Main::Instance().GetCameraController()->mMove = true;
	Ice::Main::Instance().GetCameraController()->mYRot = false;
	Ice::Main::Instance().GetCameraController()->mXRot = false;

	mPivotNode = Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mPivotOffsetNode = mPivotNode->createChildSceneNode();

	JoinNewsgroup(Ice::GlobalMessageIDs::MOUSE_MOVE);
	JoinNewsgroup(Ice::GlobalMessageIDs::UPDATE_PER_FRAME);

	Ice::Main::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode("EditorPreview");

	Ice::SceneManager::Instance().ShowEditorMeshes(true);

	setCamera(Ice::Main::Instance().GetCamera());
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
		Ice::Main::Instance().GetMainLoopThread("Physics")->SetPaused(!checked);
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
	if (toolname == "ReloadOgreResources")
	{
		STOP_MAINLOOP
		ResourceGroupReloader rgh;
		rgh.reloadResourceGroup("General");
		RESUME_MAINLOOP
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

	Ice::GameObjectPtr obj = (*mSelectedObjects.begin()).mObject;
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
	if (!HasFocus()) return;
	if (mShowMouse) ShowCursor(false);
	POINT p;
	GetCursorPos(&p);

	wxPoint winPos = GetScreenPosition();
	wxSize winSize = GetSize();
	mWinMousePosition.x = p.x;
	if (mWinMousePosition.x < winPos.x) mWinMousePosition = winPos.x + 5;
	if (mWinMousePosition.x > winPos.x + winSize.GetX()) mWinMousePosition.x = winPos.x + winSize.GetX() - 5;
	mWinMousePosition.y = p.y;
	if (mWinMousePosition.y < winPos.y) mWinMousePosition.y = winPos.y + 1;
	if (mWinMousePosition.y > winPos.y + winSize.GetY()) mWinMousePosition.y = winPos.y + winSize.GetY() - 1;
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
	if (mRightDown && mCamRotating == 0)
	{
		mCamRotating = 1;
	}
	else if (mRightDown == false)
	{	
		if (mCamRotating == 2)
		{
			mCamRotating = 0;
			FreeAndShowMouse();
			return;
		}
		mCamRotating = 0;
	}
	if (mCamRotating == 2) return;

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
				std::vector<ComponentSectionPtr> sections;

				if (mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() != "None")
				{
					bool skip = false;
					if (mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".ot") == Ogre::String::npos)
					{
						LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
						ls->LoadAtom("vector<ComponentSectionPtr>", (void*)(&sections));
						ls->CloseFile();
						delete ls;

						ITERATE(i, sections)
						{
							if ((*i)->mSectionName.find("Joint") != Ogre::String::npos)
							{
								skip = true;
								break;
							}
						}
					}

					if (!skip)
					{
						menu.Append(EVT_InsertObject, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
						if (mSelectedObjects.size() > 0)
						{
							menu.Append(EVT_InsertObjectAsChild, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() + " as Child of " +  mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject->GetName());
						}
					}
				}

				if (mSelectedObjects.size() > 1) menu.Append(EVT_CreateObjectgroup, "Merge");
				if (mSelectedObjects.size() == 2)		//Dyadic operators
				{
					Ice::GameObjectPtr obj1 = (*mSelectedObjects.begin()).mObject;
					Ice::GameObjectPtr obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;

					if (obj1->GetComponent<Ice::GOCMover>())
					{
						menu.Append(EVT_SetLookAtObject, "Look At");
						menu.Append(EVT_SetNormalLookAtObject, "Look At (Normal)");
					}
					if (obj1->GetComponent<Ice::GOCRigidBody>() && obj2->GetComponent<Ice::GOCRigidBody>())
					{
						if (mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() != "None" && mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".ot") == Ogre::String::npos)
						{
							ITERATE(i, sections)
							{
								if ((*i)->mSectionName.find("Joint") != Ogre::String::npos)
								{
									menu.Append(EVT_CreateJoint, "Create joint");
									break;
								}
							}
						}
					}
				}
				if (mSelectedObjects.size() == 1)
				{
					menu.Append(EVT_SaveObjectgroup, "Save Object Group");

					Ice::GameObjectPtr obj1 = (*mSelectedObjects.begin()).mObject;
					if (obj1->GetComponent<Ice::GOCMover>())
					{
						menu.Append(EVT_SetLookAtObject, "Clear Look At");
						menu.Append(EVT_SetNormalLookAtObject, "Clear Look At (Normal)");
					}

					if (obj1->GetComponent<Ice::GOCMeshRenderable>())
					{
						menu.Append(EVT_ComputeAO, "Compute Ambient Occlusion");
					}

					if (obj1->GetComponent("View", "Skeleton"))
					{
						menu.Append(EVT_SaveBones, "Save Bone Config");
					}
					if (obj1->GetComponent<Ice::GOCAnimKey>())
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
			wxEdit::Instance().GetpropertyWindow()->UpdateCurrentPage();
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
			wxEdit::Instance().GetpropertyWindow()->UpdateCurrentPage();
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
				STOP_MAINLOOP
				mPerformingObjMov = true;
				Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mYAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f, (mZAxisLock == AxisLock::UNLOCKED) ? 1.0f : 0.0f);
				scaleaxis = (*i).mObject->GetGlobalOrientation().Inverse() * scaleaxis;
				scaleaxis.x = Ogre::Math::Abs(scaleaxis.x);
				scaleaxis.y = Ogre::Math::Abs(scaleaxis.y);
				scaleaxis.z = Ogre::Math::Abs(scaleaxis.z);
				(*i).mObject->Rescale(scaleaxis * ev.m_wheelRotation * mObjectScaleSpeed);
				(*i).mObject->Freeze(true);
				AttachAxisObject((*i).mObject);
				RESUME_MAINLOOP
			}
		}
	}

	ev.Skip();
};

void Edit::AttachAxisObject(Ice::GameObjectPtr object)
{
	object->RemoveComponent("AxisObject");
	AxisComponent* caxis = new AxisComponent();
	object->AddComponent(Ice::GOComponentPtr(caxis));
}
void Edit::DetachAxisObject(Ice::GameObjectPtr object)
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

	if (mLeftDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == AxisLock::UNLOCKED || mXAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					std::set<Ice::GameObject*> blacklist;
					(*i).mObject->Translate(Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(RotX.valueRadians() * mObjectMovSpeed,0,0), mXAxisLock == AxisLock::UNLOCKED, mXAxisLock == AxisLock::UNLOCKED, &blacklist);
				}
			}
			if (mYAxisLock == AxisLock::UNLOCKED || mYAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					std::set<Ice::GameObject*> blacklist;
					(*i).mObject->Translate(Ogre::Vector3(0,-RotY.valueRadians() * mObjectMovSpeed,0), mYAxisLock == AxisLock::UNLOCKED, mYAxisLock == AxisLock::UNLOCKED, &blacklist);
				}
			}
			if (mZAxisLock == AxisLock::UNLOCKED || mZAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					Ogre::Vector3 movaxis = Ice::Main::Instance().GetCamera()->getDerivedDirection() * -1.0f;
					movaxis.y = 0;
					movaxis.normalise();
					std::set<Ice::GameObject*> blacklist;
					(*i).mObject->Translate(movaxis * RotY.valueRadians() * mObjectMovSpeed , mZAxisLock == AxisLock::UNLOCKED, mZAxisLock == AxisLock::UNLOCKED, &blacklist);
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
		else
		{
			BrushMultitexture(0);
		}
	}
	else if (mRightDown == true && mSelectedObjects.size() > 0)
	{
		if (mXAxisLock == AxisLock::UNLOCKED || mXAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
		{
			mPerformingObjRot = true;
			for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				std::set<Ice::GameObject*> blacklist;
				(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * Ice::Main::Instance().GetCamera()->getDerivedOrientation().xAxis(), RotY * mObjectRotSpeed, mXAxisLock == AxisLock::UNLOCKED, mXAxisLock == AxisLock::UNLOCKED, &blacklist);
			}
		}
		if (mYAxisLock == AxisLock::UNLOCKED || mYAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
		{
			mPerformingObjRot = true;
			for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				std::set<Ice::GameObject*> blacklist;
				(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * Ogre::Vector3::UNIT_Y, RotX * mObjectRotSpeed, mYAxisLock == AxisLock::UNLOCKED, mYAxisLock == AxisLock::UNLOCKED, &blacklist);
			}
		}
		if (mZAxisLock == AxisLock::UNLOCKED || mZAxisLock == AxisLock::UNLOCKED_SKIPCHILDREN)
		{
			mPerformingObjRot = true;
			for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				Ogre::Vector3 rotaxis = Ice::Main::Instance().GetCamera()->getDerivedDirection() * -1.0f;
				rotaxis.y = 0;
				rotaxis.normalise();
				std::set<Ice::GameObject*> blacklist;
				(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * rotaxis, RotX * mObjectRotSpeed, mZAxisLock == AxisLock::UNLOCKED, mZAxisLock == AxisLock::UNLOCKED, &blacklist);
			}
		}
	}
	if (!mPerformingObjRot && mCamRotating > 0 && !mAltIsDown)
	{
		mCamRotating = 2;
		LockAndHideMouse();
		Ice::Main::Instance().GetCamera()->yaw(-RotX * mRotSpeed);
		Ice::Main::Instance().GetCamera()->pitch(-RotY * mRotSpeed);
	}
	else if (!mPerformingObjRot && mCamRotating > 0 && mAltIsDown)
	{
		mCamRotating = 2;
		LockAndHideMouse();
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(0,1,0), -RotX * mRotSpeed);
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(1,0,0), -RotY * mRotSpeed);
		mPivotOffsetNode->needUpdate(true);
		Ice::Main::Instance().GetCamera()->setPosition(mPivotOffsetNode->_getDerivedPosition());
		Ice::Main::Instance().GetCamera()->lookAt(mPivotNode->getPosition());
	}

	//move the preview object
	for (auto i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
	{
		(*i)->SetGlobalOrientation(Ogre::Quaternion());
		AlignObjectWithMesh(*i);
	}
};

void Edit::BrushMultitexture(unsigned int textureLayer)
{
	Ogre::Ray ray = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(mMousePosition.x, mMousePosition.y);
	if (Ice::SceneManager::Instance().HasLevelMesh())
	{
		Ogre::Entity *levelMeshEnt = Ice::Main::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity");
		if (levelMeshEnt)
		{
			VertexMultitextureWeightBrusher::SetMultitextureWeight(levelMeshEnt, ray, 2.0f, textureLayer);
		}
	}
	
}

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
	STOP_MAINLOOP
	for (auto i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
	{
		Ice::SceneManager::Instance().RemoveGameObject((*i)->GetID());
	}
	mPreviewObjects.clear();
	RESUME_MAINLOOP
}
void Edit::CreatePreviewObject()
{
	STOP_MAINLOOP
	Ice::GameObjectPtr preview = InsertObject(0, true, true);
	if (preview)
	{
		if (preview->GetComponent("Physics")) preview->RemoveComponent("Physics");
		mPreviewObjects.push_back(preview);
	}
	RESUME_MAINLOOP
}

void Edit::OnLoadWorld(Ogre::String fileName)
{
	STOP_MAINLOOP
	DeselectAllObjects();
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");
	mMoverReset.Mover.reset();
	Ice::SceneManager::Instance().LoadLevel(fileName);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetOgrePane()->SetFocus();
	RESUME_MAINLOOP
};

void Edit::OnSaveWorld(Ogre::String fileName)
{
	Ice::SceneManager::Instance().SaveLevel(fileName);
};

Ice::GameObjectPtr Edit::InsertWaypoint(bool align, bool create_only)
{
	Ice::GameObjectPtr waypoint = Ice::SceneManager::Instance().CreateGameObject();
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

Ice::GameObjectPtr Edit::CreateGameObject(std::vector<ComponentSectionPtr> componentParameters)
{
	Ice::GameObjectPtr object = Ice::SceneManager::Instance().CreateGameObject();

	Ogre::Vector3 offset;
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	Ogre::Quaternion orientation;
	for (auto i = componentParameters.begin(); i != componentParameters.end(); i++)
	{
		if ((*i)->mSectionName == "GameObject")
		{
			offset = (*i)->mSectionData.GetOgreVec3("Position");
			orientation = (*i)->mSectionData.GetOgreQuat("Orientation");
			scale = (*i)->mSectionData.GetOgreVec3("Scale");
			object->SetGlobalOrientation(orientation);
			object->SetGlobalScale(scale);
			continue;
		} 
		(*i)->mSectionData.AddOgreVec3("Scale", scale);

		Ice::GOCEditorInterface *component = Ice::SceneManager::Instance().NewGOC((*i)->mSectionName);
		component->SetParameters(&(*i)->mSectionData);
		object->AddComponent(Ice::GOComponentPtr(component->GetGOComponent()));
	}
	return object;
}

Ice::GameObjectPtr Edit::InsertObject(Ice::GameObjectPtr parent, bool align, bool create_only)
{
	STOP_MAINLOOP
	Ice::GameObjectPtr object;
	Ogre::String sResource = wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().c_str();
	if (sResource == "None") return object;
	if (sResource.find("Waypoint.static") != Ogre::String::npos)
	{
		return InsertWaypoint(align, create_only);
	}
	else if (sResource.find(".") != Ogre::String::npos)
	{
		LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(sResource);
		if (sResource.find(".ot") != Ogre::String::npos)
		{
			object = ls->LoadTypedObject<Ice::GameObject>();

			if (parent.get()) object->SetParent(parent);
		}
		else
		{
			std::vector<ComponentSectionPtr> sections;
			ls->LoadAtom("vector<ComponentSectionPtr>", (void*)(&sections));

			object = CreateGameObject(sections);
			object->SetParent(parent);

		}

		ls->CloseFile();
		delete ls;
	}

	std::set<Ice::GameObject*> blacklist;
	if (align) AlignObjectWithMesh(object);
	else object->SetGlobalPosition(GetInsertPosition(), true, true, true, &blacklist); 

	if (!create_only)
	{
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(object);
		SelectObject(object);

		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

		wxEdit::Instance().GetOgrePane()->SetFocus();
	}

	object->FirePostInit();

	object->ShowEditorVisuals(true);

	RESUME_MAINLOOP

	return object;
}

void Edit::OnInsertObject( wxCommandEvent& WXUNUSED(event) )
{
	InsertObject();
}

void Edit::OnKillFocus( wxFocusEvent& event )
{
	mCamRotating = 0;
	mPerformingObjMov = false;
	mPerformingObjRot = false;
	if (mPlaying)
		PauseGame();
	if (!mShowMouse) ShowCursor(true);
	mMouseLocked = false;
	mShowMouse = true;
	Ice::Main::Instance().GetInputManager()->SetEnabled(false);
}

void Edit::OnSetFocus( wxFocusEvent& event )
{
	Ice::Main::Instance().GetInputManager()->SetEnabled(true);
	if (!mShowMouse) ShowCursor(true);
	mMouseLocked = false;
	mShowMouse = true;
}

void Edit::OnInsertObjectAsChild( wxCommandEvent& WXUNUSED(event) )
{
	if (mSelectedObjects.size() > 0) InsertObject(mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject);
}

void Edit::OnDeleteObject( wxCommandEvent& WXUNUSED(event) )
{
	STOP_MAINLOOP

	wxEditIceGameObject *propgrid_page = dynamic_cast<wxEditIceGameObject*>(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage());
	if (propgrid_page)
	{
		propgrid_page->SetObject(Ice::GameObjectPtr());
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");

	bool skip = false;
	bool one = false;
	if (mSelectedObjects.size() == 1)
	{
		one = true;
		Ice::GameObjectPtr object = (*mSelectedObjects.begin()).mObject;

		//check if the object owns other objects.
		bool hasChildren = false;
		object->ResetObjectReferenceIterator();
		while (object->HasNextObjectReference())
		{
			Ice::ObjectReferencePtr objRef = object->GetNextObjectReference();
			if (objRef->UserID == Ice::GameObject::ReferenceTypes::PARENT) hasChildren = true;
		}
		if (hasChildren)
		{
			skip = true;
			wxMessageDialog dialog( NULL, "Delete Children?",
				"Deleting root object...", wxYES_DEFAULT|wxYES_NO|wxCANCEL|wxICON_QUESTION);
			int input = dialog.ShowModal();
			if (input == wxID_YES)
			{
				if (object.get() == mMoverReset.Mover.lock().get()) mMoverReset.Mover.reset();
				Ice::SceneManager::Instance().RemoveGameObject(object->GetID());
			}
			else if (input == wxID_NO)
			{
				object->ResetObjectReferenceIterator();
				while (object->HasNextObjectReference())
				{
					Ice::ObjectReferencePtr objRef = object->GetNextObjectReference();
					if (objRef->UserID == Ice::GameObject::ReferenceTypes::PARENT) objRef->Object.lock()->SetParent(object->GetParent());
				}
				if (object.get() == mMoverReset.Mover.lock().get()) mMoverReset.Mover.reset();
				Ice::SceneManager::Instance().RemoveGameObject(object->GetID());
			}
			else if (input == wxID_CANCEL)
			{
				RESUME_MAINLOOP
				return;
			}
		}
	}
	if (!skip)
	{
		for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
		{
			Ice::SceneManager::Instance().RemoveGameObject((*i).mObject->GetID());
		}
	}
	mSelectedObjects.clear();
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	RESUME_MAINLOOP
}

void Edit::OnCreateObjectGroup( wxCommandEvent& WXUNUSED(event) )
{
	Ice::GameObjectPtr parent = Ice::SceneManager::Instance().CreateGameObject();
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
			LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(dialog.GetPath().c_str().AsChar(), Ogre::String(dialog.GetPath().c_str()) + ".xml");
			Ice::GameObjectPtr go = (*mSelectedObjects.begin()).mObject;
			DeselectAllObjects();
			Ogre::Vector3 position = go->GetGlobalPosition();
			std::set<Ice::GameObject*> blacklist;
			go->SetGlobalPosition(Ogre::Vector3(0,0,0), true, true, true, &blacklist);
			blacklist.clear();
			ss->SaveObject(go.get(), "Root");
			ss->CloseFiles();
			go->SetGlobalPosition(position, true, true, true, &blacklist);
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
	STOP_MAINLOOP
	DeselectMaterial();

	//Potenziell unsicher!
	wxEdit::Instance().GetWorldExplorer()->SetSelection(3);

	Ogre::Ray ray = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY);
	Ice::ObjectLevelRayCaster rc(ray);
	Ice::GameObject* object = rc.GetFirstHit(true);
	if (object)
	{
		Ice::GOCMeshRenderable *gocmesh = object->GetComponent<Ice::GOCMeshRenderable>();
		if (gocmesh != 0)
		{
			EntityMaterialInspector emi(gocmesh->GetEntity(), ray);
			Ogre::SubEntity *ent = emi.GetSubEntity();
			if (!ent)
			{
				RESUME_MAINLOOP
				return;
			}
			mCurrentMaterialSelection.mSubEntity = ent;
			mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
			ent->setMaterialName("Editor_Submesh_Selected");
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(((Ogre::Entity*)gocmesh->GetEntity())->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
			wxEdit::Instance().GetOgrePane()->SetFocus();
			RESUME_MAINLOOP
			return;
		}
	}

	if (Ice::SceneManager::Instance().HasLevelMesh())
	{
		EntityMaterialInspector emi(Ice::Main::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity"), ray);
		Ogre::SubEntity *ent = emi.GetSubEntity();
		if (ent)
		{
			//Ice::Log::Instance().LogMessage("change material: " + ent->getMaterialName());
			mCurrentMaterialSelection.mSubEntity = ent;
			mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
			ent->setMaterialName("Editor_Submesh_Selected");
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(Ice::Main::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity")->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
			wxEdit::Instance().GetOgrePane()->SetFocus();
		}
	}
	RESUME_MAINLOOP
}

void Edit::DeselectMaterial()
{
	STOP_MAINLOOP
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
	RESUME_MAINLOOP
}

void Edit::OnSelectObject(float MouseX, float MouseY)
{
	//Ice::Log::Instance().LogMessage("OnSelectObject");
	Ice::ObjectLevelRayCaster rc(Ice::Main::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY));
	Ice::GameObject *object = rc.GetFirstHit();
	//the user is capable to iterate through the ray direction array
	bool found = false;
	while (object)
	{
		for (auto i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
		{
			if (i->mObject.get() == object)
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
		SelectObject(Ice::SceneManager::Instance().GetObjectByInternID(object->GetID()));
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
}

void Edit::OnBrush()
{
	InsertObject(0, true);
}

void Edit::SelectObject(Ice::GameObjectPtr object)
{
	if (!object.get()) return;

	//Ice::Log::Instance().LogMessage("Select Object " + object->GetName());
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
	//Ice::Log::Instance().LogMessage("SelectObject " + object->GetName());
}

void Edit::SelectChildren(Ice::GameObjectPtr object)
{
	//Ice::Log::Instance().LogMessage("select Children " + object->GetName());
	object->ResetObjectReferenceIterator();
	while (object->HasNextObjectReference())
	{
		Ice::ObjectReferencePtr objRef = object->GetNextObjectReference();
		Ice::GameObjectPtr child = objRef->Object.lock();
		if (child.get() && objRef->Flags & Ice::ObjectReference::OWNER)
		{
			child->Freeze(true);
			Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
			if (visuals != 0)
			{
				visuals->GetNode()->showBoundingBox(true);
			}
			SelectChildren(child);
		}
	}
}

void Edit::DeselectChildren(Ice::GameObjectPtr object)
{
	//Ice::Log::Instance().LogMessage("Deselect Children " + object->GetName());
	object->ResetObjectReferenceIterator();
	while (object->HasNextObjectReference())
	{
		Ice::ObjectReferencePtr objRef = object->GetNextObjectReference();
		Ice::GameObjectPtr child = objRef->Object.lock();
		if (child.get() && objRef->Flags & Ice::ObjectReference::OWNER)
		{
			child->Freeze(false);
			Ice::GOCOgreNode *visuals = object->GetComponent<Ice::GOCOgreNode>();
			if (visuals != nullptr)
			{
				visuals->GetNode()->showBoundingBox(false);
			}
			DeselectChildren(child);
		}
	}
}

bool Edit::ObjectIsSelected(Ice::GameObjectPtr object)
{
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		if ((*i).mObject.get() == object.get())
		{
			return true;
		}
	}
	return false;
}

void Edit::DeselectObject(Ice::GameObjectPtr object)
{
	if (object->GetComponent("CharacterInput", "PlayerInput") && object->GetComponent("Camera"))
	{
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", false);
	}
	//Ice::Log::Instance().LogMessage("Deselect Object " + object->GetName());
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
	STOP_MAINLOOP		
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", false);
	//Ice::Log::Instance().LogMessage("Deselect all Objects");
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
	RESUME_MAINLOOP
}

void Edit::AlignObjectWithMesh(const Ice::GameObjectPtr &object, bool rotate)
{
	Ogre::Ray mouseRay = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(mMousePosition.x, mMousePosition.y);
	OgrePhysX::Scene::QueryHit report;
	std::vector<OgrePhysX::Scene::QueryHit> lReport;
	bool found = false;
	float shortest = 999999;
	Ice::Main::Instance().GetPhysXScene()->raycastAllShapes(lReport, mouseRay);
	for (std::vector<OgrePhysX::Scene::QueryHit>::iterator i = lReport.begin(); i != lReport.end(); i++)
	{
		if ((*i).hitActor->userData != object.get() && i->distance < shortest)
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
		std::set<Ice::GameObject*> blacklist;
		object->SetGlobalPosition(position + offset, true, true, true, &blacklist);
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
	if (msg.typeID == Ice::GlobalMessageIDs::MOUSE_MOVE)
	{
		OnMouseMove(Ogre::Degree(msg.params.GetInt("ROT_X_REL")), Ogre::Degree(msg.params.GetInt("ROT_Y_REL")));
	}
	if (msg.typeID == Ice::GlobalMessageIDs::UPDATE_PER_FRAME)
	{
		Ice::GameObjectPtr mover = mMoverReset.Mover.lock();
		if (mover.get())
		{
			if (!mover->GetComponent<Ice::GOCMover>()->IsMoving())
			{
				if (mFreezeCamera)
				{
					mFreezeCamera = false;
					mSelectedObjects.front().mObject->GetComponent<Ice::GOCOgreNode>()->GetNode()->showBoundingBox(true);
				}
				mover->GetComponent<Ice::GOCMover>()->Reset();
				mMoverReset.Mover.reset();
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

	Ice::GOCAnimKey *key = mSelectedObjects.front().mObject->GetComponent<Ice::GOCAnimKey>();
	if (!key) return;

	Ice::GameObjectPtr animKey = key->CreateSuccessor();
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

	Ice::GameObjectPtr keyObj = mover->CreateKey(0);

	keyObj->SetGlobalPosition(mover->GetOwner()->GetGlobalPosition());
	SelectObject(keyObj);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(keyObj);
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
	if (m) mMoverReset.Mover = m->GetOwner();
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
	if (mSelectedObjects.size() == 1) mover->SetLookAtObject(Ice::GameObjectPtr());
	else if (mSelectedObjects.size() == 2) mover->SetLookAtObject(mSelectedObjects.back().mObject);
}
void Edit::OnSetNormalLookAtObject( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	if (mSelectedObjects.size() < 1) return;
	Ice::GOCMover *mover = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMover>();
	IceAssert(mover);
	if (mSelectedObjects.size() == 1) mover->SetNormalLookAtObject(Ice::GameObjectPtr());
	else if (mSelectedObjects.size() == 2) mover->SetNormalLookAtObject(mSelectedObjects.back().mObject);
}
void Edit::OnComputeAO( wxCommandEvent& WXUNUSED(event) /*= wxCommandEvent()*/ )
{
	if (mSelectedObjects.size() < 1) return;
	Ice::GOCMeshRenderable *mesh = mSelectedObjects.front().mObject->GetComponent<Ice::GOCMeshRenderable>();
	IceAssert(mesh);
    wxFileDialog dialog
                 (
                    this,
                    "Save Mesh",
                    wxEmptyString,
                    wxEmptyString,
                    "Ogre Mesh files (*.mesh)|*.mesh",
					wxFD_SAVE|wxFD_OVERWRITE_PROMPT
                 );

    dialog.CentreOnParent();
	dialog.SetPath("Data/Media/Meshes/");

	dialog.SetFilterIndex(1);

    if (dialog.ShowModal() == wxID_OK)
    {
		STOP_MAINLOOP
		AmbientOcclusionGenerator::Instance().bakeAmbientOcclusion(mesh->GetEntity(), dialog.GetPath().c_str().AsChar());
		RESUME_MAINLOOP
	}
}

void Edit::OnCreateJoint( wxCommandEvent& WXUNUSED(event))
{
	IceAssert(mSelectedObjects.size() == 2)
		Ice::GameObjectPtr obj1 = (*mSelectedObjects.begin()).mObject;
	Ice::GameObjectPtr obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;
	IceAssert(obj1->GetComponent<Ice::GOCRigidBody>() && obj2->GetComponent<Ice::GOCRigidBody>())
	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
	std::vector<ComponentSectionPtr> sections;
	ls->LoadAtom("vector<ComponentSectionPtr>", (void*)(&sections));
	ls->CloseFile();
	delete ls;
	Ice::GameObjectPtr object = CreateGameObject(sections);
	object->GetComponent<Ice::GOCJoint>()->SetActorObjects(obj1, obj2);
	object->SetGlobalPosition(obj1->GetGlobalPosition() + ((obj2->GetGlobalPosition() - obj1->GetGlobalPosition()) * 0.5f));
	object->FirePostInit();
	object->ShowEditorVisuals(true);

	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(object);
	SelectObject(object);
	wxEdit::Instance().GetOgrePane()->SetFocus();
}

Ogre::Vector3 Edit::GetInsertPosition()
{
	return Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5));
}

void Edit::OnRender()
{
	if (mCamRotating == 2 || mPerformingObjMov || mPerformingObjRot || mPlaying)
	{
		if (mShowMouse) LockAndHideMouse();
	}
	else if (!mShowMouse) FreeAndShowMouse();

	mBlockEngineLoopCond.lock();
	if (mEngineLoopBlockers == 0) 
	{
		Ice::Main::Instance().GetMainLoopThread("Synchronized")->Step();
		Ice::Main::Instance().GetMainLoopThread("View")->Step();
	}
	mBlockEngineLoopCond.unlock();
}

void Edit::OnSize(wxSizeEvent& event)
{
	if (mInitialized && !mPaused)
	{
		STOP_MAINLOOP
		wxOgre::OnSize(event);
		if (Ice::SceneManager::Instance().GetWeatherController()) Ice::SceneManager::Instance().GetWeatherController()->UpdateViewport();
		RESUME_MAINLOOP
	}
}

void Edit::IncBlockEngineLoop()
{
	mBlockEngineLoopCond.lock();
	if (mEngineLoopBlockers == 0 && Ice::Main::Instance().GetMainLoopThread("Physics") != nullptr)
	{
		Ice::Main::Instance().GetMainLoopThread("Independant")->SetPaused(true);
		Ice::Main::Instance().GetMainLoopThread("Physics")->SetPaused(true);
	}
	mEngineLoopBlockers++;
	mBlockEngineLoopCond.unlock();
}
void Edit::DecBlockEngineLoop()
{
	mBlockEngineLoopCond.lock();
	if (mEngineLoopBlockers > 0)
	{
		mEngineLoopBlockers--;
		if (mEngineLoopBlockers == 0 && Ice::Main::Instance().GetMainLoopThread("Physics") != nullptr)
		{
			Ice::Main::Instance().GetMainLoopThread("Independant")->SetPaused(false);
			Ice::Main::Instance().GetMainLoopThread("Physics")->SetPaused(!wxEdit::Instance().GetMainToolbar()->GetToolIsChecked("Physics"));
		}
	}
	mBlockEngineLoopCond.unlock();
}