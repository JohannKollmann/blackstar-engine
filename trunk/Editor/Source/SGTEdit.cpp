
#include "../Header/SGTEdit.h"
#include "SGTInput.h"
#include "SGTCameraController.h"
#include "SGTScenemanager.h"
#include "SGTObjectLevelRayCaster.h"
#include "SGTPhysicsLevelRayCaster.h"
#include "SGTWeatherController.h"
#include "SGTGamestate.h"
#include "SGTGOCEditorInterface.h"
#include "SGTGOCIntern.h"


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

	mXAxisLock = false;
	mYAxisLock = false;
	mZAxisLock = false;

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

	if (SGTSceneManager::Instance().GetWeatherController() != NULL) SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getUniversalClock()->setTimeScale(0);

	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

};

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
				menu.Append(wxOgre_insertWaypoint, "Insert Waypoint");

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
				if (mSelectedObjects.size() == 1) menu.Append(wxOgre_saveObjectgroup, "Save Object Group");

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
				DeselectMaterial();
				OnSelectObject((float)(ev.GetX()) / SGTMain::Instance().GetWindow()->getWidth(), (float)(ev.GetY()) / SGTMain::Instance().GetWindow()->getHeight());
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
			OnBrush();
			mPerformedLDClick = true;
		}
	}
	if (ev.m_wheelRotation != 0 && mSelectedObjects.size() > 0)
	{
		if (mXAxisLock == true || mYAxisLock == true || mZAxisLock == true)
		{
			for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
			{
				SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("GOCView");
				if (visuals != 0)
				{
					if (visuals->GetComponentID() == "GOCViewContainer")
					{
						SGTMeshRenderable *gocmesh = (SGTMeshRenderable*)((SGTGOCViewContainer*)visuals)->GetItem("MeshRenderable");
						if (gocmesh != 0)
						{
							mPerformingObjMov = true;
							Ogre::Vector3 scaleaxis = Ogre::Vector3((mXAxisLock == true) ? 1.0f : 0.0f, (mYAxisLock == true) ? 1.0f : 0.0f, (mZAxisLock == true) ? 1.0f : 0.0f);	
							(*i).mObject->Rescale(scaleaxis * ev.m_wheelRotation * mObjectScaleSpeed);
							visuals->GetNode()->showBoundingBox(true);
							(*i).mObject->Freeze(true);
							AttachAxisObject((*i).mObject);
						}
					}
					else if (visuals->GetComponentID() == "MeshDebugRenderable")
					{
						visuals->GetNode()->showBoundingBox(true);
					}
				}
			}
		}
		((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
		wxEdit::Instance().GetpropertyWindow()->Refresh();
	}

	ev.Skip();
};

void SGTEdit::AttachAxisObject(SGTGameObject *object)
{
	object->RemoveComponent("GOCAxisObject");
	AxisComponent* caxis = new AxisComponent();
	object->AddComponent(caxis);
}
void SGTEdit::DetachAxisObject(SGTGameObject *object)
{
	object->RemoveComponent("GOCAxisObject");
}

void SGTEdit::OnMouseMove(Ogre::Radian RotX,Ogre::Radian RotY)
{
	if (mLeftDown && mRightDown && !mAltIsDown)
	{
		SGTMain::Instance().GetCamera()->yaw(-RotX * mRotSpeed);
		SGTMain::Instance().GetCamera()->pitch(-RotY * mRotSpeed);
	}
	if (mLeftDown && mRightDown && mAltIsDown)
	{
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(0,1,0), -RotX * mRotSpeed);
		mPivotNode->rotate(mPivotNode->getOrientation().Inverse() * Ogre::Vector3(1,0,0), -RotY * mRotSpeed);
		mPivotOffsetNode->needUpdate(true);
		SGTMain::Instance().GetCamera()->setPosition(mPivotOffsetNode->_getDerivedPosition());
		SGTMain::Instance().GetCamera()->lookAt(mPivotNode->getPosition());
	}

	if (mLeftDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == true)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(RotX.valueRadians() * mObjectMovSpeed,0,0));
				}
			}
			if (mYAxisLock == true)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,-RotY.valueRadians() * mObjectMovSpeed,0));
				}
			}
			if (mZAxisLock == true)
			{
				mPerformingObjMov = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Translate(SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,RotY.valueRadians() * mObjectMovSpeed));
				}
			}
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
	}

	if (mRightDown == true)
	{
		if (mSelectedObjects.size() > 0)
		{
			if (mXAxisLock == true)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * SGTMain::Instance().GetCamera()->getDerivedOrientation().xAxis(), RotY * mObjectRotSpeed);
				}
			}
			if (mYAxisLock == true)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * SGTMain::Instance().GetCamera()->getDerivedOrientation().yAxis(), RotX * mObjectRotSpeed);
				}
			}
			if (mZAxisLock == true)
			{
				mPerformingObjRot = true;
				for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
				{
					(*i).mObject->Rotate((*i).mObject->GetGlobalOrientation().Inverse() * SGTMain::Instance().GetCamera()->getDerivedOrientation().zAxis(), RotX * mObjectRotSpeed);
				}
			}
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}
	}
};

void SGTEdit::OnKeyDown(wxKeyEvent& key)
{
	if (key.GetKeyCode() == wxKeyCode::WXK_SHIFT) mMultiSelect = true;
	if (key.GetKeyCode() == wxKeyCode::WXK_CONTROL) mStrgPressed = true;
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

	if (key.GetKeyCode() == 49) mXAxisLock = true;
	if (key.GetKeyCode() == 50) mYAxisLock = true;
	if (key.GetKeyCode() == 51) mZAxisLock = true;

	if ((key.GetKeyCode() == 66) && (mStrgPressed == true))
	{
		if (mBrushMode == false) mBrushMode = true;
		else mBrushMode = false;
	}
};

void SGTEdit::OnKeyUp(wxKeyEvent& key)
{
	if (key.GetKeyCode() == wxKeyCode::WXK_SHIFT) mMultiSelect = false;
	if (key.GetKeyCode() == wxKeyCode::WXK_CONTROL) mStrgPressed = false;
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

	if (key.GetKeyCode() == 49) mXAxisLock = false;
	if (key.GetKeyCode() == 50) mYAxisLock = false;
	if (key.GetKeyCode() == 51) mZAxisLock = false;

};

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

SGTGameObject* SGTEdit::OnInsertWaypoint()
{
	SGTGameObject *waypoint = SGTSceneManager::Instance().CreateWaypoint();
	waypoint->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5)));
	waypoint->SetGlobalOrientation(Ogre::Quaternion(SGTMain::Instance().GetCamera()->getDerivedOrientation().getYaw(), Ogre::Vector3(0,1,0)));
	SelectObject(waypoint);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
	return waypoint;
}

SGTGameObject* SGTEdit::OnInsertObject(SGTGameObject *parent, bool align)
{
	if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".") != Ogre::String::npos)
	{
		//Ogre::LogManager::getSingleton().logMessage("OnInsertObject");
		SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource());
		SGTGameObject *object;
		if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".ot") != Ogre::String::npos)
		{
			object = (SGTGameObject*)ls->LoadObject();

			if (align) AlignObjectWithMesh(object);
			else object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

			if (parent != NULL) object->SetParent(parent);
		}
		/*else if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".SphericalJoint") != Ogre::String::npos)
		{
			if (parent == NULL)
			{
				ls->CloseFile();
				delete ls;
				Ogre::LogManager::getSingleton().logMessage("SGTEdit: Joint muss parent haben!");
				return NULL;
			}
			SGTDataMap *params = (SGTDataMap*)(ls->LoadObject());
			object = new SGTSphericalJoint();
			if (((SGTBody*)parent)->GetType() == "StaticBody") ((SGTJoint*)object)->InitCustom(params, (SGTBody*)mTempJointBodyB, (SGTAbstractBody*)parent);
			else ((SGTJoint*)object)->InitCustom(params, (SGTBody*)parent, mTempJointBodyB);
			object->SetGlobalPosition(parent->GetGlobalPosition());//SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 
		}
		else if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".RevoluteJoint") != Ogre::String::npos)
		{
			if (parent == NULL)
			{
				ls->CloseFile();
				delete ls;
				Ogre::LogManager::getSingleton().logMessage("SGTEdit: Joint muss parent haben!");
				return NULL;
			}
			SGTDataMap *params = (SGTDataMap*)(ls->LoadObject());
			object = new SGTRevoluteJoint();
			if (((SGTBody*)parent)->GetType() == "StaticBody") ((SGTJoint*)object)->InitCustom(params, (SGTBody*)mTempJointBodyB, (SGTAbstractBody*)parent);
			else ((SGTJoint*)object)->InitCustom(params, (SGTBody*)parent, mTempJointBodyB);
			object->SetGlobalPosition(parent->GetGlobalPosition());//object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 
		}
		else if (wxEdit::Instance().GetWorldExplorer()->GetResourceTree()->GetSelectedResource().find(".FixedJoint") != Ogre::String::npos)
		{
			if (parent == NULL)
			{
				ls->CloseFile();
				delete ls;
				Ogre::LogManager::getSingleton().logMessage("SGTEdit: Joint muss parent haben!");
				return NULL;
			}
			SGTDataMap *params = (SGTDataMap*)(ls->LoadObject());
			object = new SGTFixedJoint();
			if (((SGTBody*)parent)->GetType() == "StaticBody") ((SGTJoint*)object)->InitCustom(params, (SGTBody*)mTempJointBodyB, (SGTAbstractBody*)parent);
			else ((SGTJoint*)object)->InitCustom(params, (SGTBody*)parent, mTempJointBodyB);
			object->SetGlobalPosition(parent->GetGlobalPosition());//object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 
		}*/
		else
		{
			object = new SGTGameObject();
			std::list<ComponentSection> sections;
			Ogre::Vector3 offset;
			Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
			Ogre::Quaternion orientation;
			ls->LoadAtom("std::list<ComponentSection>", (void*)(&sections));
			for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
			{
				if ((*i).mSectionName == "Transform")
				{
					offset = (*i).mSectionData->GetOgreVec3("Position");
					orientation = (*i).mSectionData->GetOgreQuat("Orientation");
					scale = (*i).mSectionData->GetOgreVec3("Scale");
					continue;
				} 
				(*i).mSectionData->AddOgreVec3("Scale", scale);
				SGTGOCEditorInterface *component = SGTSceneManager::Instance().CreateComponent((*i).mSectionName, (*i).mSectionData.getPointer());
				if (component->IsViewComponent())
				{
					SGTGOCViewContainer *container = (SGTGOCViewContainer*)object->GetComponent("GOCViewContainer");
					if (!container)
					{
						container = new SGTGOCViewContainer();
						object->AddComponent(container);
					}
					container->AddItem(dynamic_cast<SGTGOCViewComponent*>(component));
				}
				else
				{
					object->AddComponent(dynamic_cast<SGTGOComponent*>(component));
				}
			}

			if (align) AlignObjectWithMesh(object);
			else object->SetGlobalPosition(SGTMain::Instance().GetCamera()->getDerivedPosition() + (SGTMain::Instance().GetCamera()->getDerivedOrientation() * Ogre::Vector3(0,0,-5))); 

			//SGTMain::Instance().GetNxWorld()->getPhysXDriver()->simulate(0);
		}

		ls->CloseFile();
		delete ls;
		wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->Update();
		SelectObject(object);

		wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->Update();

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
	for (std::list<SGTEditorSelection>::iterator i = mSelectedObjects.begin(); i != mSelectedObjects.end(); i++)
	{
		delete (*i).mObject;
		(*i).mObject = 0;
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
			SGTKernel::Instance().doLoop(); //Kleiner hack, damit alle children die temporären Positionen übernehmen
			ss->SaveObject((*mSelectedObjects.begin()).mObject, "Root");
			ss->CloseFiles();
			(*mSelectedObjects.begin()).mObject->SetGlobalPosition(position);
			SGTKernel::Instance().doLoop();
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

void SGTEdit::OnCreateChain()
{
	wxTextEntryDialog dialog(wxEdit::Instance().GetOgrePane(),
		_T("Enter chain length:"),
		_T("Chain length"),
		_T(""),
		wxOK | wxCANCEL);

	Ogre::String sChainLength = "";
	/*if (dialog.ShowModal() == wxID_OK)
	{
		sChainLength = dialog.GetValue().c_str();
		int iChainLength = Ogre::StringConverter::parseInt(sChainLength);
		SGTGameObject *object1 = (*mSelectedObjects.begin()).mObject;
		SGTGameObject *object2 = mSelectedObjects.begin()._Mynode()->_Next->_Myval.mObject;
		SGTDataMap *object1Params = new SGTDataMap();
		object1->GetParams(object1Params);
		SGTDataMap *object2Params = new SGTDataMap();
		object2->GetParams(object2Params);

		SGTJoint *joint = NULL;
		for (unsigned short i = 0; i < object1->GetNumChildren(); i++)
		{
			bool jointfound = false;
			SGTGameObject *child = object1->GetChild(i);
			if (child->GetType().find("Joint") != Ogre::String::npos)
			{
				for (unsigned short x = 0; x < object2->GetNumChildren(); x++)
				{
					if (object2->GetChild(x) == child)
					{
						joint = (SGTJoint*)(child);
					}
				}
			}
			if (jointfound) break;
		}
		SGTDataMap *jointParams = new SGTDataMap();
		joint->GetParams(jointParams);
		Ogre::Vector3 vObjectOffset = object2->GetGlobalPosition() - object1->GetGlobalPosition();
		Ogre::Vector3 vJointOffset = joint->GetGlobalPosition() - object1->GetGlobalPosition();
		for (int i = 1; i <= iChainLength - 2; i++)
		{
			if (i % 2 == 0)
			{
				object2 = SGTSceneManager::Instance().CreateEntity(object2->GetType(), *object2Params, object2->GetParent());
				object2->SetGlobalPosition(object1->GetGlobalPosition() + vObjectOffset);
				SGTJoint *newjoint = NULL;
				if (joint->GetType() == "SphericalJoint")
				{
					newjoint = new SGTSphericalJoint();
				}
				if (joint->GetType() == "RevoluteJoint")
				{
					newjoint = new SGTRevoluteJoint();
				}
				if (joint->GetType() == "FixedJoint")
				{
					newjoint = new SGTFixedJoint();
				}
				newjoint->InitCustom(jointParams, (SGTBody*)object1, (SGTAbstractBody*)object2);
				newjoint->SetGlobalPosition(object1->GetGlobalPosition() + vJointOffset);
			}
			else
			{
				object1 = SGTSceneManager::Instance().CreateEntity(object2->GetType(), *object1Params, object1->GetParent());
				object1->SetGlobalPosition(object2->GetGlobalPosition() + vObjectOffset);
				SGTJoint *newjoint = NULL;
				if (joint->GetType() == "SphericalJoint")
				{
					newjoint = new SGTSphericalJoint();
				}
				if (joint->GetType() == "RevoluteJoint")
				{
					newjoint = new SGTRevoluteJoint();
				}
				if (joint->GetType() == "FixedJoint")
				{
					newjoint = new SGTFixedJoint();
				}
				newjoint->InitCustom(jointParams, (SGTBody*)object2, (SGTAbstractBody*)object1);
				newjoint->SetGlobalPosition(object2->GetGlobalPosition() + vJointOffset);
			}
		}

		delete object1Params;
		delete object2Params;
		delete jointParams;*/

	
}

void SGTEdit::OnSelectMaterial(float MouseX, float MouseY)
{
	DeselectMaterial();

	if (wxEdit::Instance().GetWorldExplorer()->GetSelection() < 2) wxEdit::Instance().GetWorldExplorer()->AdvanceSelection(true);
	if (wxEdit::Instance().GetWorldExplorer()->GetSelection() < 2) wxEdit::Instance().GetWorldExplorer()->AdvanceSelection(true);

	Ogre::Ray ray = SGTMain::Instance().GetCamera()->getCameraToViewportRay(MouseX, MouseY);
	SGTObjectLevelRayCaster rc(ray);
	SGTGameObject *object = rc.GetFirstHit(true);
	if (object)
	{
		SGTGOCNodeRenderable *visuals = (SGTGOCViewContainer*)object->GetComponent("GOCView");
		if (visuals != 0)
		{
			if (visuals->GetComponentID() == "GOCViewContainer")
			{
				SGTMeshRenderable *gocmesh = (SGTMeshRenderable*)((SGTGOCViewContainer*)visuals)->GetItem("MeshRenderable");
				if (gocmesh != 0)
				{
					SGTEntityMaterialInspector emi((Ogre::Entity*)gocmesh->GetEditorVisual());
					Ogre::SubEntity *ent = emi.GetSubEntity(ray);
					if (ent == NULL) return;
					mCurrentMaterialSelection.mSubEntity = ent;
					mCurrentMaterialSelection.mOriginalMaterialName = ent->getMaterialName();
					ent->setMaterialName("Editor_Submesh_Selected");
					wxEdit::Instance().GetWorldExplorer()->GetMaterialTree()->ExpandToMaterial(((Ogre::Entity*)gocmesh->GetEditorVisual())->getMesh()->getName(), mCurrentMaterialSelection.mOriginalMaterialName);
					wxEdit::Instance().GetOgrePane()->SetFocus();
					return;
				}
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
	OnInsertObject(NULL, true);
}

void SGTEdit::SelectObject(SGTGameObject *object)
{
	//Ogre::LogManager::getSingleton().logMessage("Select Object " + object->GetName());
	if (!mMultiSelect) DeselectAllObjects();
	else DeselectObject(object);

	if (wxEdit::Instance().GetWorldExplorer()->GetSelection() > 0) wxEdit::Instance().GetWorldExplorer()->AdvanceSelection(false);
	if (wxEdit::Instance().GetWorldExplorer()->GetSelection() > 0) wxEdit::Instance().GetWorldExplorer()->AdvanceSelection(false);
	wxEdit::Instance().GetWorldExplorer()->GetSceneTree()->ExpandToObject(object);
	wxEdit::Instance().GetpropertyWindow()->SetPage("EditGameObject");
	((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(object);

	object->Freeze(true);
	SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)object->GetComponent("GOCView");
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
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)child->GetComponent("GOCView");
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
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)child->GetComponent("GOCView");
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
			SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("GOCView");
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
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)(*i).mObject->GetComponent("GOCView");
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
		SGTGOCNodeRenderable *visuals = (SGTGOCNodeRenderable*)object->GetComponent("GOCView");
		if (visuals != 0)
		{
			offset = visuals->GetNode()->getScale() * 0.5;//object->GetVisual()->getBoundingBox().getSize()
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
		/*if (wxEdit::Instance().GetpropertyWindow()->GetCurrentPageName() == "EditGameObject")
		{
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->Update();
			if (((wxEditSGTGameObject*)wxEdit::Instance().GetpropertyWindow()->GetCurrentPage())->GetGameObject() != NULL)
			((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->SetObject(((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->GetGameObject());
			wxEdit::Instance().GetpropertyWindow()->Refresh();
		}*/
	}
};