enum GunBuilderUI_PanMode {
	NONE,
	CAMERA,
	LIGHTS
}

class GunBuilderUI_PreviewUIComponent : ScriptedWidgetComponent {
	
	
	// 
	static ref array<ref Color> s_LightColors = {
		new Color(0.656992, 0.893004, 1.000000, 0.000000),
		new Color(0.931670, 0.309651, 0.075288, 0.000000),
		new Color(0.208148, 0.041321, 0.799039, 0.000000)
	};
	static int s_LightSelectedColor = 0;
	
	Widget m_wRoot;
	RenderTargetWidget m_wRenderTarget;
	
	// references to previewed entities
	InventoryItemComponent m_previewedItemComponent = null;
	InventoryItemComponent m_previewedStorageItemComponent = null;
	
	IEntity m_storageEntity;
	
	// InventoryStorageSlot m_previewedSlot = null;
	
	Bacon_GunBuilderUI_SlotInfo m_previewedSlotInfo = null;
	
	InventoryItemComponent m_characterItemComponent = null;
	
	// 
	BaseWorld m_PreviewWorld = null;
	ref SharedItemRef m_PreviewSharedItemRef = null;
	int	m_PreviewCamera	= -1;
	GenericEntity m_PreviewEntity = null;

	vector m_targetCameraPosition = {5, 1, 5};
	vector m_targetCameraLookPosition = {0, 0.5, 0};

	vector m_previewedEntityCenterWorld = vector.Zero;
	float m_previewedEntitySize = 0;
	float m_previewedEntityDefaultDistance = 1;

	vector m_currentCameraLookPosition;
	vector m_currentCameraMatrix[4];
	
	float m_cameraDistanceToTarget;
	float m_cameraLookDistanceToTarget;

	vector m_cameraTargetDirection;
	float m_cameraTargetDistance;
	
	Bacon_GunBuilderUI_Mode m_previewMode;
	
	InputManager m_pInputManager;
	
	GunBuilderUI_PanMode m_ePanMode = GunBuilderUI_PanMode.NONE;
	
	IEntity m_LightEntities;
	
	bool m_cameraTargetChangeAllowed = true;
	bool m_bIsFocused = false;
	
	bool m_bDisableCameraAutoFocus = false;
	
	static ResourceName m_PreviewWorldResourceName = "{E91350F79536248E}Prefabs/BaconLoadoutEditor/PreviewWorld/GunBuilderPreviewWorld.et";
	static ResourceName m_PreviewWorldLightsResourceName = "{336AC58F7064BACD}Prefabs/BaconLoadoutEditor/PreviewWorld/LightEntities.et";
	
	vector m_LightAnglesTarget;
	
	bool m_bIsPanningEnabled = true;

	override protected void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
	
		m_wRoot = w;
		m_wRenderTarget = RenderTargetWidget.Cast(w.FindAnyWidget("ItemPreview"));
		
		m_pInputManager = GetGame().GetInputManager();
		if (!m_pInputManager)
			return;
	};
	
	void SetPanningEnabled(bool enabled) {
		m_bIsPanningEnabled = enabled;
	}
	
	protected void DeletePreview()
	{
		if (m_PreviewSharedItemRef)
		{
			m_PreviewCamera = -1;
			m_PreviewEntity = null;
			m_PreviewWorld = null;
			m_PreviewSharedItemRef = null;
		}
	}

	void Init(IEntity characterEntity) {
		m_characterItemComponent = InventoryItemComponent.Cast(characterEntity.FindComponent(InventoryItemComponent));
	}
	
	// edited storage and edited slot
	// void SetPreviewedEntity(Bacon_GunBuilderUI_Mode editorMode, IEntity storageEntity, InventoryStorageSlot editedSlot, bool shouldResetCamera = true) {
	void SetPreviewedEntity(Bacon_GunBuilderUI_Mode editorMode, IEntity storageEntity, Bacon_GunBuilderUI_SlotInfo editedSlot, bool shouldResetCamera = true) {
		if (!m_PreviewSharedItemRef)
			CreatePreview();
		
		m_storageEntity = storageEntity;
		
		if (storageEntity) {
			m_previewedStorageItemComponent = InventoryItemComponent.Cast(storageEntity.FindComponent(InventoryItemComponent));
			if (!m_previewedStorageItemComponent) {
				Print(string.Format("Entity %1 has no Inventory Item Component!", storageEntity), LogLevel.ERROR);
				return;
			}
		} else {
			m_previewedStorageItemComponent = null;
		}
		
		// m_previewedSlot = editedSlot;
		// changed slot?
		if (m_previewedSlotInfo != editedSlot) {
			m_bDisableCameraAutoFocus = false;
		}

		m_previewedSlotInfo = editedSlot;
		
		// bool shouldResetCamera = false;
		if (editorMode != m_previewMode) {
			m_bDisableCameraAutoFocus = false;
			m_previewMode = editorMode;
			shouldResetCamera = true;
		}

		if (editorMode == Bacon_GunBuilderUI_Mode.CHARACTER) {
			m_previewedItemComponent = m_characterItemComponent;
		}
		
		if (editorMode == Bacon_GunBuilderUI_Mode.ENTITY) {
			m_previewedItemComponent = m_previewedStorageItemComponent;
		}
		
		if (!m_previewedItemComponent)
			return;
		
		UpdatePreview();
		UpdateCameraTargetsB(shouldResetCamera);
	}

	protected void CreatePreview()
	{
		DeletePreview();
		
		m_PreviewSharedItemRef = BaseWorld.CreateWorld("GunBuilder", "GunBuilder");
		m_PreviewWorld = m_PreviewSharedItemRef.GetRef();
		m_PreviewCamera = 0;
		
		m_PreviewWorld.SetCameraType(m_PreviewCamera, CameraType.PERSPECTIVE);
		m_PreviewWorld.SetCameraFarPlane(m_PreviewCamera, 150);
		m_PreviewWorld.SetCameraNearPlane(m_PreviewCamera, 0.001);
		m_PreviewWorld.SetCameraVerticalFOV(m_PreviewCamera, 40);
		
		vector mat[4];
		mat[3] = m_targetCameraPosition;
		SCR_Math3D.LookAt(m_targetCameraPosition, m_targetCameraLookPosition, vector.Up, mat);
		
		m_PreviewWorld.SetCameraEx(m_PreviewCamera, mat);

		Resource rsc = Resource.Load(m_PreviewWorldResourceName);
		if (rsc.IsValid())
			GetGame().SpawnEntityPrefab(rsc, m_PreviewWorld);
		
		rsc = Resource.Load(m_PreviewWorldLightsResourceName);
		if (rsc.IsValid())
			m_LightEntities = GetGame().SpawnEntityPrefab(rsc, m_PreviewWorld);
		
		m_LightAnglesTarget = m_LightEntities.GetAngles(); 

		m_wRenderTarget.SetWorld(m_PreviewWorld, m_PreviewCamera);
	}
	
	// using this to offset the items slightly
	vector GetScreenSizeOffset() {
		float width, height;
        GetGame().GetWorkspace().GetScreenSize(width, height);
		
		float screenOffset = (height / (width - 400)) * m_previewedEntityDefaultDistance;
		
		return (vector.Forward - vector.Right) * (screenOffset * -0.25);
	}
	
	void SetTargetCameraPosition(vector newPos) {
		if (m_cameraTargetChangeAllowed)
			m_targetCameraPosition = newPos; // * m_targetCameraPositionOffsets;
		// m_targetCameraPosition = newPos;
	}
	
	void UpdateCameraTargetsB(bool shouldResetCamera) {
		vector camLookPos, camDirection;
		float camTargetDistance = m_previewedEntitySize * 1.5;

		if (!m_bDisableCameraAutoFocus && GetCameraTargetsForStorageType(m_storageEntity, m_previewedSlotInfo, camLookPos, camDirection, camTargetDistance)) {
			m_cameraTargetDirection = camDirection.Normalized();
			m_targetCameraLookPosition = camLookPos;
			m_cameraTargetDistance = camTargetDistance;
		}
		
		if (!m_bDisableCameraAutoFocus && GetCameraTargetsForSlot(m_storageEntity, m_previewedSlotInfo, camLookPos, camDirection, camTargetDistance)) {
			m_cameraTargetDirection = camDirection.Normalized();
			m_targetCameraLookPosition = camLookPos;
			m_cameraTargetDistance = camTargetDistance;
		}
	}
	
	bool GetCameraTargetsForStorageType(IEntity editedEntity, Bacon_GunBuilderUI_SlotInfo slotInfo, out vector targetCameraLookPosition, out vector targetCameraDirection, out float cameraTargetDistance) {
		if (!editedEntity)
			return false;

		switch (m_previewMode) {
			case Bacon_GunBuilderUI_Mode.CHARACTER: {
				// no slot selected
				if (!slotInfo) {
					cameraTargetDistance = m_previewedEntitySize * 1.5;
					targetCameraLookPosition = m_previewedEntityCenterWorld;
					targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+(vector.Forward + vector.Right));
					return true;
				}
				return false;
			}
			case Bacon_GunBuilderUI_Mode.ENTITY: {
				cameraTargetDistance = m_previewedEntitySize * 1.25;
				targetCameraLookPosition = m_previewedEntityCenterWorld;
				
				// retain camera left/right pos
				if (m_currentCameraMatrix[3][0] < -0.1)
					targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition-vector.Right);
				else
					targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+vector.Right);
				
				return true;
			}
		}
		
		return false;
	}
	
	bool GetCameraTargetsForSlot(IEntity editedEntity, Bacon_GunBuilderUI_SlotInfo slotInfo, out vector targetCameraLookPosition, out vector targetCameraDirection, out float cameraTargetDistance) {
		if (!editedEntity)
			return false;

		IEntity attachedEntity;
		if (slotInfo)
			attachedEntity = slotInfo.slot.GetAttachedEntity();
		
		// character slots do not have position information
		// so default to viewing entire character
		if (m_previewMode == Bacon_GunBuilderUI_Mode.CHARACTER) {
			if (!slotInfo)
				return false;
			
			if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON) {
				targetCameraLookPosition = m_previewedEntityCenterWorld;

				if (Bacon_GunBuilderUI_Helpers.GetWeaponTypeStringFromWeaponSlot(BaseWeaponComponent.Cast(slotInfo.slot.GetParentContainer())) == "primary")
					targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition-(vector.Forward + vector.Right));
				else
					targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+(vector.Forward + vector.Right));
				
				return true;
			}
			
			if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_EQUIPMENT) {
				targetCameraLookPosition = m_previewedEntityCenterWorld;
				targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+(vector.Forward + vector.Right));
				return true;
			}

			if (!attachedEntity) {
				return false;
			}

			vector mins, maxs;
			attachedEntity.GetBounds(mins, maxs);
			
			if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT) {
				targetCameraLookPosition = m_previewedEntityCenterWorld;
				targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+(vector.Forward + vector.Right));

				cameraTargetDistance = Math.Clamp(vector.Distance(mins, maxs) * 1.5, 1, 2);
				typename areaType;
				
				if (!Bacon_GunBuilderUI_Helpers.GetLoadoutAreaType(slotInfo.slot, areaType))
					return true;
				
				// lets not change look position if it's at the feet for some reason
				vector center = vector.Lerp(mins, maxs, 0.5);
				if (areaType == LoadoutBootsArea) {
					targetCameraLookPosition = center;
				} else {
					if (vector.Distance(vector.Zero, center) > 0.2)
						targetCameraLookPosition = center;
					
					if (areaType == LoadoutBackpackArea) {
						targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition-(vector.Forward + vector.Right));
					} else {
						// test
						float size = vector.Distance(mins, maxs);
						vector offsets = {1,0,1};
						
						if (center[2] < -size)
							offsets[2] = -1;
						
						if (center[0] < -size)
							offsets[0] = -1;
						
						targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+offsets);
					}
				}
				
				return true;
			}
			
			targetCameraLookPosition = vector.Lerp(mins, maxs, 0.5);
			targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition+(vector.Forward + vector.Right));

			return true;
		}
		
		if (m_previewMode == Bacon_GunBuilderUI_Mode.ENTITY) {
			// no slot, abort
			if (!slotInfo)
				return false;

			vector slotMat[4];
			slotInfo.slot.GetWorldTransform(slotMat);
			
			targetCameraLookPosition = m_PreviewEntity.GetOrigin() + editedEntity.CoordToLocal(slotMat[3]);
			// targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition);
			
			ApplyOffsetsFromAttachmentMatrix(editedEntity, slotMat, targetCameraLookPosition, targetCameraDirection);
			NudgeByRelativePoint(m_PreviewEntity.GetOrigin(), targetCameraLookPosition, targetCameraDirection);

			if (!attachedEntity) {
				cameraTargetDistance = 0.5;
				return true;
			}
			
			vector mins, maxs;
			attachedEntity.GetBounds(mins, maxs);
			
			targetCameraLookPosition = m_PreviewEntity.GetOrigin() + editedEntity.CoordToLocal(slotMat[3]) + vector.Lerp(mins, maxs, 0.5);
			// targetCameraDirection = 
			
			ApplyOffsetsFromAttachmentMatrix(editedEntity, slotMat, targetCameraLookPosition, targetCameraDirection);
			NudgeByRelativePoint(m_PreviewEntity.GetOrigin(), targetCameraLookPosition, targetCameraDirection);

			cameraTargetDistance = Math.Clamp(vector.Distance(mins, maxs)*1.5, 0.4, 1);
			
			return true;
		}
		
		return false;
	}
	
	void ApplyOffsetsFromAttachmentMatrix(IEntity parent, vector matrix[4], vector targetCameraLookPosition, out vector targetCameraDirection) {
		vector attachmentSlotUp = parent.VectorToLocal(matrix[1]);
			
		float dotUpCameraSlot = vector.Dot(vector.Direction(targetCameraLookPosition, m_currentCameraMatrix[3]), attachmentSlotUp);

		// facing away
		if (dotUpCameraSlot < -0.6) {
			// tell the camera to move to the other direction
			if (m_currentCameraMatrix[3][0] < -0.1) {
				targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition + vector.Right);
				return;
			}
			
			if (m_currentCameraMatrix[3][0] > 0.1) {
				targetCameraDirection = vector.Direction(targetCameraLookPosition, targetCameraLookPosition - vector.Right);
				return;
			}
		}
	}
	
	// add a little forward or back offset based on whether the attachment is forward or back
	void NudgeByRelativePoint(vector origin, vector lookPos, out vector targetCameraDirection) {
		vector diff = origin - lookPos;
		
		if (diff[2] < -0.1) {
			targetCameraDirection[2] = targetCameraDirection[2] + 0.2;
		} else {
			targetCameraDirection[2] = targetCameraDirection[2] - 0.2;
		}
	}

	void UpdatePreview() {
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);

		m_PreviewEntity = GenericEntity.Cast(m_previewedItemComponent.CreatePreviewEntity(m_PreviewWorld, m_PreviewCamera));
		SetEntityQuality(m_PreviewEntity);
		
		if (m_previewMode == Bacon_GunBuilderUI_Mode.ENTITY) {
			m_PreviewEntity.SetOrigin(vector.Up);
			m_PreviewEntity.Update();
		}

		vector mins, maxs;
		m_PreviewEntity.GetWorldBounds(mins, maxs);

		m_previewedEntityCenterWorld = vector.Lerp(mins, maxs, 0.5);
		m_previewedEntityDefaultDistance = vector.Distance(mins, maxs);
		m_previewedEntitySize = vector.Distance(mins, maxs);
		
		IEntity child = m_PreviewEntity.GetChildren();
		while (child)
		{
			SetEntityQuality(child);

			child = child.GetSibling();
		}
	}
	
	void SetEntityQuality(IEntity ent) {
		VObject vObj = ent.GetVObject();
		ent.SetVComponentFlags(VCFlags.NOFILTER & VCFlags.NOLIGHT);
		ent.SetFixedLOD(0);
	}
	
	void Update(float timeSlice) {
#ifdef WORKBENCH
		HandleDebugUi();
#endif
		HandlePanningInputs(timeSlice);
		
		if (!m_PreviewSharedItemRef)
			return;
		
		m_PreviewWorld.GetCamera(m_PreviewCamera, m_currentCameraMatrix);
		
		vector targetCamPosByDistance = m_targetCameraLookPosition + (m_cameraTargetDirection * m_cameraTargetDistance);
		
		m_cameraDistanceToTarget = vector.Distance(m_currentCameraMatrix[3], targetCamPosByDistance);

		if (m_cameraDistanceToTarget > 0.025) {
			m_currentCameraMatrix[3] = m_currentCameraMatrix[3] + ((targetCamPosByDistance - m_currentCameraMatrix[3]) * ((m_cameraDistanceToTarget*2) * timeSlice * 2));
		}
		
		m_cameraLookDistanceToTarget = vector.Distance(m_currentCameraLookPosition, m_targetCameraLookPosition);
		if (m_cameraLookDistanceToTarget > 0.025) {
			m_currentCameraLookPosition = m_currentCameraLookPosition + ((m_targetCameraLookPosition - m_currentCameraLookPosition) * (m_cameraLookDistanceToTarget * timeSlice * 5));
		}

		SCR_Math3D.LookAt(m_currentCameraMatrix[3], m_currentCameraLookPosition, vector.Up, m_currentCameraMatrix);
		
		m_PreviewWorld.SetCameraEx(m_PreviewCamera, m_currentCameraMatrix);
		
		vector camAngles = Math3D.MatrixToAngles(m_currentCameraMatrix);
		
		vector test = m_LightEntities.GetAngles();
		test[1] = camAngles[0];

		m_LightEntities.SetAngles(test + m_LightAnglesTarget);
		m_LightEntities.Update();
	}
	
	void HandlePanningInputs(float timeSlice) {
		if (!m_bIsPanningEnabled)
			return;

		m_ePanMode = GunBuilderUI_PanMode.NONE;

		EInputDeviceType device = m_pInputManager.GetLastUsedInputDevice();
		switch (device) {
			case EInputDeviceType.MOUSE: {
				if (!m_bIsFocused)
					return;
				
				if (m_pInputManager.GetActionValue("Inventory_Drag") > 0.1) {
					m_ePanMode = GunBuilderUI_PanMode.CAMERA;
					break;
				}
				
				if (m_pInputManager.GetActionValue("Panning_ModifierA") > 0.1)
					m_ePanMode = GunBuilderUI_PanMode.LIGHTS;
				
				break;
			}
			case EInputDeviceType.GAMEPAD: {
				m_ePanMode = GunBuilderUI_PanMode.CAMERA;
				
				if (m_pInputManager.GetActionValue("Panning_ModifierA") > 0.1)
					m_ePanMode = GunBuilderUI_PanMode.LIGHTS;
				
				break;
			}
		}
		
		if (m_ePanMode == GunBuilderUI_PanMode.NONE)
			return;

		float panX = m_pInputManager.GetActionValue("Inventory_InspectPanX");
		float panY = m_pInputManager.GetActionValue("Inventory_InspectPanY");
		
//		float panX = m_pInputManager.GetActionValue("InventoryCursorX");
//		float panY = m_pInputManager.GetActionValue("InventoryCursorY");
		
		if (device == EInputDeviceType.GAMEPAD) {
			if ((Math.AbsFloat(panX)+Math.AbsFloat(panY)) < 0.03)
				return;

			panX = panX * 5;
			panY = panY * -5;
		} else {
			panX = panX * 1.5;
			panY = panY * 1.5;
		}
		
		m_bDisableCameraAutoFocus = true;
		PanCamera(panX, panY);
	}
	
	void PanCamera(float inputX, float inputY) {
		if (!m_PreviewWorld)
			return;
		
		vector axis = {inputX, inputY, 0};
		vector newMat[4];
		
		// move lights instead of camera
		if (m_ePanMode == GunBuilderUI_PanMode.LIGHTS) {
			m_LightAnglesTarget[1] = m_LightAnglesTarget[1] + inputX;
			// m_LightEntities.SetAngles(m_LightEntities.GetAngles() + (-vector.Up * inputX));
			// m_LightEntities.Update();
			return;
		}
		
		if (((Math.AbsFloat(inputY)+Math.AbsFloat(inputX))*0.01) > 0.05)
			m_cameraTargetChangeAllowed = false;

		newMat = m_currentCameraMatrix;
		newMat[3] = m_targetCameraLookPosition + (m_cameraTargetDirection * m_cameraTargetDistance);

		SCR_Math3D.RotateAround(newMat, m_targetCameraLookPosition, m_currentCameraMatrix[1], inputX*0.01, newMat);
		m_cameraTargetDirection = vector.Direction(m_targetCameraLookPosition, newMat[3]).Normalized();

		SCR_Math3D.RotateAround(newMat, m_targetCameraLookPosition, m_currentCameraMatrix[0], inputY*0.01, newMat);

		vector angles = Math3D.MatrixToAngles(newMat);

		if (inputY > 0 && angles[1] < -60)
			return;
		
		if (inputY < 0 && angles[1] > 45)
			return;

		m_cameraTargetDirection = vector.Direction(m_targetCameraLookPosition, newMat[3]).Normalized();
	}
	
	override bool OnFocus(Widget w, int x, int y) {
		m_bIsFocused = true;
		
		return super.OnFocus(w,x,y);
	}
	
	override bool OnFocusLost(Widget w, int x, int y) {
		m_bIsFocused = false;
		
		return super.OnFocusLost(w,x,y);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y) {
		m_bIsFocused = true;
		
		return super.OnMouseEnter(w,x,y);
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y) {
		m_bIsFocused = false;
		
		return super.OnMouseLeave(w,enterW,x,y);
	}
	
	void HandleDebugUi() {
		DbgUI.Begin("Preview UI Component");

		DbgUI.Text(string.Format("m_bIsFocused: %1", m_bIsFocused));
		DbgUI.Text(string.Format("m_previewedEntityCenterWorld: %1", m_previewedEntityCenterWorld));
		DbgUI.Text(string.Format("m_previewedEntityDefaultDistance: %1", m_previewedEntityDefaultDistance));
		
		
		DbgUI.Text("Entities and modes");
		DbgUI.Text(string.Format("m_previewMode: %1", m_previewMode));
		DbgUI.Text(string.Format("m_PreviewEntity: %1", m_PreviewEntity.Type()));
		
		DbgUI.Text(string.Format("m_previewedSlotInfo: %1", m_previewedSlotInfo));
		if (m_previewedSlotInfo) {
			vector mat[4];
			m_previewedSlotInfo.slot.GetWorldTransform(mat);
			
			
			DbgUI.Text(string.Format("m_previewedSlotInfo.slot: %1", m_previewedSlotInfo.slot));
			DbgUI.Text(string.Format("m_previewedSlotInfo.slotType: %1", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_SlotType, m_previewedSlotInfo.slotType)));
			DbgUI.Text(string.Format("m_previewedSlotInfo.storageType: %1", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_StorageType, m_previewedSlotInfo.storageType)));
			
			DbgUI.Text(string.Format("- slot world position: %1", mat[3]));
			if (m_previewedSlotInfo.slot.GetAttachedEntity()) {
				DbgUI.Text(string.Format("- slot entity type: %1", m_previewedSlotInfo.slot.GetAttachedEntity().Type()));
				DbgUI.Text(string.Format("- slot entity origin: %1", m_previewedSlotInfo.slot.GetAttachedEntity().GetOrigin()));
				
				vector mins, maxs;
				m_previewedSlotInfo.slot.GetAttachedEntity().GetBounds(mins, maxs);
				
				DbgUI.Text(string.Format("- slot entity center: %1", vector.Lerp(mins, maxs, 0.5)));
				DbgUI.Text(string.Format("- slot entity size: %1", vector.Distance(mins, maxs)));
				DbgUI.Text(string.Format("- slot entity distance from zero: %1", vector.Distance(vector.Zero, vector.Lerp(mins, maxs, 0.5))));
				
			}
		}
		
		DbgUI.Text("Preview World");
		DbgUI.Text(string.Format("m_LightEntities: %1", m_LightEntities));
		
		if (m_LightEntities) {
			DbgUI.Text(string.Format("- angles: %1", m_LightEntities.GetAngles()));
		}
		
		
		
		DbgUI.Text(string.Format("m_storageEntity: %1", m_storageEntity));
		if (m_storageEntity)
			DbgUI.Text(string.Format("- origin: %1", m_storageEntity.GetOrigin()));
		
		DbgUI.Text("Camera");
		DbgUI.Text(string.Format("m_targetCameraLookPosition: %1", m_targetCameraLookPosition));
		DbgUI.Text(string.Format("m_targetCameraPosition: %1", m_targetCameraPosition));
		DbgUI.Text(string.Format("Camera Angles: %1", Math3D.MatrixToAngles(m_currentCameraMatrix)));
		DbgUI.Text(string.Format("Camera Position: %1", m_currentCameraMatrix[3]));
		DbgUI.Text(string.Format("Camera Target Change Allowed: %1", m_cameraTargetChangeAllowed));
		DbgUI.Text(string.Format("m_cameraTargetDistance: %1", m_cameraTargetDistance));
		DbgUI.Text(string.Format("m_cameraTargetDirection: %1", m_cameraTargetDirection));
		
		DbgUI.End();
	}
}

class Bacon_TestClass: ScriptComponentClass {}
class Bacon_Test: ScriptComponent {
	
}