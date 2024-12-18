class SCR_ServerSaveRequestCallback: SCR_BackendCallback
{
	static const string SESSION_SAVE_NAME = "SCR_SaveFileManager_SessionSave";
	
	protected string m_sFileName;
	protected ref SCR_UploadSaveCallback_PageParams m_PageParams;
	protected ref SCR_BackendCallback m_UploadCallback;
	
	protected bool m_bHasData = false;
	protected string m_sId;
	
	//----------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);	
		Print(string.Format("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", code), LogLevel.NORMAL);
		
		if (code == 0)
			return;
		
		// Get saves 
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop(); 
		
		array<WorkshopItem> items = {};
		workshop.GetPageItems(items);
		
		m_bHasData = code == EBackendRequest.EBREQ_WORKSHOP_GetAsset;
		
		WorldSaveItem save;
		WorldSaveManifest manifest = new WorldSaveManifest();
		
		bool isNew = items.IsEmpty();
		if (!isNew)
		{
			// Clear  
			save = WorldSaveItem.Cast(items[0]);
			
			if (m_bHasData)
			{
				save.FillManifest(manifest);
			}
			else
			{
				save.AskDetail(this);
				return;
			}		
		}
		
		//--- Define manifest params
		manifest.m_sName = SESSION_SAVE_NAME;
		manifest.m_sSummary = m_sFileName;
		manifest.m_aFileNames = {m_sFileName};
		manifest.m_bUnlisted = true;
		
		//--- Create new save from manifest
		if (isNew)
		{
			save = WorldSaveItem.CreateLocalWorldSave(manifest);
		}
		
		//--- Upload file
		m_sId = save.Id();
		m_UploadCallback = new SCR_BackendCallback();
		m_UploadCallback.GetEventOnResponse().Insert(OnUploadResponse);
		
		save.UploadWorldSave(manifest, m_UploadCallback);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{
		super.OnError(code, restCode, apiCode);
		Print(string.Format("[SCR_ServerSaveRequestCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code)), LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		super.OnTimeout();
		Print("[SCR_ServerSaveRequestCallback] OnTimeout", LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnUploadResponse(SCR_BackendCallback callback)
	{
		switch (callback.GetResponseType())
		{
			case EBackendCallbackResponse.SUCCESS:
			{
				Print(string.Format("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", callback.GetCode()), LogLevel.NORMAL);
		
				BaseChatComponent chatComponent = BaseChatComponent.Cast(GetGame().GetPlayerController().FindComponent(BaseChatComponent));
				if (chatComponent)
					chatComponent.SendMessage(string.Format("#load %1", m_sId), 0);
				
				break;
			}
			
			case EBackendCallbackResponse.ERROR:
			{
				Print(string.Format("[SCR_ServerSaveUploadCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", callback.GetCode(), callback.GetRestCode(), callback.GetApiCode(), GetGame().GetBackendApi().GetErrorCode(callback.GetCode())), LogLevel.NORMAL);
				break;
			}
			
			case EBackendCallbackResponse.TIMEOUT:
			{
				Print("[SCR_ServerSaveUploadCallback] OnTimeout", LogLevel.NORMAL);
				break;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_ServerSaveRequestCallback(string fileName)
	{
		m_sFileName = fileName;
		
		m_PageParams = new SCR_UploadSaveCallback_PageParams();
		m_PageParams.limit = 1;
		m_PageParams.type = "world-save";
		GetGame().GetBackendApi().GetWorkshop().RequestPage(this, m_PageParams, false);
	}
};

//----------------------------------------------------------------------------------------
class SCR_UploadSaveCallback_PageParams: PageParams
{
	override void OnPack()
	{
		StoreBoolean("owned", true);
		StoreString("search", SCR_ServerSaveRequestCallback.SESSION_SAVE_NAME);
	}
};