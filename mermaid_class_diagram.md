classDiagram

    class SCR_PlayerControllerClass {
        - m_bRetain3PV: bool
        - m_OnOwnershipChangedInvoker: return
        - isBlockedAuthorPresent: bool
        - aimSensitivityMouse: float
        - aimSensitivityGamepad: float
        - aimMultipADS: float
        - inputCurveMouse: float
        - inputCurveStick: float
        - inputCurveGyro: float
        - stickyADS: bool
        - stickyGadgets: bool
        - mouseControlAircraft: bool
        - gamepadFreelookInAircraft: bool
        - drivingAssistance: EVehicleDrivingAssistanceMode
        - gyroAlways: bool
        - gyroFreelook: bool
        - gyroADS: bool
        - gyroSensitivity: float
        - gyroVerticalHorizontalRatio: float
        - gyroDirectionYaw: float
        - gyroDirectionPitch: float
        - gyroDirectionRoll: float
        - focusInADS: float
        - focusInPIP: float
        - m_bIsPossessing: return
        - m_MainEntity: return
        - 0: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - 0: return
        - focus: float
        - inputAnalogue: float
        - compartment: BaseCompartmentSlot
        - turretController: TurretControllerComponent
        - input: float
        - focus: return
        - m_bGadgetFocus: return
        - false: return
        - true: return
        - true: return
        - true: return
        - true: return
        - icon: string
        - icon: string
        + OnControlledEntityChangedPlayerController(IEntity from, IEntity to) void
        + OwnershipChangedDelegate(bool isChanging, bool becameOwner) void
        + OnDestroyedPlayerController(Instigator killer, IEntity killerEntity) void
        + OnPossessed(IEntity entity) void
        + OnBeforePossessed(IEntity entity) void
        + GetOnOwnershipChangedInvoker() OnOwnershipChangedInvoker
        + OnPlayerRegistered(SCR_EditorManagerEntity managerEntity) void
        + OnBlockedListUpdated(bool success) void
        + OnAuthorsRequestFinished(set<SCR_EditableEntityAuthor> activeUGCAuthors) void
        + OnPlayerNameCacheUpdate(bool success) void
        + DisconnectFromGame() void
        + OnBlockedPlayerJoined(int playerID) void
        + SetPossessedEntity(IEntity entity) void
        + SetInitialMainEntity(notnull IEntity entity) void
        + IsPossessing() bool
        + GetMainEntity() IEntity
        + GetControlledEntity() return
        + GetFocusValue(float adsProgress = 0, float dt = -1) float
        + SetGadgetFocus(bool gadgetFocus) void
        + GetGadgetFocus() bool
        + ActionFocusToggle(float value = 0.0, EActionTrigger reason = 0) void
        + ActionFocusToggleVehicle(float value = 0.0, EActionTrigger reason = 0) void
        + ActionFocusToggleUnarmed(float value = 0.0, EActionTrigger reason = 0) void
        + OnWalk() void
        + OnEndWalk() void
        + ActionOpenInventory() void
        + ActionGesturePing(float value = 0.0, EActionTrigger reason = 0) void
        + ActionGesturePingHold(float value = 0.0, EActionTrigger reason = 0) void
        + SetPlatformImageTo(int playerID, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false) bool
        + SetPlatformImageToKind(PlatformKind targetPlatform, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false) bool
    }

    class SCR_PlayerController {
        - m_bRetain3PV: bool
        - m_OnOwnershipChangedInvoker: return
        - isBlockedAuthorPresent: bool
        - aimSensitivityMouse: float
        - aimSensitivityGamepad: float
        - aimMultipADS: float
        - inputCurveMouse: float
        - inputCurveStick: float
        - inputCurveGyro: float
        - stickyADS: bool
        - stickyGadgets: bool
        - mouseControlAircraft: bool
        - gamepadFreelookInAircraft: bool
        - drivingAssistance: EVehicleDrivingAssistanceMode
        - gyroAlways: bool
        - gyroFreelook: bool
        - gyroADS: bool
        - gyroSensitivity: float
        - gyroVerticalHorizontalRatio: float
        - gyroDirectionYaw: float
        - gyroDirectionPitch: float
        - gyroDirectionRoll: float
        - focusInADS: float
        - focusInPIP: float
        - m_bIsPossessing: return
        - m_MainEntity: return
        - 0: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - null: return
        - 0: return
        - focus: float
        - inputAnalogue: float
        - compartment: BaseCompartmentSlot
        - turretController: TurretControllerComponent
        - input: float
        - focus: return
        - m_bGadgetFocus: return
        - false: return
        - true: return
        - true: return
        - true: return
        - true: return
        - icon: string
        - icon: string
        + GetOnOwnershipChangedInvoker() OnOwnershipChangedInvoker
        + OnPlayerRegistered(SCR_EditorManagerEntity managerEntity) void
        + OnBlockedListUpdated(bool success) void
        + OnAuthorsRequestFinished(set<SCR_EditableEntityAuthor> activeUGCAuthors) void
        + OnPlayerNameCacheUpdate(bool success) void
        + DisconnectFromGame() void
        + OnBlockedPlayerJoined(int playerID) void
        + SetPossessedEntity(IEntity entity) void
        + SetInitialMainEntity(notnull IEntity entity) void
        + IsPossessing() bool
        + GetMainEntity() IEntity
        + GetControlledEntity() return
        + GetFocusValue(float adsProgress = 0, float dt = -1) float
        + SetGadgetFocus(bool gadgetFocus) void
        + GetGadgetFocus() bool
        + ActionFocusToggle(float value = 0.0, EActionTrigger reason = 0) void
        + ActionFocusToggleVehicle(float value = 0.0, EActionTrigger reason = 0) void
        + ActionFocusToggleUnarmed(float value = 0.0, EActionTrigger reason = 0) void
        + OnWalk() void
        + OnEndWalk() void
        + ActionOpenInventory() void
        + ActionGesturePing(float value = 0.0, EActionTrigger reason = 0) void
        + ActionGesturePingHold(float value = 0.0, EActionTrigger reason = 0) void
        + SetPlatformImageTo(int playerID, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false) bool
        + SetPlatformImageToKind(PlatformKind targetPlatform, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false) bool
    }

