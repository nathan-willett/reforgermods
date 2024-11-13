class BaconLoadoutEditor_EventComponentClass: ScriptComponentClass {}
class BaconLoadoutEditor_EventComponent: ScriptComponent {
	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner) {
		SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		
		if (soundComponent)
			soundComponent.SoundEvent("MUSIC");
	}
}