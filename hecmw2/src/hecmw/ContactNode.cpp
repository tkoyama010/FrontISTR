//
//  ContactNode.cpp
//
//
//
//              2009.10.26
//              2009.10.26
//              k.Takeda
#include "ContactNode.h"
using namespace pmw;


// construct & destruct
//
CContactNode::CContactNode()
{
    mbMesh=false;
    mbNode=false;
    
    mbSlave=false;
    mbMarkingMFace=false;
}
CContactNode::~CContactNode()
{
    ;
}

// Meshが自身の計算領域データの場合マーキング.
//
void CContactNode::markingSelfMesh()
{
    mbMesh=true;
}

// Nodeが自身の計算領域データの場合マーキング
//
void CContactNode::markingSelfNode()
{
    mbNode=true;
}


//// 節点集合の"SkinFaceID",とFace内部の"ノード配列インデックス番号"をセット
////
//void CContactNode::pushSkinFaceID(const uint& skinFaceID, const uint& nodeIndex)
//{
//    mvAggFaceID.push_back(skinFaceID);
//    mmFaceVertNum[skinFaceID]= nodeIndex;
//}


// EQUATION 関連パラメータ
// ------------
// Arbitrary DOF(任意自由度)
//
// "変位"の配列確保
//
void CContactNode::resizeDisp(const uiint& dof)
{
    mvDisplacement.resize(dof);
}

// 変位の初期化
//
void CContactNode::initDisp()
{
    uiint numOfDOF;
    uiint idof;
    
    numOfDOF= mvDisplacement.size();
    for(idof=0; idof< numOfDOF; idof++){
        mvDisplacement[idof]=0.0;
    };
}

// スカラーパラメータの配列確保(複数のスカラー)
//
void CContactNode::resizeScalar(const uiint& numOfScalar)
{
    mvScalar.resize(numOfScalar);
}

// スカラーの初期化
//
void CContactNode::initScalar()
{
    uiint numOfScalar= mvScalar.size();
    uiint i;

    for(i=0; i< numOfScalar; i++){
        mvScalar[i]= 0.0;
    };
}

// スレーブ点か否かのマーキング
//
void CContactNode::markingSlave()
{
    mbSlave=true;
}

// 新バージョン '10.03.05
// ----
// 各Levelでの,マスター面
// ----
void CContactNode::setMasterFaceID(const uiint& faceID, const uiint& level)
{
    mmMasterFaceID[level]= faceID;
    mvbMasterFaceID[level-mLevel]=true;
}
uiint& CContactNode::getMasterFaceID(const uiint& level)
{
    return mmMasterFaceID[level];
}
bool CContactNode::have_MasterFaceID(const uiint& level)
{
    // 節点ごとに,自身のLevelから階層構造が始まるので,問い合わせLevelから自身のLevelを引く
    return mvbMasterFaceID[level - mLevel];
}




// 八分木
//
void CContactNode::resizeOctreeID(const uiint& res_size)
{
    mvKnotID.resize(res_size);
}

void CContactNode::setOctreeID(const uiint& layer, const uiint& knot_id)
{
//    //debug
//    cout << "contact_node ,layer= " << layer << ", knot_id= " << knot_id << endl;

    mvKnotID[layer]= knot_id;
}















