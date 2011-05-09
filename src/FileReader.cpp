//
//	FileReader.cpp
//
//			2008.12.09
//			2008.12.09
//			k.Takeda

#include "FileReader.h"
using namespace FileIO;

//
//
CFileReader::CFileReader()
{
    mpLogger = Utility::CLogger::Instance();
}

CFileReader::~CFileReader()
{
}

// pmw::CMeshFactory Reader
//
void CFileReader::setFactory(pmw::CMeshFactory *pFactory)
{
    mpFactory = pFactory;
}

// CFileBlockName, CFileTagName
//
bool CFileReader::TagCheck(string& s_line, const char* ctag)
{
    bool bcheck(false);

    uint i;
    for(i=0; i< s_line.length(); i++){
        if(s_line[i]=='\r') s_line[i]=' ';
        if(s_line[i]==',')  s_line[i]=' ';
    };

    istringstream iss(s_line.c_str());
    string stoken;
    while(iss >> stoken){
        if(stoken==ctag){
            //debug
            //cout << "TagCheck == " << stoken << endl;

            bcheck=true;
            return bcheck;
        }
    };

    return bcheck;
}

// getline()-> string
//
string& CFileReader::getLineSt(ifstream& ifs)
{
    char c_Line[BUFFERLENGTH];

    ifs.getline(c_Line, sizeof(c_Line), '\n');

    msLine.clear();
    msLine = c_Line;

    uint i;
    for(i=0; i< msLine.length(); i++){
        if(msLine[i]=='\r') msLine[i]=' ';
        if(msLine[i]==',')  msLine[i]=' ';
    };

    return msLine;
}