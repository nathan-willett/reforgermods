enum ESaveType
{
	USER		= 1 << 0, ///< Save created by the player
	AUTO		= 1 << 1, ///< Save initiated by the game
	EDITOR		= 1 << 2, ///< Play mode back-up in singleplayer editor
	COMPOSITION	= 1 << 3, ///< Save with only entities which does not require restart
};