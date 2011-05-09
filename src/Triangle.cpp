
#include "FaceTree.h"

//
//  Triangle.cpp
//                          2009.06.19
//                          2008.12.01
//                          k.Takeda
#include "Triangle.h"
using namespace pmw;

uint CTriangle::mnElemType = ElementType::Triangle;
uint CTriangle::mNumOfFace = 1;
uint CTriangle::mNumOfEdge = 3;
uint CTriangle::mNumOfNode = 3;
//
//
CTriangle::CTriangle()
{
    mvNode.resize(mNumOfNode);
    mvvEdgeElement.resize(mNumOfEdge);

    mvEdgeInterNode.resize(mNumOfEdge);

    mvb_edge.reserve(mNumOfEdge);
    uint i;
    for(i=0; i< mNumOfEdge; i++){
      mvb_edge.push_back(false);
    };

    mvFaceElement.resize(mNumOfFace);
    mvFaceNode.resize(mNumOfFace);

    mvb_face.reserve(mNumOfFace);
    // Face ノード bool
    //
    for(i=0; i< mNumOfFace; i++){
        mvb_face.push_back(false);
    };
    
}

//
//
CTriangle::~CTriangle()
{
//    //debug
//    cout << "~CTriangle" << endl;
}





bool CTriangle::IndexCheck(const uint& propType, const uint& index, string& method_name)
{
    Utility::CLogger *pLogger = Utility::CLogger::Instance();

    uint numOfProp;
    switch(propType){
        case(ElementPropType::Face):
            numOfProp = mNumOfFace;
            break;
        case(ElementPropType::Edge):
            numOfProp = mNumOfEdge;
            break;
        case(ElementPropType::Node):
            numOfProp = mNumOfNode;
            break;
        default:
            numOfProp = 0;
            break;
    }

    if(index >= numOfProp){
        pLogger->Info(Utility::LoggerMode::Error, "CTriangle::"+ method_name);
        return false;
    }else{
        return true;
    }
}



// out:(PariNode edgeNode)
//
PairNode& CTriangle::getPairNode(const uint& edgeIndex)
{
    switch(edgeIndex){
        //surf 0
        case(0):
            mEdgeNode.first=mvNode[0];
            mEdgeNode.second=mvNode[1];
            break;
        case(1):
            mEdgeNode.first=mvNode[1];
            mEdgeNode.second=mvNode[2];
            break;
        case(2):
            mEdgeNode.first=mvNode[2];
            mEdgeNode.second=mvNode[0];
            break;

        default:
            break;
    }

    return mEdgeNode;
}

// out:(vint& pairNodeIndex) -> pair Node index num.
//
void CTriangle::getPairNode(vint& pairNodeIndex, const uint& edgeIndex)
{
    switch(edgeIndex){
        //surf 0 (low surf)
        case(0):
            pairNodeIndex[0]= mvNode[0]->getID();
            pairNodeIndex[1]= mvNode[1]->getID();
            break;
        case(1):
            pairNodeIndex[0]= mvNode[1]->getID();
            pairNodeIndex[1]= mvNode[2]->getID();
            break;
        case(2):
            pairNodeIndex[0]= mvNode[2]->getID();
            pairNodeIndex[1]= mvNode[0]->getID();
            break;

        default:
            break;
    }
}

// ノードの局所番号に対応した辺(Edge)番号
//
uint& CTriangle::getEdgeIndex(CNode* pNode0, CNode* pNode1)
{ 
    getLocalNodeNum(pNode0, pNode1);//mvPairNodeLocalNumに値が入る.

    CEdgeTree *edgeTree = CEdgeTree::Instance();

    return edgeTree->getTriangleEdgeIndex(mvPairNodeLocalNum[0], mvPairNodeLocalNum[1]);//Triangle Edge Tree
}

// EdgeElement集合がセット済みか?
//
bool CTriangle::isEdgeElem(CNode* pNode0, CNode* pNode1)
{
    //    //
    //    //-- getLocalNodeNum(pNode0, pNode1)と同じ --
    //    //
    //    uint index0, index1;//MeshでのノードのIndex番号
    //    uint local0, local1;//局所ノード番号
    //
    //    index0= pNode0->getID();  index1= pNode1->getID();
    //
    //    // pNode0とpNode1のグローバルなIndex番号 => 局所番号に変換
    //    //
    //    uint i;
    //    for(i=0; i< mvNode.size(); i++){
    //        if(index0 == mvNode[i]->getID()) local0 = i;
    //        if(index1 == mvNode[i]->getID()) local1 = i;
    //    };
    //    //--------------------------------------------


    // Edgeに要素集合が作られているか? のbool値を返す
    uint edgeNum;
    edgeNum = getEdgeIndex(pNode0, pNode1);//Triangle Edge Tree

    return mvb_edge[edgeNum];
}

// EdgeElement集合がセットされていることをスタンプ
//
void CTriangle::setBoolEdgeElem(CNode* pNode0, CNode* pNode1)
{
    //vuint vLocalNum = getLocalNodeNum(pNode0, pNode1);

    // Edgeに要素集合が作られているか? のbool値を返す
    uint edgeNum;
    edgeNum = getEdgeIndex(pNode0, pNode1);//Triangle Edge Tree

    mvb_edge[edgeNum]=true;// スタンプ
}

// 局所ノード番号が正しければ"0"を返す.
//
uint& CTriangle::getLocalFaceNum(const vuint& vLocalNodeNum)
{
    CFaceTree *pFaceTree = CFaceTree::Instance();
    uint faceIndex = pFaceTree->getTriangleFaceIndex(vLocalNodeNum);

    if(faceIndex==0){
        mTempo= 0;
    }else{
        Utility::CLogger *pLogger = Utility::CLogger::Instance();
        pLogger->Info(Utility::LoggerMode::Error,"mismatch!, Local Node number@CTriangle::getLocalFaceNum");
        mTempo= 1;
    }
    return mTempo;
}

// 3個のノードから、面番号を取得.
//
uint& CTriangle::getFaceIndex(CNode* pNode0, CNode* pNode1, CNode* pNode2)
{
    // 面構成ノード -> 局所ノード番号
    // 局所ノード番号 -> 面番号==iface
    vuint vLocalNum; vLocalNum.reserve(3);

    //vLocalNum[0]= mmIDLocal[pNode0->getID()];
    //vLocalNum[1]= mmIDLocal[pNode1->getID()];
    //vLocalNum[2]= mmIDLocal[pNode2->getID()];
    vLocalNum[0]= 0;
    vLocalNum[1]= 1;
    vLocalNum[2]= 2;

    CFaceTree *faceTree = CFaceTree::Instance();
    uint faceIndex = faceTree->getTriangleFaceIndex(vLocalNum);

    if(faceIndex==0){
        mTempo= 0;
    }else{
        Utility::CLogger *pLogger= Utility::CLogger::Instance();
        pLogger->Info(Utility::LoggerMode::Error,"mismatch!, Local Node Number @Triangle::getLocalFaceNum(arg:pNode)");
        mTempo= 1;
    }
    return mTempo;
}


// 2 Edge => Face Index
//
uint& CTriangle::getFaceIndex(const uint& edge0, const uint& edge1)
{
    CEdgeFaceTree *pTree= CEdgeFaceTree::Instance();

    return pTree->getTriangleFaceIndex(edge0, edge1);
}

// mvvFaceCnvNodeのセットアップ
//
void CTriangle::setupFaceCnvNodes()
{
    // Face構成のノード・コネクティビティ
    //
    CNode *pNode;
    mvvFaceCnvNode.resize(1);
    mvvFaceCnvNode[0].resize(3);
    uint ivert;

    for(ivert=0; ivert< 3; ivert++){
        pNode= mvNode[ivert];
        mvvFaceCnvNode[0][ivert]=pNode;
    };
}


// Node周囲の接続先Nodeを配列で返す.
// (係数行列 作成用途, CMesh::setupAggregate)
//
vector<CNode*> CTriangle::getConnectNode(CNode* pNode)
{
    vector<CNode*> vNode;
    vNode.reserve(2);

    uint gID= pNode->getID();
    uint localID= mmIDLocal[gID];

    CNodeConnectNodeTree *pConnTree = CNodeConnectNodeTree::Instance();
    vuint vLocalID;
    vLocalID= pConnTree->getTriangleConnectNode(localID);

    uint i;
    for(i=0; i< 2; i++){
        localID= vLocalID[i];
        vNode.push_back(mvNode[localID]);
    };

    return vNode;
}













