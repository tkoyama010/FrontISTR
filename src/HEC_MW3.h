//
// HEC_MW3.h
//
// pMW
//  => Main & Interface
// 
// (MeshFactory)
// (GMGModel)
// 
//			2009.05.08
//                      2008.11.19
//			k.Takeda
#ifndef PMW_MAIN_HH_C8955A70_0EE9_4f3e_82D1_FB32C9626513
#define PMW_MAIN_HH_C8955A70_0EE9_4f3e_82D1_FB32C9626513

#include "CommonStd.h"
#include "TypeDef.h"

//
// MPI include
// 

#include "FileIO.h"
#include "Logger.h"

#include "MeshFactory.h"
#include "GMGModel.h"


namespace pmw{
class CMWMain
{
// Singleton Constructor
//
public:
    static CMWMain* Instance(){
	static CMWMain pmwMain;
	return &pmwMain;
    }
private:
    CMWMain(void);
public:
    //CMWMain(void);
    virtual ~CMWMain(void);

// 
//
protected:
    FileIO::CFileIO   *mpFileIO;
    Utility::CLogger  *mpLogger;    

    pmw::CGMGModel    *mpGMGModel;
    pmw::CMeshFactory  *mpFactory;

    bool mb_file;//file読み込み後？

public:
    int FileRead(string& file_name); // マルチグリッド数のAssyModel生成、"Level=0"のMesh(アッセンブリーパーツ)生成
    int FileWrite(string& file_name);// ファイル出力

    int Initialize();// MPI, Logger設定
    int Finalize();  // Loggerのログファイルclose
    
    
    int Refine();//ファイル指定のマルチグリッドMeshデータ生成
};
}

#endif