/**
 * This file is a part of client manager.
 * It has been divided but shouldn't be used anywhere else.
 */

public:
	void sendRawCommand(const UserPtr& user, const string& aRaw, bool checkProtection = false);
	void setListLength(const UserPtr& p, const string& listLen);
	void fileListDisconnected(const UserPtr& p);
	void connectionTimeout(const UserPtr& p);
	void checkCheating(const UserPtr& p, DirectoryListing* dl);
	void setCheating(const UserPtr& p, const string& _ccResponse, const string& _cheatString, int _actionId, bool _displayCheat,
		bool _badClient, bool _badFileList, bool _clientCheckComplete, bool _fileListCheckComplete);	void setPkLock(const UserPtr& p, const string& aPk, const string& aLock);
	void setSupports(const UserPtr& p, const string& aSupports);
	void setGenerator(const UserPtr& p, const string& aGenerator, const string& aCID, const string& aBase);
	void setUnknownCommand(const UserPtr& p, const string& aUnknownCommand);
	void reportUser(const HintedUser& user);

	//RSX++
	void setListSize(const UserPtr& p, int64_t aFileLength, bool adc);
	void addCheckToQueue(const UserPtr& p, bool filelist);
	void cleanUser(const HintedUser& user);
	bool getSharingHub(const HintedUser& user);
	void checkSlots(const HintedUser& user, int slots);
	void sendAction(const UserPtr& p, const int aAction);
	void sendAction(OnlineUser& ou, const int aAction);
	void kickFromAutosearch(const UserPtr& p, int action, const string& cheat, const string& file, const string& size, const string& tth, bool display = false);
	void multiHubKick(const UserPtr& p, const string& aRaw);
	//END
private:
	bool compareUsers(const OnlineUser& ou1, const OnlineUser& ou2) const;
public: //file included in public block, so don't break it