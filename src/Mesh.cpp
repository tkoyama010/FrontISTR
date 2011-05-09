
#include "Vertex.h"
#include "Logger.h"
#include "Node.h"


#include <vector>


#include "VectorNode.h"


#include "BoundaryGroup.h"
#include "Element.h"


#include "AggregateElement.h"

//
//				2008.11.10
//				k.Takeda
#include "Mesh.h"
#include "ScalarNode.h"
#include "ScalarVectorNode.h"
using namespace pmw;

CMesh::CMesh(void)
{
    mpLogger = Utility::CLogger::Instance();
    mpLogger->Info(Utility::LoggerMode::MWDebug, "Use! CMesh(const uint& numOfNode, const uint& numOfElem)");

    //debug
    mnDummyCount= 0;
}

CMesh::CMesh(const uint& numofnode, const uint& numofelem)
{
    mpLogger = Utility::CLogger::Instance();

    numOfNode = numofnode;  numOfElement = numofelem;

    mvNode.resize(numOfNode);
    mvElement.resize(numOfElement);
}

// Node, Element, BoundaryNode
//
CMesh::~CMesh(void)
{
    //debug
    cout << "~CMesh start,  mMGLevel==" << mMGLevel << endl;

    // Levelが同一なら↓これでOK.
    // for_each(mvNode.begin(), mvNode.end(), DeleteObject());

    // MGLevelを分けてノードを削除.
    //
    uint i;
    CNode* pNode;
    for(i=0; i< mvNode.size(); i++){
        pNode= mvNode[i];

        if(pNode){
            if(pNode->getMGLevel()==mMGLevel) delete pNode;
        }
        mvNode.erase(mvNode.begin()+i);
    };
    
    
    // 要素(Element)の削除
    //
    //for_each(mvElement.begin(), mvElement.end(), DeleteObject());
    CElement *pElem;
    for(i=0; i< mvElement.size(); i++){
        pElem= mvElement[i];

        if(pElem){
            if(pElem->getMGLevel()==mMGLevel) delete pElem;
        }
        mvElement.erase(mvElement.begin()+i);
    };


    // 要素集合(AggretateElement)の削除
    //
    CAggregateElement *pAggElem;
    for(i=0; i< mvAggElement.size(); i++){
        pAggElem= mvAggElement[i];
        pNode= mvNode[i];// <= MultiGridレベルを調べるため.

        if(pAggElem){
            // NodeのMGLevelによるAggElemの選別
            if(pNode->getMGLevel()==mMGLevel) delete pAggElem;
        }
        mvAggElement.erase(mvAggElement.begin()+i);
    };


    // 境界ノード,面,体の削除
    //
    CBoundaryFace *pBoundFace;
    for(i=0; i< mBoundaryFaces.NumOfBoundary(); i++){
        pBoundFace = mBoundaryFaces.get_withIndex(i);
        delete pBoundFace;
    };

    CBoundaryVolume *pBoundVol;
    for(i=0; i < mBoundaryVolumes.NumOfBoundary(); i++){
        pBoundVol = mBoundaryVolumes.get_withIndex(i);
        delete pBoundVol;
    };

    CBoundaryNode *pBoundNode;
    for(i=0; i < mBoundaryNodes.NumOfBoundary(); i++){
        pBoundNode = mBoundaryNodes.get_withIndex(i);
        delete pBoundNode;
    };


    //debug
    cout << "~CMesh   end,  mMGLevel==" << mMGLevel << endl;
}

// Bucket setup -> Node
//
// case : max_id, min_id 判明
//
void CMesh::initBucketNode(const uint& max_id, const uint& min_id)
{
    moBucket.resizeBucketNode(max_id, min_id);
}
// ID -> Index, setup
void CMesh::setupBucketNodeIndex(const uint& id, const uint& index)
{
    moBucket.setIndexNode(id, index);
}


// case : mvNode setup 済み
//        Max_ID , Min_ID を取得してBucketにセットアップ
//
void CMesh::setupBucketNode()
{
    CNode *pNode;
    uint maxID,minID, i;

    // init
    pNode = mvNode[0];
    maxID = pNode->getID(); minID = pNode->getID();

    // serch Max,Min
    for(i=0; i < mvNode.size(); i++){
        pNode = mvNode[i];
        if(pNode->getID() > maxID) maxID = pNode->getID();
        if(pNode->getID() < minID) minID = pNode->getID();
    };

    moBucket.resizeBucketNode(maxID, minID);

    // set data
    for(i=0; i < mvNode.size(); i++){
        pNode = mvNode[i];
        moBucket.setIndexNode(pNode->getID(), i);
    };
}

// Bucket setup -> Element
//
// case : max_id, min_id 判明
//
void CMesh::initBucketElement(const uint& max_id, const uint& min_id)
{
    moBucket.resizeBucketElement(max_id, min_id);
}
// ID -> Index, setup
void CMesh::setupBucketElementIndex(const uint& id, const uint& index)
{
    moBucket.setIndexElement(id, index);
}


// case : mvElement setup 済み
//        Max_ID , Min_ID を取得してBucketにセットアップ
//
void CMesh::setupBucketElement()
{
    CElement *pElement;
    uint maxID,minID, i;

    // init
    pElement = mvElement[0];
    maxID = pElement->getID(); minID = pElement->getID();

    // serch Max,Min
    for(i=0; i < mvElement.size(); i++){
        pElement = mvElement[i];
        if(pElement->getID() > maxID) maxID = pElement->getID();
        if(pElement->getID() < minID) minID = pElement->getID();
    };

    moBucket.resizeBucketElement(maxID,minID);

    // set data
    for(i=0; i < mvElement.size(); i++){
        pElement = mvElement[i];
        moBucket.setIndexElement(pElement->getID(), i);
    };
}

////
//// Node
////

// Reserve mvNode
//
void CMesh::reserveNode(const uint& num_of_node)
{
    mvNode.reserve(num_of_node);
    //numOfNode= num_of_node;
}

// Node  setup
//
// arg::id  ==> Bucket
//
void CMesh::setNode(CNode *pNode)
{
    ////debug
    //cout << "CMesh::setNode  ID = " << pNode->getID() << endl;

    mvNode.push_back(pNode);

    ////debug
    //cout << "CMesh::setNode mvNode.size() = " << mvNode.size() << endl;
}

//
//
CNode* CMesh::getNode_inID(const uint &id)
{
    uint index;
    index = moBucket.getIndexNode(id);

    if(index==-1){ index=id;}// index==-1, ID Nothing

    return mvNode[index];
}

// mvNode[index]
//
CNode* CMesh::getNode(const uint& index)
{
    //debug
    if(mvNode.size() <= index) mpLogger->Info(Utility::LoggerMode::Error,"CMesh::getNode, Mesh Node size over",index);

    return mvNode[index];
}


////
//// Element
////

// Reserve mvElement
//
void CMesh::reserveElement(const uint& num_of_elem)
{
    mvElement.reserve(num_of_elem);
    //numOfElement= num_of_elem;
}

// Element Setup
//
// arg::id => Bucket
//
void CMesh::setElement(CElement *pElement)
{
    mvElement.push_back(pElement);
}

//
//
CElement* CMesh::getElement_inID(const uint& id)
{
    uint index;
    index = moBucket.getIndexElement(id);

    if(index==-1){ index = id;}// index== -1, ID Nothing

    return mvElement[index];
}

// mvElement[index]
//
CElement* CMesh::getElement(const uint& index)
{
    return mvElement[index];
}

////
//// Boundary Node (Displacement)
////
//void CMesh::setBDispNode(const uint& node_id)
//{
//    CNode* pNode;
//    int index;
//    index = moBucket.getIndexNode(node_id);
//    pNode = mvNode[index];
//
//    moNodeBoundaryDisp.setID(node_id);
//    moNodeBoundaryDisp.setNode(pNode);
//}
//
//// Boundary Node
////
//void CMesh::setBLoadNode(const uint& node_id)
//{
//    CNode* pNode;
//    int index;
//    index = moBucket.getIndexNode(node_id);
//    pNode = mvNode[index];
//
//    moNodeBoundaryLoad.setID(node_id);
//    moNodeBoundaryLoad.setNode(pNode);
//}

// Aggregate-Element, Aggregate-Node
// ---
//
void CMesh::reserveAggregate(const uint& res_size)
{
    mvAggElement.reserve(res_size);
    mvAggNode.reserve(res_size);
}

void CMesh::setAggElement(CAggregateElement* pAggElem)
{
    mvAggElement.push_back(pAggElem);
}

void CMesh::setAggNode(CAggregateNode* pAggNode)
{
    mvAggNode.push_back(pAggNode);
}

// AggregateElement,AggregateNode のセットアップ
// ---
void CMesh::setupAggregate()
{
    ////debug
    //Utility::CLogger *pLogger= Utility::CLogger::Instance();
    
    CElement *pElem;   CNode *pNode;
    uint ielem, local_id, inode;
    
    // 第2段 以降のprolongationのときに,
    //  pNode(Vertex)のElemIndexに同じIDが入らないようにクリア.
    for(inode=0; inode< mvNode.size(); inode++){
        pNode= mvNode[inode];
        pNode->clearElemIndex();
    };
    
    // Node(Vertex)が所属する、要素のIndex番号をセット(in Node)
    //     (複数の要素に所属している)
    //    
    for(ielem=0; ielem< numOfElement; ielem++){
        pElem = mvElement[ielem];
        for(local_id=0; local_id< pElem->getNumOfNode();local_id++){
            pNode = pElem->getNode(local_id);
            pNode->setElemIndex(ielem);
        };
    };


    CAggregateElement *pAggElem;
    uint elemIndex,iagg_elem, numOfAggElem;
    //
    // Nodeにセットされた要素のIDを基に,AggregateElement(in Mesh)をセット
    //
    //for(inode=0; inode< mvNode.size(); inode++){ // △09.08.11にnumOfNodeに変更
    for(inode=0; inode< numOfNode; inode++){
        pNode = mvNode[inode];
        numOfAggElem = pNode->getNumOfAggElem();

        pAggElem = mvAggElement[inode];
        pAggElem->reserve(numOfAggElem);

        for(iagg_elem=0; iagg_elem< numOfAggElem; iagg_elem++){
            elemIndex = pNode->getElemIndex(iagg_elem);
            pElem = mvElement[elemIndex];

            pAggElem->push(pElem);
        };
    };

    CAggregateNode *pAggNode;
    vector<CNode*> vConnNode;
    uint ncon, nNumOfConnNode;
    // AggregateElementを基に,AggregateNodeをセット
    //
    for(inode=0; inode< numOfNode; inode++){
        pNode= mvNode[inode];
        numOfAggElem= pNode->getNumOfAggElem();

        ////debug
        //cout << "mvAggNode.size => " << mvAggNode.size() << endl;

        pAggNode = mvAggNode[inode];
        pAggNode->reserveNode(numOfAggElem);

        for(iagg_elem=0; iagg_elem< numOfAggElem; iagg_elem++){
            elemIndex= pNode->getElemIndex(iagg_elem);
            pElem= mvElement[elemIndex];

            // pNodeのIDに対応する辺の一方の端のNodeを拾う.
            vConnNode= pElem->getConnectNode(pNode);
            nNumOfConnNode= vConnNode.size();

            ////debug
            //cout << "nNumOfConnNode => " << nNumOfConnNode << endl;

            // Node集合にセットしていく.
            //
            for(ncon=0; ncon < nNumOfConnNode; ncon++){
                pAggNode->setNode(vConnNode[ncon]);//既に取得済みのNodeかどうか,pAggNodeが判定してAggNodeに追加.
            };
        };
    };
}



// ○ 要素辺、面 などにノードを生成する前に、土台Mesh{このMesh自身、つまり"this＊"}のノード&要素データを上位Meshにプリセット
// ○ pProgMesh::Node,Elementのreserve
//
void CMesh::presetProgMesh(CMesh* pProgMesh)
{
    uint progNumOfNode= numOfNode;
    uint progNumOfElem= numOfElement;
    //uint numOfHexa(0),numOfTetra(0),numOfPrism(0),numOfPyramid(0),numOfQuad(0),numOfTriangle(0),numOfBeam(0);//現Meshの種類別数
    
    // 増加要素 予測
    CElement *pElem;
    uint ielem;
    for(ielem=0; ielem< numOfElement; ielem++){
        pElem= mvElement[ielem];
        
        // 要素ごとの要素増加数 & 現Meshの要素種類別数
        switch(pElem->getType()){
            case(ElementType::Hexa):case(ElementType::Hexa2):
                progNumOfElem += 7;//増加する要素数なので、Hexaを8分割できるから、増加数==7;
                //numOfHexa++;
                break;
            case(ElementType::Tetra):case(ElementType::Tetra2):
                progNumOfElem += 3;//増加数==3, 分割数==4
                //numOfTetra++;
                break;
            case(ElementType::Prism):case(ElementType::Prism2):
                progNumOfElem += 5;//増加数==5, 分割数==6
                //numOfPrism++;
                break;
            case(ElementType::Pyramid):case(ElementType::Pyramid2):
                progNumOfElem += 7;//増加数==7, 分割数==8
                //numOfPyramid++;
                break;
            case(ElementType::Quad):case(ElementType::Quad2):
                progNumOfElem += 3;//増加数==3, 分割数==4
                //numOfQuad++;
                break;
            case(ElementType::Triangle):case(ElementType::Triangle2):
                progNumOfElem += 2;//増加数==2, 分割数==3
                //numOfTriangle++;
                break;
            case(ElementType::Beam):case(ElementType::Beam2):
                progNumOfElem += 1;//増加数==1, 分割数==2
                //numOfBeam++;
                break;
        }
    };
////    // 増加ノード予測(大雑把) -> 見直しのこと.
////    progNumOfNode += numOfHexa*7;//Hexa 辺ノード(3倍),面ノード(3倍),体ノード(1倍)
////    progNumOfNode += numOfTetra*4;//Tetra 辺ノード(1倍),面ノード(2倍),体ノード(1倍)
////    progNumOfNode += numOfPrism*5.5;//Prism 辺ノード(2倍),面ノード(2.5倍),体ノード(1倍)
////    progNumOfNode += numOfPyramid*4;//Pyramid 辺ノード((2/3)倍),面ノード(2.5倍),体ノード(1倍)== (25/6) ~= 4
////    progNumOfNode += numOfQuad*3;//Quad 辺ノード(2倍),面ノード(1倍),体ノード(0倍)
////    progNumOfNode += numOfTriangle*2.5;//Triangle 辺ノード(1.5倍),面ノード(1倍),体ノード(0倍)
////    progNumOfNode += numOfBeam;//Beam 辺ノード(1倍),面ノード(0倍),体ノード(0倍)

    // 増加ノード予測(更に大雑把) => 2*2*2=8
    progNumOfNode *= 8;

    pProgMesh->reserveNode(progNumOfNode);   //暫定的なノード数である.
    pProgMesh->reserveElement(progNumOfElem);//確定した要素数である.

    // --
    // NodeデータをpProgMeshにセット
    // --
    uint i;
    for(i=0; i< numOfNode; i++){
        pProgMesh->setNode(mvNode[i]);
    };
    // --
    // Elementデータは,pProgMeshにセットしない.
    //  => 要素については,新たに生成する要素のみがpProgMeshの要素である.
    // --
    

    // ○ ノード(Node)数については,未確定
    //     ->setupEdgeElement,setupFaceElement,setupVolumeNodeでカウントしてその都度セットする.
    // ○ 要素数は確定
    // pProgMesh->setNumOfElement(progNumOfElem);
    //
    //  -> FactoryでのsetupNumOfElement()に変更(vectorのsize()を利用する)


    //debug
    Utility::CLogger *pLogger= Utility::CLogger::Instance();
    pLogger->Info(Utility::LoggerMode::MWDebug,"prolongation numOfElem",progNumOfElem);
    pLogger->Info(Utility::LoggerMode::MWDebug,"prolongation numOfNode",progNumOfNode);

}


// 各要素の辺に接する要素集合の集計(要素の自分自身のCElement*も集合に含まれる)=> 各要素に辺要素集合をセット
//                && prolongation中間ノードの生成(現LevelのMeshでのノード数に影響なし)
//
void CMesh::setupEdgeElement(CMesh *pProgMesh)
{
    // 節点の要素集合は取得済みとする(mvAggregateElement)
    //
    CElement *pElem;
    PairNode edgeNode;
    CNode *pNode0,*pNode1;
    
    // pElemまわりに集められる要素に対する処理用
    //
    CElement *pOtherElem;
    vector<CElement*> vEdgeElem;//一つの要素の辺の要素集合
    ////vector<uint> vEdgeElemIndex;//一つの要素の辺の要素Index集合
    uint nOtherEdge;// elem loop外の要素の辺(Edge)index番号

    //vint pairIndex;  pairIndex.resize(2);
    uint numOfEdge;
    uint ielem,iedge;
    uint indexCount;//ノードID生成用
    CNode *inNode;  //中間ノード
    
    //indexCount = pProgMesh->getNumOfNode();
    indexCount = pProgMesh->getNodeSize();//prolongation MeshのノードIDの初期値

    ////debug
    //cout << "indexCount initial = " << indexCount << ", CMesh::setupEdgeElement" << endl;
    
    // 要素 ループ
    for(ielem=0; ielem< numOfElement; ielem++){
        pElem = mvElement[ielem];
        numOfEdge = pElem->getNumOfEdge();
        
        // 辺 ループ
        for(iedge=0; iedge< numOfEdge; iedge++){
            
            edgeNode = pElem->getPairNode(iedge);
            pNode0= edgeNode.first;  pNode1= edgeNode.second;

            ////debug
            //{
            //    vint vpair; vpair.resize(2); vpair[0]= pNode0->getID(); vpair[1]= pNode1->getID();
            //    mpLogger->Monitor(Utility::LoggerMode::MWDebug, pElem->getID(), vpair, "CMesh::setupEdgeElement");
            //}

            // 同一のEdgeを処理することを回避
            if(!pElem->isEdgeElem(pNode0,pNode1)){
                //pElem->getPairNode(pairIndex,iedge);
                uint iagg,jagg, elemIndex0, elemIndex1;
                uint numOfAggElem0=pNode0->getNumOfAggElem();// ノードの要素集合
                uint numOfAggElem1=pNode1->getNumOfAggElem();// ノードの要素集合

                pElem->reserveEdgeElement(iedge,numOfAggElem0);// ノードの要素集合の個数を越えることはないので、ノード要素集合数で確保


                // 両端のノードの要素集合比較による, 辺の要素集合
                //
                for(iagg=0; iagg< numOfAggElem0; iagg++){
                    elemIndex0= pNode0->getElemIndex(iagg);
                    for(jagg=0; jagg< numOfAggElem1; jagg++){
                        elemIndex1= pNode1->getElemIndex(jagg);
                        
                        //pElem自身もEdgeElementに入れる.
                        if(elemIndex0 == elemIndex1)
                            pElem->setEdgeElement(iedge, mvElement[elemIndex0]);
                    };
                };

                
                
                // Edge(辺)中間ノードの生成.
                inNode= GeneInterNode(pNode0);// 辺ノードの生成

                // ノード2点間の平均
                vdouble P0_coord= pNode0->getCoord();
                vdouble P1_coord= pNode1->getCoord();
                vdouble In_coord; In_coord.reserve(3);
                for(uint i=0; i< 3; i++){ In_coord.push_back( (P0_coord[i] + P1_coord[i])*0.5 );}

                inNode->setCoord(In_coord);//inNode(中間ノード)に座標値をセット
                inNode->setID(indexCount); //indexCountノード数カウントのセット
                
                pElem->setEdgeInterNode(inNode,iedge);//ループ要素自身に中間ノードをセット
                pElem->setBoolEdgeElem(pNode0, pNode1);//iedgeにEdgeElementとNodeをセットしたことをスタンプ
                
                pProgMesh->setNode(inNode);
                indexCount++;

                vEdgeElem.clear();
                // EdgeElement ループ:pElemの辺に集めた要素集合を、要素集合に含まれる要素の各辺にセットする
                //
                vEdgeElem= pElem->getEdgeElement(iedge);
                uint n;
                for(n=0; n< vEdgeElem.size(); n++){
                    pOtherElem=vEdgeElem[n];

                    //条件1.辺の要素集合にはpElem自身も入っているので,pElemを除外
                    //条件2.辺の要素集合の要素の辺が既にEdge要素をセットしてあるか(isEdgeElem==false)判定
                    if(pOtherElem->getID() != pElem->getID() && !pOtherElem->isEdgeElem(pNode0,pNode1)){
                        nOtherEdge =pOtherElem->getEdgeIndex(pNode0, pNode1);//pNode0,pNode1に該当する辺のIndex番号
                        pOtherElem->setEdgeAggElement(nOtherEdge,vEdgeElem);//集められた要素集合を一度にセット
                        pOtherElem->setBoolEdgeElem(pNode0, pNode1);        //辺の要素集合をセット済みをスタンプ

                        pOtherElem->setEdgeInterNode(inNode,nOtherEdge);//辺集合の要素に中間ノードをセット
                    }
                };//for(vEdgeElemのループ)
                
            }//ifブロック終端(pElemの一つの辺の処理終端
            
        };//for ( iedge )
        
    };//for (ielem)

    // 現時点でのノード数をセット
    //pProgMesh->setNumOfNode(indexCount);
    // -> Factoryで行う.

    ////debug
    //cout << "indexCount final   = " << indexCount << ", CMesh::setupEdgeElement" << endl;
    //cout << "pProgMesh NodeSize = " << pProgMesh->getNodeSize() << ", CMesh::setupEdgeElement" << endl;
    
}

// prolongationの為の,中間節点の生成(setupEdgeElementからコール) <= 現LevelのMeshでのノード数に影響なし
//
CNode* CMesh::GeneInterNode(CNode* pNode)
{
    CNode *inNode;
    uint numOfScalar, numOfVector;
    // 要素内では同一種類のノードで構成されるので、片方のノードの型で判断
    // --
    // Scalar
    switch(pNode->getType()){
        case(NodeType::Scalar):
            inNode= new CScalarNode;
            numOfScalar= pNode->numOfScalarParam();
            inNode->reserveScalar(numOfScalar);//DOFの確保

            break;
        case(NodeType::Vector):
            inNode= new CVectorNode;
            numOfVector= pNode->numOfVectorParam();
            inNode->reserveVector(numOfVector);

            break;
        case(NodeType::ScalarVector):
            inNode= new CScalarVectorNode;

            numOfScalar= pNode->numOfScalarParam();
            numOfVector= pNode->numOfVectorParam();

            inNode->reserveScalar(numOfScalar);
            inNode->reserveVector(numOfVector);

            break;
        default:
            mpLogger->Info(Utility::LoggerMode::Error,"Node Generation Error, CMesh::GeneInterNode");
            break;
    }

    inNode->setMGLevel(mMGLevel+1);//prolongation用としてのノードなのでLevelを一段上げる

    ////debug
    //mnDummyCount++;
    //cout << "GeneInterNode count = " << mnDummyCount << endl;

    return inNode;
}

// 各要素の面に隣接する要素を検索 => 要素自身に、面で隣接する要素をセット
//
void CMesh::setupFaceElement(CMesh* pProgMesh)
{
    CElement *pElem;
    vector<CNode*> vFaceCnvNode;
    uint  elemIndex0, elemIndex1;
    CNode *pNode0,*pNode1,*pNode2;
    CNode *inNode;//中間ノード用ポインター
    uint ielem,isurf,jsurf, iagg,jagg, kagg;
    uint numAggElems0, numAggElems1, numAggElems2;
    uint numOfFace;

    vuint vnShareElems0, vnShareElems1;//共有_要素インデックス

    uint indexCount;// prolongation MeshのノードID
    indexCount = pProgMesh->getNodeSize();


    for(ielem=0; ielem< numOfElement; ielem++){
        pElem= mvElement[ielem];
        numOfFace= pElem->getNumOfFace();

        for(isurf=0; isurf< numOfFace; isurf++){
            
            vFaceCnvNode = pElem->getFaceCnvNodes(isurf);//面を構成するノードの取得

            pNode0= vFaceCnvNode[0]; pNode1= vFaceCnvNode[1]; pNode2= vFaceCnvNode[2];
            
            //共有 要素インデックス クリア
            vnShareElems0.clear(); vnShareElems1.clear();


            // 既にFaceElemがセットされているか判定(セットされている=> true)
            //
            if(!pElem->isFaceElem(pNode0,pNode1,pNode2)){

                numAggElems0= pNode0->getNumOfAggElem(); vnShareElems0.reserve(numAggElems0);
                numAggElems1= pNode1->getNumOfAggElem();
                //
                // ノード0 とノード1の共有elemIndex => vnShareElems0
                //
                for(iagg=0; iagg< numAggElems0; iagg++){
                for(jagg=0; jagg< numAggElems1; jagg++){
                    elemIndex0= pNode0->getElemIndex(iagg);
                    elemIndex1= pNode1->getElemIndex(jagg);

                    if(elemIndex0 == elemIndex1)
                        vnShareElems0.push_back(elemIndex0);//ノード0,ノード1 の共有elemIndex

                };//iagg ループ(Vertex周囲の要素インデックス)
                };//jagg ループ

                numAggElems2= pNode2->getNumOfAggElem(); vnShareElems1.reserve(numAggElems2);
                //
                // ノード2 と vnShareElems0(ノード0とノード1の共有要素)との共有elemIndex => vnShareElems1
                //
                for(jagg=0; jagg< vnShareElems0.size(); jagg++){
                for(kagg=0; kagg< numAggElems2; kagg++){
                    if(vnShareElems0[jagg] == pNode2->getElemIndex(kagg))
                        vnShareElems1.push_back(vnShareElems0[jagg]);//ノード0,ノード1,ノード2 の共有elemIndex

                };//kagg ループエンド
                };//jagg ループエンド
                

                // 1.隣接(Adjacent)要素をpElemへセットアップ
                // 2.隣接要素へpElemをセットアップ
                //
                CElement* pAdjElem;

                // 隣接要素が存在する面
                if(vnShareElems1.size()> 1){
                    // pElem自身と隣接要素で2つ. 2つを越える場合は隣接要素形状が2分割されている.<= 現状のメッシュは隣接メッシュは一つとする.
                    for(uint i=0; i< vnShareElems1.size(); i++){

                        if(pElem->getID() != vnShareElems1[i]){
                            
                            pAdjElem= mvElement[vnShareElems1[i]];

                            pElem->setFaceElement(pAdjElem, isurf);
                            pElem->setBoolFaceElem(pNode0,pNode1,pNode2);//隣接要素をセットしたことをスタンプ

                            ///////////////////////////
                            // 面ノードの生成,セット
                            inNode= GeneInterNode(pNode0);
                            avgCoord(vFaceCnvNode, inNode);//vFaceCnvNodeの平均座標をセット
                            inNode->setID(indexCount);//IDをセット
                            pProgMesh->setNode(inNode);
                            pElem->setFaceNode(inNode, isurf);// Faceへノードをセット(要素ループしている要素)

                            
                            //隣接要素の"pNode0,pNode1,pNode2で構成される面へpElemをセット
                            //
                            jsurf= pAdjElem->getFaceIndex(pNode0, pNode1, pNode2);
                            pAdjElem->setFaceElement(pElem, jsurf);
                            pAdjElem->setBoolFaceElem(pNode0,pNode1,pNode2);//隣接要素をセットしたことをスタンプ
                            pAdjElem->setFaceNode(inNode, jsurf);           // Faceへノードをセット(隣の要素)

                            // IDのためのカウントアップ
                            indexCount++;
                        }
                    };
                // 隣接要素が存在しない面
                }else{
                    //////////////////////////////
                    //面ノードの生成,セット
                    inNode= GeneInterNode(pNode0);
                    avgCoord(vFaceCnvNode, inNode);//vFaceCnvNodeの平均座標をセット

                    inNode->setID(indexCount);//IDをセット
                    pProgMesh->setNode(inNode);
                    
                    
                    // IDのためのカウントアップ
                    indexCount++;
                    
                    pElem->setFaceNode(inNode,isurf);
                    pElem->setBoolFaceElem(pNode0, pNode1, pNode2);
                }
            }//if(!isFaceElem) ブロック エンド
        };//isurf ループ エンド
    };//ielem ループ エンド
    
    // 現時点でのノード数をセット
    // pProgMesh->setNumOfNode(indexCount);
    //   -> FactoryでのsetupNumOfNode()に統一.
}


// 要素の中心にノードを生成、セットアップ
//
void CMesh::setupVolumeNode(CMesh *pProgMesh)
{
    CElement* pElem;
    CNode *pNode,*cntNode;
    vector<CNode*> vLocalNode;
    uint numOfLocalNode;
    vdouble vCoord;
    uint ielem, inode;

    uint indexCount;// prolongation MeshのノードID
    indexCount = pProgMesh->getNodeSize();

    ////debug
    //cout << "indexCount initial = " << indexCount << ", CMesh::setupVolumeNode" << endl;


    for(ielem=0; ielem< numOfElement; ielem++){
        pElem= mvElement[ielem];

        // Solid要素のみにVolume中心ノードを設置
        //
        //if(pElem->getNumOfFace() > 1){
        if(pElem->getEntityType()==BaseElementType::Solid){
            pNode= pElem->getNode(0);//"局所番号0"のノード <= 生成するノードの種類を決めるために必要.

            cntNode= GeneInterNode(pNode);//要素中心ノードの生成(pNodeと同一種類のノード)
            cntNode->setID(indexCount);

            vCoord.clear();
            vCoord.resize(3);
            vCoord[0]=0.0; vCoord[1]=0.0; vCoord[2]=0.0;
            //局所ノードの座標平均をとる
            vLocalNode = pElem->getNode();
            numOfLocalNode= pElem->getNumOfNode();
            for(inode=0; inode< numOfLocalNode; inode++){
                pNode= vLocalNode[inode];

                vCoord[0] += pNode->getX();
                vCoord[1] += pNode->getY();
                vCoord[2] += pNode->getZ();
            };
            vCoord[0] /= (double)numOfLocalNode;  vCoord[1] /= (double)numOfLocalNode;  vCoord[2] /= (double)numOfLocalNode;

            //生成した中心ノードに座標値をセット
            cntNode->setCoord(vCoord);

            pProgMesh->setNode(cntNode);
            ++indexCount;

            pElem->setVolumeNode(cntNode);//要素へ中心ノードをセット
        }//if()エンド：要素の面が1を越える=> Solid要素
    };

    ////debug
    //cout << "indexCount final   = " << indexCount << ", CMesh::setupVolumeNode" << endl;
    //cout << "pProgMesh NodeSize = " << pProgMesh->getNodeSize() << ", CMesh::setupVolumeNode" << endl;

    // 現時点でのノード数をセット
    // pProgMesh->setNumOfNode(indexCount);
    //   -> FactoryでのsetupNumOfNode()に統一.
}


// 複数のノードの座標平均をpNodeにセット
//
void CMesh::avgCoord(vector<CNode*> vCnvNode, CNode *pNode)
{
    //inNodeの座標を計算(平均値)
    vdouble vCoord; vCoord.resize(3);
    uint i;
    for(i=0; i< 3; i++){ vCoord[i]=0.0;}

    uint numOfFaceNode = vCnvNode.size();
    for(i=0; i< numOfFaceNode; i++){
        vCoord[0] += vCnvNode[i]->getX();
        vCoord[1] += vCnvNode[i]->getY();
        vCoord[2] += vCnvNode[i]->getZ();
    };
    for(i=0; i< 3; i++){
        vCoord[i] /= (double)numOfFaceNode;
    };

    ////debug
    //cout << "vCoord[0]= " << vCoord[0] << ", vCoord[1]= " << vCoord[1] << ", vCoord[2]= " << vCoord[2]<< endl;

    //座標値をinNodeにセットする.
    pNode->setCoord(vCoord);
}












