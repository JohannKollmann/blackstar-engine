
#include "../Header/SGTEdit.h"
#include "SGTInput.h"
#include "SGTCameraController.h"
#include "SGTScenemanager.h"
#include "SGTObjectLevelRayCaster.h"
#include "SGTPhysicsLevelRayCaster.h"
#include "SGTWeatherController.h"
#include "SGTMainLoop.h"
#include "SGTGOCEditorInterface.h"
#include "SGTGOCIntern.h"
#include "SGTGOCAnimatedCharacter.h"


SGTEdit::SGTEdit()
{
	mEdit = &wxEdit::Instance();

	mCamRotating = false;
	mLeftDown = false;
	mRightDown = false;
	mMultiSelect = false;
	mInsertAsChild = false;
	mStrgPressed = false;
	mAltIsDown = false;

	mMaterialMode = false;
	mBrushMode = false;

	mXAxisLock = SGTAxisLock::LOCKED;
	mYAxisLock = SGTAxisLock::LOCKED;
	mZAxisLock = SGTAxisLock::LOCKED;

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
	SGTMain::Instance().GetCameraController()->mMove = true;
	SGTMain::Instance().GetCameraController()->mYRot = false;
	SGTMain::Instance().GetCameraController()->mXRot = false;

	mPivotNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mPivotOffsetNode = mPivotNode->createChildSceneNode();

	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	//wxEdit::Instance().GetMainToolbar()->RegisterTool("SaveWorld", "General", "Data/Editor/Intern/Engine_Icon08.png", SGTEdit::OnToolbarEvent);
	//wxEdit::Instance().GetMainToolbar()->SetGroupStatus("General", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("Physics", "WorldSettings", "Data/Editor/Intern/Editor_gewicht_04.png", SGTEdit::OnToolbarEvent, "Enables/Disables Physics simulation", true, true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("TimeCycle", "WorldSettings", "Data/Editor/Intern/editor_clock.png", SGTEdit::OnToolbarEvent, "Enables/Disables game clock", true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("WorldSettings", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("XAxisLock", "Transform", "Data/Editor/Intern/xAxis.png", SGTEdit::OnToolbarEvent, "Enables/Disables X Axis lock for object movements/rotations", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("YAxisLock", "Transform", "Data/Editor/Intern/yAxis.png", SGTEdit::OnToolbarEvent, "Enables/Disables Y Axis lock for object movements/rotations", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("ZAxisLock", "Transform", "Data/Editor/Intern/zAxis.png", SGTEdit::OnToolbarEvent, "Enables/Disables Z Axis lock for object movements/rotations", true);
	//wxEdit::Instance().GetMainToolbar()->RegisterTool("MoveRelative", "Transform", "Data/Editor/Intern/BrushMode.png", SGTEdit::OnToolbarEvent, true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("Transform", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("BrushMode", "EditMode", "Data/Editor/Intern/BrushMode.png", SGTEdit::OnToolbarEvent, "Enables/Disables the Brush mode for object placement", true);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("MaterialMode", "EditMode", "Data/Editor/Intern/materialmode.png", SGTEdit::OnToolbarEvent, "Enables/Disables the Material mode for material selection", true);
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("EditMode", true);

	wxEdit::Instance().GetMainToolbar()->RegisterTool("InsertObject", "ObjectMgr", "Data/Editor/Intern/Engine_Icon02.png", SGTEdit::OnToolbarEvent, "Inserts an object");
	//wxEdit::Instance().GetMainToolbar()->RegisterTool("InsertObjectAsChild", "ObjectMgr", "Data/Editor/Intern/BrushMode.png", SGTEdit::OnToolbarEvent);
	wxEdit::Instance().GetMainToolbar()->RegisterTool("DeleteObject", "ObjectMgr", "Data/Editor/Intern/Engine_Icon04.png", SGTEdit::OnToolbarEvent, "Removes an object");
	wxEdit::Instance().GetMainToolbar()->RegisterTool("SaveObjectGroup", "ObjectMgr", "Data/Editor/Intern/Engine_Icon06.png", SGTEdit::OnToolbarEvent, "Save Object group");
	wxEdit::Instance().GetMainToolbar()->SetGroupStatus("ObjectMgr", true);
};

void SGTEdit::OnToolbarEvent(int toolID, Ogre::String toolname)
{
	SGTEdit *edit = wxEdit::Instance().GetOgrePane()->GetEdit();
	bool checked = wxEdit::Instance().GetMainToolbar()->GetToolState(toolID);
	if (toolname == "XAxisLock")
	{
		if (checked) edit->mXAxisLock = SGTAxisLock::UNLOCKED;
		else edit->mXAxisLock = SGTAxisLock::LOCKED;
	}
	if (toolname == "YAxisLock")
	{
		if (checked) edit->mYAxisLock = SGTAxisLock::UNLOCKED;
		else edit->mYAxisLock = SGTAxisLock::LOCKED;
	}
	if (toolname == "ZAxisLock")
	{
		if (checked) edit->mZAxisLock = SGTAxisLock::UNLOCKED;
		else edit->mZAxisLock = SGTAxisLock::LOCKED;
	}

	if (toolname == "Physics")
	{
		SGTMainLoop::Instance().SetPhysics(checked);
	}
	if (toolname == "TimeCycle")
	{
		SGTSceneManager::Instance().EnableClock(checked);
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
}

void SGTEdit::OnMouseEvent(wxMouseEvent &ev)
{
	mLeftDown = ev.LeftIsDown();
	mRightDown = ev.RightIsDown();

	if (ev.LeftIsDown() || ev.RightIsDown())
	{
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
	/*
	Sicherstellen, dass keine Kamerarotation vorliegt oder gerade abgeschlossen wird (sonst Abbruch)
	*/
	if (ev.LeftIsDown() && ev.RightIsDown())
	{
		mCamRotating = true;
		return;
	};
	if ((mCamRotating == true) && (mLeftDown == false) && (mRightDown == false))
	{
		mCamRotating = false;
		return;
	}
	if (mCamRotating == true) return;

	mMousePosition.x = (float)(ev.GetX()) / SGTMain::Instance().GetWindow()->getWidth();
	mMousePosition.y = (float)(ev.GetY()) / SGTMain::Instance().GetWindow()->getHeight();

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
					SGTGameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					SGTGameObject* obj2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;
					if (obj1->GetComponent("Waypoint") && obj2->GetComponent("Waypoint")) menu.Append(wxOgre_connectWaypoints, "Connect Waypoints");
					/*if ((obj1->GetType() == "Body" || obj1->GetType() == "StaticBody") && (obj2->GetType() == "Body" || obj2->GetType() == "StaticBody")
						&& mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find("Joint") != Ogre::String::npos)
					{
						menu.Append(wxOgre_insertObjectAsChild, "Insert " + mEdit->GetWorldExplorer()->GetResourceTree()->GetSelectedResource() + " as Child of " +  obj2->GetName());
						mTempJointBodyB = (SGTAbstractBody*)obj1;
					}
					if (obj1->GetType() == "Body" && obj2->GetType() == "Body")
					{
						for (unsigned short i = 0; i < obj1->GetNumChildren(); i++)
						{
							bool chainoptionappended = false;
							SGTGameObject *child = obj1->GetChild(i);
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

					SGTGameObject* obj1 = (*mSelectedObjects.begin()).mObject;
					if (obj1->GetComponent("Skeleton"))
					{
						menu.Append(wxOgre_saveBones, "Save Bone Config");
					}
				}

				if (mSelectedObjects.size() >= 1) menu.Append(wxOgre_deleteObject, "Delete");
			}

			mEdit->GetOgrePane()->PopupMenu(&menu, clientpt);

		}
		else mPerformingObjRot = false;

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
					OnSelectObject((float)(ev.GetX()) / SGTMain::Instance().GetWindow()->getWidth(), (float)(ev.GetY()) / SGTMain::Instance().GetWindow()->getHeight());
				}
			}
		}
		else mPerformingObjMov = false;
		mPerformedLDClick = false;
	}
	if (ev.LeftDClick())
	{
		if (mMaterialMode)
		{
			mPerformedLDClick = true;
			OnSelectMaterial((float)(ev.GetX()) / SGTMain::Instance().GetWindow()->getWidth(), (float)(ev.GetY()) / SGTMain::Instance().GetWindow()->getHeight());
		}
		if (mBrushMode)
		{
			//OnBrush();
			mPerformedLDClick = true;
		}
	}
	if (ev.m_wheelRotation != 0 && mSelectedObjects.size() > 0)
	{
		if (mXAxisLock == SGTAxisLock::UNLOCKED || mYAxisLock == SGTAxisLock::UNLOCKED || mZAxisLock == SGTAxisLock::UNLOCKED)
		{
			for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				/*SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("View");
				if (visuals != 0)
				{
					if (visuals->GetComponentID() == "ViewContainer")
					{
						SGTMeshRenderable *gocmesh = (SGTMeshRenderable*)((SGTGOCViewContainer*)visuals)->GetItem("MeshRenderable");
						if (gocmesh != 0)
						{
							mPerformingObjMov = true;
							Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f, (mYAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f, (mZAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f);	
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
				Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f, (mYAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f, (mZAxisLock == SGTAxisLock::UNLOCKED) ? 1.0f : 0.0f);	
				(*i).mObject->Rescale(scaleaxis * ev.m_wheelRotation * mObjectScaleSpeed);
				(*i).mObject->Freeze(true);
				AttachAxisObject((*i).mObject);
			}
		}
		((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}

	ev.Skip();
};

void SGTEdit::AttachAxisObject(SGTGameObject *object)
{
	object->RemoveComponent("AxisObject");
	AxisComponent* caxis = new AxisComponent();
	object->AddComponent(caxis);
}
void SGTEdit::DetachAxisObject(SGTGameObject *object)
{
	object->RemoveComponent("AxisObject");
}

void SGTEdit::OnMouseMove(Ogre::Radian RotX,Ogre::Radian RotY)
{
	if (mLeftDown && mRightDown && !mAltIsDown)
	{
		SGTMain::Instance().GetCamera()->yaw(-RotX * mRotSpeed);
		SGTMain::Instance().GetCamera()->pitch(-RotY * mRotSpeed);
	}
	else if (mLeftDown && mRightDown && mAltIsDown)
	{
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(0,1,0), -RotX * mRotSpeed);
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(1,0,0), -RotY * mRotSpeed);
		mPivotOffsetNode->needUpdate(true);
		SGTMain::Instance().GetCamera()->setPosition(mPivotOffsetNode->_getDerivedPosition());
		SGTMain::Instance().GetCamera()->lookAt(mPivotNode->getPosition());
	}

	else if (mLeftDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == SGTAxisLock::UNLOCKED || mXAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(RotX.valueRadians() * mObjectMovSpeed,0,0), mXAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			if (mYAxisLock == SGTAxisLock::UNLOCKED || mYAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(Ogre::Vector3(0,-RotY.valueRadians() * mObjectMovSpeed,0), mYAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			if (mZAxisLock == SGTAxisLock::UNLOCKED || mZAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					Ogre::Vector3 movaxis = SGTMain::Instance().GetCamera()->getDerivedDirection() * -1.0f;
					movaxis.y = 0;
					movaxis.normalise();
					(*i).mObject->Translate(movaxis * RotY.valueRadians() * mObjectMovSpeed , mZAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
	}
	else if (mRightDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == SGTAxisLock::UNLOCKED || mXAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * SGTMain::Instance().GetCamera()->getDerivedOrientation().xAxis(), RotY * mObjectRotSpeed, mXAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			if (mYAxisLock == SGTAxisLock::UNLOCKED || mYAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * Ogre::Vector3::UNIT_Y, RotX * mObjectRotSpeed, mYAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			if (mZAxisLock == SGTAxisLock::UNLOCKED || mZAxisLock == SGTAxisLock::UNLOCKED_SKIPCHILDREN)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					Ogre::Vector3 rotaxis = SGTMain::Instance().GetCamera()->getDerivedDirection() * -1.0f;
					rotaxis.y = 0;
					rotaxis.normalise();
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * rotaxis, RotX * mObjectRotSpeed, mZAxisLock == SGTAxisLock::UNLOCKED);
				}
			}
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
	}
	else
	{
		for (std::list<SGTGameObject*>::iterator i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
		{
			(*i)->SetGlobalOrientation(Ogre::Quaternion());
			AlignObjectWithMesh(*i);
		}
	}
};

void SGTEdit::OnKeyDown(wxKeyEvent& key)
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
		mPivotOffsetNode->setPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() - mPivotNode->getPosition());
		mAltIsDown = true;
	}

	if (key.GetKeyCode() == 49) mXAxisLock = SGTAxisLock::UNLOCKED;
	if (key.GetKeyCode() == 50) mYAxisLock = SGTAxisLock::UNLOCKED;
	if (key.GetKeyCode() == 51) mZAxisLock = SGTAxisLock::UNLOCKED;
	if (key.GetKeyCode() == 52) mXAxisLock = SGTAxisLock::UNLOCKED_SKIPCHILDREN;
	if (key.GetKeyCode() == 53) mYAxisLock = SGTAxisLock::UNLOCKED_SKIPCHILDREN;
	if (key.GetKeyCode() == 54) mZAxisLock = SGTAxisLock::UNLOCKED_SKIPCHILDREN;
	wxEdit::Instance().GetMainToolbar()->CheckTool("XAxisLock", mXAxisLock != SGTAxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("YAxisLock", mYAxisLock != SGTAxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("ZAxisLock", mZAxisLock != SGTAxisLock::LOCKED);
};

void SGTEdit::OnKeyUp(wxKeyEvent& key)
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

	if (key.GetKeyCode() == 49) mXAxisLock = SGTAxisLock::LOCKED;
	if (key.GetKeyCode() == 50) mYAxisLock = SGTAxisLock::LOCKED;
	if (key.GetKeyCode() == 51) mZAxisLock = SGTAxisLock::LOCKED;
	if (key.GetKeyCode() == 52) mXAxisLock = SGTAxisLock::LOCKED;
	if (key.GetKeyCode() == 53) mYAxisLock = SGTAxisLock::LOCKED;
	if (key.GetKeyCode() == 54) mZAxisLock = SGTAxisLock::LOCKED;
	wxEdit::Instance().GetMainToolbar()->CheckTool("XAxisLock", mXAxisLock != SGTAxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("YAxisLock", mYAxisLock != SGTAxisLock::LOCKED);
	wxEdit::Instance().GetMainToolbar()->CheckTool("ZAxisLock", mZAxisLock != SGTAxisLock::LOCKED);
};

void SGTEdit::OnSelectResource()
{
	if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_RCONTROL) || mBrushMode)
	{
		ClearPreviewObject();
		CreatePreviewObject();
	}
}

void SGTEdit::ClearPreviewObject()
{
	for (std::list<SGTGameObject*>::iterator i = mPreviewObjects.begin(); i != mPreviewObjects.end(); i++)
	{
		delete (*i);
	}
	mPreviewObjects.clear();
}
void SGTEdit::CreatePreviewObject()
{
	SGTGameObject *preview = OnInsertObject(0, true, true);
	if (preview)
	{
		std::vector<Ogre::String> components = preview->GetComponentsStr();
		for (std::vector<Ogre::String>::iterator i = components.begin(); i != components.end(); i++)
		{
			if ((*i) != "View" && (*i) != "MeshDebugRenderable")
			{
				preview->RemoveComponent((*i));
			}
		}
		mPreviewObjects.push_back(preview);
	}
}

void SGTEdit::OnLoadWorld(Ogre::String fileName)
{
	wxEdit::Instance().GetpropertyWindow()->SetPage("None");
	SGTSceneManager::Instance().LoadLevel(fileName);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();
	wxEdit::Instance().GetOgrePane()->SetFocus();
};

void SGTEdit::OnSaveWorld(Ogre::String fileName)
{
	SGTSceneManager::Instance().SaveLevel(fileName);
};

SGTGameObject* SGTEdit::OnInsertWaypoint(bool align, bool create_only)
{
	SGTGameObject *waypoint = new SGTGameObject();
	waypoint->AddComponent(new SGTGOCWaypoint());
	waypoint->AddComponent(new SGTMeshDebugRenderable("Editor_Waypoint.mesh"));
	waypoint->ShowEditorVisuals(true);
	waypoint->SetGlobalOrientation(Ogre::Quaternion(SGTMain::Instance().GetCamera()->getDerivedOrientation().getYaw(), Ogre::Vector3(0,1,0)));
	if (align) AlignObjectWithMesh(waypoint);
	else waypoint->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	if (mSelectedObjects.size() == 1)
	{
		SGTGOCWaypoint *wp2 = (SGTGOCWaypoint*)(*mSelectedObjects.begin()).mObject->GetComponent("Waypoint");
		if (wp2) ((SGTGOCWaypoint*)waypoint->GetComponent("Waypoint"))->ConnectWaypoint(wp2);
	}
	if (!create_only)
	{
		SelectObject(waypoint);
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->NotifyObject(waypoint);
	}
	wxEdit::Instance().GetOgrePane()->SetFocus();
	return waypoint;
}

SGTGameObject* SGTEdit::OnInsertObject(SGTGameObject *parent, bool align, bool create_only)
{
	if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find("Waypoint.static") != Ogre::String::npos)
	{
		return OnInsertWaypoint(align, create_only);
	}
	else if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".") != Ogre::String::npos)
	{
		//Ogre::LogManager::getSingleton().logMessage("OnInsertObject");
		SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
		SGTGameObject *object;
		if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".ot") != Ogre::String::npos)
		{
			object = (SGTGameObject*)ls->LoadObject();

			if (parent != NULL) object->SetParent(parent);
		}
		else
		{
			object = new SGTGameObject(parent);
			if (!align) object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

			std::list<ComponentSection> sections;
			Ogre::Vector3 offset;
			Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
			Ogre::Quaternion orientation;
			ls->LoadAtom("std::list<ComponentSection>", (void*)(&sections));
			for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
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

				SGTGOCEditorInterface *component = SGTSceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
				if (component) component->AttachToGO(object);
			}

		}

		if (align) AlignObjectWithMesh(object);
		else object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

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

void SGTEdit::OnInsertObjectAsChild()
{
	if (mSelectedObjects.size() > 0) OnInsertObject(mSelectedObjects.end()._Mynode()->_Prev->_Myval.mObject);
}

void SGTEdit::OnDeleteObject()
{
	bool skip = false;
	if (mSelectedObjects.size() == 1)
	{
		SGTGameObject *object = (*mSelectedObjects.begin()).mObject;
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
		for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
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

void SGTEdit::OnCreateObjectGroup()
{
	SGTGameObject *parent = new SGTGameObject();
	parent->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		(*i).mObject->SetParent(parent);
	}
	DeselectAllObjects();
	SelectObject(parent);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
}

void SGTEdit::OnSaveObjectGroup()
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
			SGTSaveSystem *ss=SGTLoadSave::Instance().CreateSaveFile(dialog.GetPath().c_str(), Ogre::String(dialog.GetPath().c_str()) + ".xml");
			Ogre::Vector3 position = (*mSelectedObjects.begin()).mObject->GetGlobalPosition();
			(*mSelectedObjects.begin()).mObject->SetGlobalPosition(Ogre::Vector3(0,0,0));
			SGTMainLoop::Instance().doLoop(); //Kleiner hack, damit alle children die temporären Positionen übernehmen
			ss->SaveObject((*mSelectedObjects.begin()).mObject, "Root");
			ss->CloseFiles();
			(*mSelectedObjects.begin()).mObject->SetGlobalPosition(position);
			SGTMainLoop::Instance().doLoop();
			delete ss;
		}
	}
	wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->SetRootPath("Data/Editor/Objects");
}

void SGTEdit::OnConnectWaypoints()
{
	SGTGOCWaypoint* wp1 = (SGTGOCWaypoint*)(*mSelectedObjects.begin()).mObject->GetComponent("Waypoint");
	SGTGOCWaypoint* wp2 = (SGTGOCWaypoint*)mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject->GetComponent("Waypoint");
	if (!wp1->HasConnectedWaypoint(wp2)) wp1->ConnectWaypoint(wp2);
	else wp1->DisconnectWaypoint(wp2);
}

void SGTEdit::OnSaveBones()
{
	SGTGOCAnimatedCharacter *ragdoll = (SGTGOCAnimatedCharacter*)(*mSelectedObjects.begin()).mObject->GetComponent("Skeleton");
	ragdoll->SerialiseBoneObjects("Data\\Scripts\\Animation\\" + ragdoll->GetRagdoll()->GetEntity()->getMesh()->getName() + ".bones");
}

void SGTEdit::OnCreateChain()
{
	wxTextEntryDialog dialog(wxEdit::Instance().GetOgrePane(),
		_T("Enter chain length:"),
		_T("Chain length"),
		_T(""),
		wxOK | wxCANCEL);

	Ogre::String sChainLength = "";
}

void SGTEdit::OnSelectMaterial(float MouseX, float MouseY)
{
	DeselectMaterial();

	//Potenziell unsicher!
	wxEdit::Instance().GetWorldExplorer()->SetSelection(3);

	Ogre::Ray ray = SGTMain::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY);
	SGTObjectLevelRayCaster rc(ray);
	SGTGameObject *object = rc.GetFirstHit(true);
	if (object)
	{
		SGTGOCViewContainer *visuals = (SGTGOCViewContainer*)object->GetComponent("View", "ViewContainer");
		if (visuals != 0)
		{
			SGTMeshRenderable *gocmesh = (SGTMeshRenderable*)visuals->GetItem("MeshRenderable");
			if (gocmesh != 0)
			{
				SGTEntityMaterialInspector emi((Ogre::Entity*)gocmesh->GetEditorVisual());
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

	if (SGTSceneManager::Instance().HasLevelMesh())
	{
		SGTEntityMaterialInspector emi(SGTMain::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity"));
		Ogre::SubEntity *ent = emi.GetSubEntity(ray);
		if (ent)
		{
			//Ogre::LogManager::getSingleton().logMessage("change material: " + ent->getMaterialName());
			mCurrentMaterialSelection.mSubEntity = ent;
			mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
			ent->setMaterialName("Editor_Submesh_Selected");
			wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(SGTMain::Instance().GetOgreSceneMgr()->getEntity("LevelMesh-entity")->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
			wxEdit::Instance().GetOgrePane()->SetFocus();
		}
	}
}

void SGTEdit::DeselectMaterial()
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

void SGTEdit::OnSelectObject(float MouseX, float MouseY)
{
	//Ogre::LogManager::getSingleton().logMessage("OnSelectObject");
	if (!mMultiSelect) DeselectAllObjects();
	SGTObjectLevelRayCaster rc(SGTMain::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY));
	SGTGameObject *object = rc.GetFirstHit();
	if (object != NULL)
	{
		SelectObject(object);
		wxEdit::Instance().GetOgrePane()->SetFocus();
	}
}

void SGTEdit::OnBrush()
{
	OnInsertObject(0, true);
}

void SGTEdit::SelectObject(SGTGameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Select Object " + object->GetName());
	if (!mMultiSelect) DeselectAllObjects();
	else DeselectObject(object);

	wxEdit::Instance().GetWorldExplorer()->SetSelection(4);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->ExpandToObject(object);
	wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(object);

	object->Freeze(true);
	SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)object->GetComponent("View");
	if (visuals != 0)
	{
		visuals->GetNode()->showBoundingBox(true);
	}
	SGTEditorSelection sel;
	sel.mObject = object;
	AttachAxisObject(object);
	mSelectedObjects.push_back(sel);
	SelectChildren(object);
	//Ogre::LogManager::getSingleton().logMessage("SelectObject " + object->GetName());
}

void SGTEdit::SelectChildren(SGTGameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("select Children " + object->GetName());
	for (unsigned short i = 0; i < object->GetNumChildren(); i++)
	{
		SGTGameObject *child = object->GetChild(i);
		child->Freeze(true);
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)child->GetComponent("View");
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(true);
		}
		SelectChildren(child);
	}
}

void SGTEdit::DeselectChildren(SGTGameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Deselect Children " + object->GetName());
	for (unsigned short i = 0; i < object->GetNumChildren(); i++)
	{
		SGTGameObject *child = object->GetChild(i);
		child->Freeze(false);
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)child->GetComponent("View");
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(false);
		}
		DeselectChildren(child);
	}
}

bool SGTEdit::ObjectIsSelected(SGTGameObject *object)
{
	for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		if ((*i).mObject == object)
		{
			return true;
		}
	}
	return false;
}

void SGTEdit::DeselectObject(SGTGameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Deselect Object " + object->GetName());
	DeselectChildren(object);
	for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		if ((*i).mObject == object)
		{
			DetachAxisObject((*i).mObject);
			SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("View");
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

void SGTEdit::DeselectAllObjects()
{
	//Ogre::LogManager::getSingleton().logMessage("Deselect all Objects");
	for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		DetachAxisObject((*i).mObject);
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("View");
		if (visuals != 0)
		{
			visuals->GetNode()->showBoundingBox(false);
		}
		(*i).mObject->Freeze(false);
		DeselectChildren((*i).mObject);
	}
	mSelectedObjects.clear();
}

void SGTEdit::AlignObjectWithMesh(SGTGameObject *object)
{
	Ogre::Ray mouseRay = SGTMain::Instance().GetCamera()->getCameraToViewportRay(mMousePosition.x, mMousePosition.y);
	NxOgre::Raycaster *Ray = SGTMain::Instance().GetNxScene()->createRaycaster(); 
	Ray->setToDefault();
	Ray->filter(NxOgre::Raycaster::ActorFilter::AF_None);
	Ray->type(NxOgre::Raycaster::RaycastType::RCT_Closest);
	Ray->distance(200);
	Ray->accuracy(NxOgre::Raycaster::Accuracy::AC_Accurate);
	Ray->origin(mouseRay.getOrigin());
	Ray->normal(mouseRay.getDirection().normalisedCopy());
	Ray->cast();
	NxOgre::ActorRaycastHitNodeIterator actors = Ray->Actors();
	NxOgre::ActorRaycastHit* hit = actors.Begin();
	if (hit != NULL)
	{
		Ogre::Vector3 position = hit->getImpactAsOgreVector3();
		Ogre::Vector3 normal = hit->getNormalAsOgreVector3();
		//Ogre::LogManager::getSingleton().logMessage("Raycast hit: " + Ogre::StringConverter::toString(position));
		object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(normal.z * 90)));
		object->Rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-normal.x * 90)));
		if (normal.y < 0)
		{
			object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(-normal.y * 180)));
		}
		Ogre::Vector3 offset = Ogre::Vector3(0,0,0);
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)object->GetComponent("View");
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
	SGTMain::Instance().GetNxScene()->destroyRaycaster(Ray);

	/*if (!rayCaster.isEmpty())
	{
		if (rayCaster.getClosestActorHit() != NULL)
		{
			Ogre::Vector3 position = rayCaster.getClosestRaycastHit().mWorldImpact;
			Ogre::Vector3 normal = rayCaster.getClosestRaycastHit().mWorldNormal;

			object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(normal.z * 90)));
			object->Rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-normal.x * 90)));
			if (normal.y < 0)
			{
				object->Rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(Ogre::Degree(-normal.y * 180)));
			}
			Ogre::Vector3 offset = Ogre::Vector3(0,0,0);
			if (object->GetVisual()) offset = normal * object->GetNode()->getScale() * 0.5;//object->GetVisual()->getBoundingBox().getSize()
			object->SetGlobalPosition(position + offset);
		}
	}*/

}


void SGTEdit::SetCameraMoveSpeed(float factor)
{
	SGTMain::Instance().GetCameraController()->mMoveSpeed = factor;
}

void SGTEdit::SetCameraRotationSpeed(float factor)
{
	mRotSpeed = factor;
}

void SGTEdit::SetObjectMoveSpeed(float factor)
{
	mObjectMovSpeed = factor;
}

void SGTEdit::SetObjectRotationSpeed(float factor)
{
	mObjectRotSpeed = factor;
}



void SGTEdit::ReceiveMessage(SGTMsg &msg)
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
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->Update();
			if (((wxEditSGTGameObject*)wxEdit::Instance().GetpropertyWindow()->GetCurrentPage())->GetGameObject() != NULL)
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}*/
	}
};