//
// Element.h
//
//			2008.12.12
//			2008.11.05
//			k.Takeda
#ifndef ELEMENT_HH_05B90007_6581_437f_AD9D_7C9227A8D7B6
#define ELEMENT_HH_05B90007_6581_437f_AD9D_7C9227A8D7B6


#include "CommonStd.h"
#include "TypeDef.h"
#include <map>
#include <utility> //pair

#include "Node.h"
#include "ElementType.h"
typedef std::pair<pmw::CNode*,pmw::CNode*> PairNode;

#include "EdgeTree.h"    // node0,node1   -> edge
#include "FaceTree.h"    // <node_indexs> -> face
#include "EdgeFaceTree.h"// edge0,edge1   -> face
#include "NodeConnectNodeTree.h"// node   -> node

#include "Logger.h"

namespace pmw{
class CElement 
{
public:
    CElement(void);
    virtual ~CElement(void);

protected:
    uint mnID;// Meshの要素Index番号
    vector<CNode*> mvNode;

    uint mMGLevel;//MultiGrid Level

    vector<vector<CNode*> > mvvFaceCnvNode;//面を構成するノード
    vuint mvPairNodeLocalNum;//ペアノードの局所ノード番号
    PairNode mEdgeNode;

    map<uint, uint, less<uint> > mmIDLocal;//key:グローバルIndex => val:局所番号



    // prolongation準備 (ノード周囲の要素集合は、ノードがIndex番号で所有)
    // ---------------
    // 辺の要素集合、辺の中間ノード
    //
    vector<CNode*> mvEdgeInterNode;// prolongation用の,Edge中間ノード（要素のノード数には含めない)
    vector<vector<CElement*> > mvvEdgeElement;//辺(Edge)に隣接する要素
    vector<bool>   mvb_edge;// Edgeに要素が既にセットされているか?
    // 面に隣接する要素
    // ---
    vector<CElement*> mvFaceElement;// 面に隣接する要素(面-隣接は1個の要素に限定)
    vector<CNode*> mvFaceNode;// 面中心のノード
    vector<bool>   mvb_face;  // Faceにノードが既にセットされているか？
    // 要素中心のノード
    CNode* mpCenterNode;

    // prolongationされて生成された場合の親の要素のIndex(ID)
    uint parentID;


    // Property Check
    virtual bool IndexCheck(const uint& propType, const uint& index, string& method_name)=0;

    // ノードから要素内の局所番号に変換
    vuint& getLocalNodeNum(CNode* pNode0, CNode* pNode1);

    // 局所ノード番号から、面の局所番号に変換
    virtual uint& getLocalFaceNum(const vuint& vLocalNodeNum)=0;
    // ノードから、面番号に変換、public:getFaceIndex に移行.
    // virtual uint& getLocalFaceNum(CNode* pNode0, CNode* pNode1, CNode* pNode2)=0;


public:
    // Element ID
    void setID(const uint& id){ mnID = id;}
    uint& getID(){ return mnID;}

    // Parent_Element ID (再分割した要素の場合の親の要素)
    void setParentID(const uint& id){ parentID= id;}
    uint& getParentID(){ return parentID;}

    // MultiGrid Level
    void setMGLevel(const uint& mgLevel){ mMGLevel= mgLevel;}
    uint& getMGLevel(){ return mMGLevel;}

    // Node*
    void  setNode(CNode* pNode,const uint& local_id);
    vector<CNode*>& getNode() { return mvNode;}
    CNode* getNode(const uint& local_id){ return mvNode[local_id];}


    // Property
    virtual const uint& getType()=0;
    virtual const uint& getNumOfFace()=0;
    virtual const uint& getNumOfEdge()=0;
    virtual const uint& getNumOfNode()=0;
    virtual const uint& getEntityType()=0;


    // prolongation準備
    // ---------------
    // EdgeElement
    virtual PairNode& getPairNode(const uint& edgeIndex)=0;
    virtual void     getPairNode(vint& pairNodeIndex, const uint& edgeIndex)=0;
    void reserveEdgeElement(const uint& edgeIndex, const uint& numOfElem);
    void setEdgeElement(const uint& edgeIndex, CElement* pElem);
    void setEdgeAggElement(const uint& edgeIndex, vector<CElement*> vElement);
    vector<CElement*>& getEdgeElement(const uint& edgeIndex){ return mvvEdgeElement[edgeIndex];}
    // EdgeElement boolスタンプ
    virtual bool isEdgeElem(CNode* pNode0, CNode* pNode1)=0;
    virtual void setBoolEdgeElem(CNode* pNode0, CNode* pNode1)=0;
    // (局所ノード番号,局所ノード番号)に対応した辺の、Index番号
    virtual uint& getEdgeIndex(CNode* pNode0, CNode* pNode1)=0;
    // EdgeNode
    // set Intermediate Node for prolongation
    // (prolongation 辺ノード)
    void setEdgeInterNode(CNode* pNode, const uint& edgeIndex);
    CNode* getEdgeInterNode(const uint& edgeIndex){ return mvEdgeInterNode[edgeIndex];}


    // FaceElement
    //
    void setFaceElement(CElement* pElem, const uint& faceIndex){ mvFaceElement[faceIndex]= pElem;}
    // 局所面番号に対応する、面構成ノード配列を提供.
    //
    vector<CNode*>& getFaceCnvNodes(const uint& faceIndex){ return  mvvFaceCnvNode[faceIndex];}
    // mvvFaceCnvNodeのセットアップ
    virtual void setupFaceCnvNodes()=0;


    //
    // FaceNode
    //
    void setFaceNode(CNode* pNode, const uint& faceIndex){ mvFaceNode[faceIndex]= pNode;}
    CNode* getFaceNode(const uint& faceIndex){ return mvFaceNode[faceIndex];}
    virtual bool isFaceElem(CNode* pNode0, CNode* pNode1, CNode* pNode2);   //Faceにノードがセットされているか？
    virtual void setBoolFaceElem(CNode* pNode0, CNode* pNode1, CNode* pNode2);//Faceノードがセットされたことをスタンプ


    // Edge -> Face_Index
    virtual uint& getFaceIndex(const uint& edge0, const uint& edge1)=0;
    // 3 Nodes -> Face Index
    virtual uint& getFaceIndex(CNode* pNode0, CNode* pNode1, CNode* pNode2)=0;


    // CenterNode(Volume Center)
    //
    void setVolumeNode(CNode* pNode){ mpCenterNode= pNode;}
    CNode* getVolumeNode(){return mpCenterNode;}


    // Node周囲の接続先Nodeを配列で返す.
    // (係数行列 作成用途, CMesh::setupAggregate)
    virtual vector<CNode*> getConnectNode(CNode* pNode)=0;
};
}
#endif