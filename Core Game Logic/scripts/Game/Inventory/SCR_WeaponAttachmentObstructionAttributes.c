class SCR_WeaponAttachmentObstructionAttributes : WeaponAttachmentAttributes
{
	[Attribute(desc: "Types of attachments that cannot be mounted at the same time as this one")]
	protected ref array<ref BaseAttachmentType> m_aObstructedAttachmentTypes;
	
	[Attribute(desc: "Types of attachments required for this one to be attachable e.g. M9 Bayonet requires A2 flash hider for mounting")]
	protected ref array<ref BaseAttachmentType> m_aRequiredAttachmentTypes;
	
	//------------------------------------------------------------------------------------------------
	array<typename> GetObstructedAttachmentTypes()
	{
		array<typename> types = {};
		foreach (BaseAttachmentType obstructedType : m_aObstructedAttachmentTypes)
		{
			types.Insert(obstructedType.Type());
		}
		
		return types;
	}
	
	//------------------------------------------------------------------------------------------------
	array<typename> GetRequiredAttachmentTypes()
	{
		array<typename> types = {};
		foreach (BaseAttachmentType requiredType : m_aRequiredAttachmentTypes)
		{
			types.Insert(requiredType.Type());
		}
		
		return types;
	}
}