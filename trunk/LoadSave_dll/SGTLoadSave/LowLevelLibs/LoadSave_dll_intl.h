
namespace LoadSave
{

enum ESaveFileLevel
{
	ATOM,//the object is an atom.. just wait for the close
	NO_ATOM,//a simple sub-object
	ATOM_ARRAY,//an atom-array (n-dim)
	NO_ATOM_ARRAY,//a no-atom (ie object-) array
	CHUNK,//used by load to indicate that a chunk has been entered.
};

class CSaveFileLevel
{
public:
	ESaveFileLevel level_desc;
	std::vector<int> levelSizes;//first is current level
	int iCurrLevelPos;
	int iArrayLevel;
	bool bArrayEntryClosed;
	std::string strArrayType;
};


class CLoadFileLevel
{
public:
	ESaveFileLevel level_desc;
	std::vector<int> levelSizes;//first is current level
	std::vector<int> levelPos;//first is current level
	int iID;
	int iTypeMod;
	int iRecordID;
};

static int GetAtomSize(int** paiAtomSizes, int iID)//if 0 it was no atom
{
	for(int i=0; paiAtomSizes[i]!=NULL; i++)
	{
		if(iID==paiAtomSizes[i][0])
			return paiAtomSizes[i][1];
	}
	return 0;
}

};