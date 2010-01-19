
#include "Edit.h"
#include "IceInput.h"
#include "IceCameraController.h"
#include "IceScenemanager.h"
#include "IceObjectLevelRayCaster.h"
#include "IceWeatherController.h"
#include "IceMainLoop.h"
#include "IceGOCEditorInterface.h"
#include "IceGOCIntern.h"
#include "IceGOCAnimatedCharacter.h"
#include "OgrePhysX.h"
#include "NxScene.h"


Edit::Edit()
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
	SetCameraMoveSpeed(mMovSpeed);
	SetCameraRotationSpeed(mRotSpeed);

	mCurrentMaterialSelection.mSubEntity = NULL;
	mCurrentMaterialSelection.mBillboardSet = NULL;
	mCurrentMaterialSelection.mOriginalMaterialName = "";

	//OIS Maus Input ist im embedded Mode unzuverlässig. Wir übernehmen.
	Ice::Main::Instance().GetCameraController()->mMove = true;
	Ice::Main::Instance().GetCameraController()->mYRot = false;
	Ice::Main::Instance().GetCameraController()->mXRot = false;

	mPivotNode = Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mPivotOffsetNode = mPivotNode->createChildSceneNode();

	Ice::MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	//wxEdit::Instance().GetMainToolbar()->RegisterTool("SaveWorld", "General", "Data/Editor/Intern/Engine_Icon08.png", Edit::OnToolbarEvent);
	//wxEdit::Instance().GetMainToolbar()->SetGroupStatus("General", true);

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
};

void Edit::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	Edit *edit = wxEdit::Instance().GetOgrePane()->GetEdit();
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
	if (mSelectedObjects.size () != 1) return;

	Ice::GameObject* obj = (*mSelectedObjects.begin()).mObject;
	Ice::GOCCameraController *cam = (Ice::GOCCameraController*)obj->GetComponent("Camera", "CameraController");
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
					menu.Append(wxOgre_insertObject, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
					if (mSelectedObjects.size() > 0)
					{
						menu.Append(wxOgre_insertObjectAsChild, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() + " as Child of " +  mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject->GetName());
					}
				}

				if (mSelectedObjects.size() > 1) menu.Append(wxOgre_createObjectgroup, "Merge");
				if (mSelectedObjects.size() == 2)
				{
					Ice::GameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					Ice::GameObject* obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;
					if (obj1->GetComponent("Waypoint") && obj2->GetComponent("Waypoint")) menu.Append(wxOgre_connectWaypoints, "Connect Waypoints");
					/*if ((obj1->GetType() == "Body" || obj1->GetType() == "StaticBody") && (obj2->GetType() == "Body" || obj2->GetType() == "StaticBody")
						&& mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find("Joint") != Ogre::String::npos)
					{
						menu.Append(wxOgre_insertObjectAsChild, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() + " as Child of " +  obj2->GetName());
						mTempJointBodyB = (Ice::AbstractBody*)obj1;
					}
					if (obj1->GetType() == "Body" && obj2->GetType() == "Body")
					{
						for (unsigned short i = 0; i < obj1->GetNumChildren(); i++)
						{
							bool chainoptionappended = false;
							Ice::GameObject *child = obj1->GetChild(i);
							if (child->GetType().find("Joint") != Ogre::String::npos)
							{
								for (unsigned short x = 0; x < obj2->GetNumChildren(); x++)
								{
									if (obj2->GetChild(x) == child)
									{
										menu.Append(wxOgre_createChain, "Create Chain...");
										chainoptionappended = true;
										break;
									}
								}
							}
							if (chainoptionappended) break;
						}
					}*/
				}
				if (mSelectedObjects.size() == 1)
				{
					menu.Append(wxOgre_saveObjectgroup, "Save Object Group");

					Ice::GameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					if (obj1->GetComponent("View", "Skeleton"))
					{
						menu.Append(wxOgre_saveBones, "Save Bone Config");
					}
				}

				if (mSelectedObjects.size() >= 1) menu.Append(wxOgre_deleteObject, "Delete");
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
		((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}

	ev.Skip();
};

void Edit::AttachAxisObject(Ice::GameObject *object)
{
	object->RemoveComponent("AxisObject");
	AxisComponent* caxis = new AxisComponent();
	object->AddComponent(caxis);
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
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
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
			((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
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
		if (!mBrushMode)
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
	Ice::GameObject *preview = OnInsertObject(0, true, true);
	if (preview)
	{
		std::vector<Ogre::String> components = preview->GetComponentsStr();
		for (std::vector<Ogre::String>::iterator i = components.begin(); i != components.end(); i++)
		{
			if ((*i) != "View" && (*i) != "MeshDebugRenderable" && (*i) != "Skeleton")
			{
				preview->RemoveComponent((*i));
			}
		}
		mPreviewObjects.push_back(preview);
	}
}

void Edit::OnLoadWorld(Ogre::String fileName)
{
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");
	Ice::SceneManager::Instance().LoadLevel(fileName);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetOgrePane()->SetFocus();
};

void Edit::OnSaveWorld(Ogre::String fileName)
{
	Ice::SceneManager::Instance().SaveLevel(fileName);
};

Ice::GameObject* Edit::OnInsertWaypoint(bool align, bool create_only)
{
	Ice::GameObject *waypoint = new Ice::GameObject();
	waypoint->AddComponent(new Ice::GOCWaypoint());
	waypoint->AddComponent(new Ice::MeshDebugRenderable("Editor_Waypoint.mesh"));
	waypoint->ShowEditorVisuals(true);
	waypoint->SetGlobalOrientation(Ogre::Quaternion(Ice::Main::Instance().GetCamera()->getDerivedOrientation().getYaw(), Ogre::Vector3(0,1,0)));
	if (align) AlignObjectWithMesh(waypoint);
	else waypoint->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	if (mSelectedObjects.size() == 1)
	{
		Ice::GOCWaypoint *wp2 = (Ice::GOCWaypoint*)(*mSelectedObjects.begin()).mObject->GetComponent("Waypoint");
		if (wp2) ((Ice::GOCWaypoint*)waypoint->GetComponent("Waypoint"))->ConnectWaypoint(wp2);
	}
	if (!create_only)
	{
		SelectObject(waypoint);
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(waypoint);
	}
	wxEdit::Instance().GetOgrePane()->SetFocus();
	return waypoint;
}

Ice::GameObject* Edit::OnInsertObject(Ice::GameObject *parent, bool align, bool create_only)
{
	if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find("Waypoint.static") != Ogre::String::npos)
	{
		return OnInsertWaypoint(align, create_only);
	}
	else if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".") != Ogre::String::npos)
	{
		//Ogre::LogManager::getSingleton().logMessage("OnInsertObject");
		LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
		Ice::GameObject *object;
		if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".ot") != Ogre::String::npos)
		{
			object = (Ice::GameObject*)ls->LoadObject();

			if (parent != NULL) object->SetParent(parent);
		}
		else
		{
			object = new Ice::GameObject(parent);
			if (!align) object->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

			std::list<Ice::ComponentSection> sections;
			Ogre::Vector3 offset;
			Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
			Ogre::Quaternion orientation;
			ls->LoadAtom("std::list<ComponentSection>", (void*)(&sections));
			for (std::list<Ice::ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
			{
				if ((*i).mSectionName == "GameObject")
				{
					offset = (*i).mSectionData->GetOgreVec3("Position");
					orientation = (*i).mSectionData->GetOgreQuat("Orientation");
					scale = (*i).mSectionData->GetOgreVec3("Scale");
					object->SetGlobalOrientation(orientation);
					object->SetGlobalScale(scale);
					continue;
				} 
				(*i).mSectionData->AddOgreVec3("Scale", scale);

				Ice::GOCEditorInterface *component = Ice::SceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
				if (component) component->AttachToGO(object);
			}

		}

		if (align) AlignObjectWithMesh(object);
		else object->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

		ls->CloseFile();
		delete ls;

		if (!create_only)
		{
			wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(object);
			SelectObject(object);

			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

			wxEdit::Instance().GetOgrePane()->SetFocus();
		}

		wxEdit::Instance().GetOgrePane()->SetFocus();
		return object;
	}
	return 0;
}

void Edit::OnSetFocus(bool focus)
{
	if (!mPlaying)
	{
		Ice::Main::Instance().GetInputManager()->SetEnabled(focus);
	}
	else if (!focus)
	{
		PauseGame();
	}
}

void Edit::OnInsertObjectAsChild()
{
	if (mSelectedObjects.size() > 0) OnInsertObject(mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject);
}

void Edit::OnDeleteObject()
{
	bool skip = false;
	if (mSelectedObjects.size() == 1)
	{
		Ice::GameObject *object = (*mSelectedObjects.begin()).mObject;
		if (object->GetNumChildren() > 0)
		{
			skip = true;
			wxMessageDialog dialog( NULL, _T("Delete Children?"),
				_T("Deleting root object..."), wxYES_DEFAULT|wxYES_NO|wxCANCEL|wxICON_QUESTION);
			int input = dialog.ShowModal();
			if (input == wxID_YES)
			{
				delete object;
			}
			else if (input == wxID_NO)
			{
				while (object->GetNumChildren() > 0)
				{
					object->GetChild(0)->SetParent(object->GetParent());
				}
				delete object;
			}
			else if (input == wxID_CANCEL) return;
		}
	}
	if (!skip)
	{
		for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
		{
			delete (*i).mObject;
			(*i).mObject = 0;
		}
	}
	mSelectedObjects.clear();
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
}

void Edit::OnCreateObjectGroup()
{
	Ice::GameObject *parent = new Ice::GameObject();
	parent->SetGlobalPosition(Ice::Main::Instance().GetCamera()->getDerivedPosition() + (Ice::Main::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	for (std::list<EditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		(*i).mObject->SetParent(parent);
	}
	DeselectAllObjects();
	SelectObject(parent);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
}

void Edit::OnSaveObjectGroup()
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

		if (dialog.ShowModal() == wxID_OK)
		{
			LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(dialog.GetPath().c_str(), Ogre::String(dialog.GetPath().c_str()) + ".xml");
			Ogre::Vector3 position = (*mSelectedObjects.begin()).mObject->GetGlobalPosition();
			(*mSelectedObjects.begin()).mObject->SetGlobalPosition(Ogre::Vector3(0,0,0));
			Ice::MainLoop::Instance().doLoop(); //Kleiner hack, damit alle children die temporären Positionen übernehmen
			ss->SaveObject((*mSelectedObjects.begin()).mObject, "Root");
			ss->CloseFiles();
			(*mSelectedObjects.begin()).mObject->SetGlobalPosition(position);
			Ice::MainLoop::Instance().doLoop();
			delete ss;
		}
	}
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects");
}

void Edit::OnConnectWaypoints()
{
	Ice::GOCWaypoint* wp1 = (Ice::GOCWaypoint*)(*mSelectedObjects.begin()).mObject->GetComponent("Waypoint");
	Ice::GOCWaypoint* wp2 = (Ice::GOCWaypoint*)mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject->GetComponent("Waypoint");
	if (!wp1->HasConnectedWaypoint(wp2)) wp1->ConnectWaypoint(wp2);
	else wp1->DisconnectWaypoint(wp2);
}

void Edit::OnSaveBones()
{
	Ice::GOCAnimatedCharacter *ragdoll = (Ice::GOCAnimatedCharacter*)(*mSelectedObjects.begin()).mObject->GetComponent("View", "Skeleton");
	ragdoll->SerialiseBoneObjects("Data\\Scripts\\Animation\\" + ragdoll->GetRagdoll()->getEntity()->getMesh()->getName() + ".bones");
}

void Edit::OnCreateChain()
{
	wxTextEntryDialog dialog(wxEdit::Instance().GetOgrePane(),
		_T("Enter chain length:"),
		_T("Chain length"),
		_T(""),
		wxOK | wxCANCEL);

	Ogre::String sChainLength = "";
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
		Ice::GOCViewContainer *visuals = (Ice::GOCViewContainer*)object->GetComponent("View", "ViewContainer");
		if (visuals != 0)
		{
			Ice::MeshRenderable *gocmesh = (Ice::MeshRenderable*)visuals->GetItem("MeshRenderable");
			if (gocmesh != 0)
			{
				EntityMaterialInspector emi((Ogre::Entity*)gocmesh->GetEditorVisual());
				Ogre::SubEntity *ent = emi.GetSubEntity(ray);
				if (ent == 0) return;
				mCurrentMaterialSelection.mSubEntity = ent;
				mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
				ent->setMaterialName("Editor_Submesh_Selected");
				wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(((Ogre::Entity*)gocmesh->GetEditorVisual())->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
				wxEdit::Instance().GetOgrePane()->SetFocus();
				return;
			}
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
	if (!mMultiSelect) DeselectAllObjects();
	Ice::ObjectLevelRayCaster rc(Ice::Main::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY));
	Ice::GameObject *object = rc.GetFirstHit();
	if (object != NULL)
	{
		SelectObject(object);
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
}

void Edit::OnBrush()
{
	OnInsertObject(0, true);
}

void Edit::SelectObject(Ice::GameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Select Object " + object->GetName());
	if (!mMultiSelect) DeselectAllObjects();
	else DeselectObject(object);

	wxEdit::Instance().GetWorldExplorer()->SetSelection(4);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->ExpandToObject(object);
	wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	((wxEditIceGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(object);

	if (object->GetComponent("CharacterInput", "PlayerInput") && object->GetComponent("Camera"))
	{
		wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Game", true);
		wxEdit::Instance().GetMainToolbar()->SetToolStatus("PlayGame", true);
		wxEdit::Instance().GetMainToolbar()->SetToolStatus("StopGame", false);
	}

	object->Freeze(true);
	Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)object->GetComponent("View");
	if (visuals != 0)
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
		Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)child->GetComponent("View");
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
		Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)child->GetComponent("View");
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
			Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)(*i).mObject->GetComponent("View");
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
		Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)(*i).mObject->GetComponent("View");
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(false);
		}
		(*i).mObject->Freeze(false);
		DeselectChildren((*i).mObject);
	}
	mSelectedObjects.clear();
}

void Edit::AlignObjectWithMesh(Ice::GameObject *object)
{
	Ogre::Ray mouseRay = Ice::Main::Instance().GetCamera()->getCameraToViewportRay(mMousePosition.x, mMousePosition.y);
	OgrePhysX::Scene::QueryHit report;
  	if (Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(report, mouseRay))
	{
		Ogre::Vector3 position = report.point;
		Ogre::Vector3 normal = report.normal;
		object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(normal.z * 90)));
		object->Rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-normal.x * 90)));
		if (normal.y < 0)
		{
			object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(-normal.y * 180)));
		}
		Ogre::Vector3 offset = Ogre::Vector3(0,0,0);
		Ice::GOCNodeRenderable *visuals = (Ice::GOCNodeRenderable*)object->GetComponent("View");
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
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		OnMouseMove(Ogre::Degree(msg.mData.GetInt("ROT_X_REL")), Ogre::Degree(msg.mData.GetInt("ROT_Y_REL")));
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		//Die Preview node, wenn vorhanden, rotieren
		if (wxEdit::Instance().GetAuiManager().GetPane("preview").IsShown())
		{
			wxEdit::Instance().GetPreviewWindow()->Update(msg.mData.GetFloat("TIME"));
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