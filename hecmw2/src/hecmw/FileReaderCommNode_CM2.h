/* 
 * File:   FileReaderCommNode_CM2.h
 * Author: ktakeda
 *
 * "CommMesh2"用途のCommNode読み込み用
 *
 * Created on 2010/03/12, 14:50
 */
#include "FileReader.h"
#include "FileReaderBinCheck.h" //入力ファイルとシステムのエンディアン相違判定

namespace FileIO{
#ifndef _FILEREADERCOMMNODE_CM2_H
#define	_FILEREADERCOMMNODE_CM2_H
class CFileReaderCommNodeCM2:public CFileReader{
public:
    CFileReaderCommNodeCM2();
    virtual ~CFileReaderCommNodeCM2();
    
public:
    virtual bool Read(ifstream& ifs, string& sLine);
    virtual bool Read_bin(ifstream& ifs);
};
#endif	/* _FILEREADERCOMMNODE_CM2_H */
}








