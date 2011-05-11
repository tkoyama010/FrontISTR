//
//	FileReaderNode.cpp
//
//				2008.05.26
//				2008.12.08
//				k.Takeda
#include "FileReaderNode.h"
using namespace FileIO;

//
//
CFileReaderNode::CFileReaderNode()
{
}

CFileReaderNode::~CFileReaderNode()
{
}

//// Node(MaxID,MinID,)
////
//bool CFileReaderNode::ParseRead(ifstream& ifs, char* line)
//{
//	string sLine = line;
//	uint  nNodeID;
//	uint  nNodeCount;
//
//	// Node
//	if(CFileBlockName::StartNode() == sLine){
//		nNodeCount=0;
//		// Node_ID
//		while(true){
//			sLine = getLineSt(ifs);
//			if(CFileBlockName::EndNode() == sLine) break;
//
//			// ID
//			istringstream iss(sLine.c_str());
//			iss >> nNodeID;
//
//			++nNodeCount;//Up
//		};
//		cout << "== " << nNodeCount << endl;
//		return true;
//	}else{
//		return false;
//	}
//}



//
//  StartTag
//      numOfNode  Mesh_ID  maxID  minID
//      NodeType numOfScalar numOfVector id  x   y   z
//      ....
//  EndTag
//
// --
// Nodeの生成(Factory),AggregateElementの生成(Factory)
// --
//
bool CFileReaderNode::Read(ifstream& ifs, string& sLine)
{
    uiint   nNodeID, nMeshID, numOfNode, maxID, minID;
    vdouble vCoord; vCoord.resize(3);
    uiint  nType, nNumOfScalarDOF, nNumOfVectorDOF;
    string sType;

    uiint  mgLevel(0); // MultiGrid Level==0 ::ファイル入力時は 0

    // Node
    if(TagCheck(sLine, FileBlockName::StartNode()) ){
        
        ////debug
        //cout << "FileReaderNode::Read" << endl;

        sLine = getLineSt(ifs);
        istringstream iss(sLine.c_str());
        // ノード数, MeshID, MaxID, MinID
        iss >> numOfNode >> nMeshID >> maxID >> minID;


        // Node コンテナ確保
        mpFactory->reserveNode(mgLevel, nMeshID, numOfNode);
        mpFactory->initBucketNode(mgLevel, nMeshID, maxID, minID);

        uiint nCount(0);
        // Node 読み込み
        //
        while(!ifs.eof()){
            sLine = getLineSt(ifs);
            if(TagCheck(sLine, FileBlockName::EndNode()) ) break;

            istringstream iss(sLine.c_str());
            // iss -> Token  取得: NodeID, X, Y, Z
            iss  >> sType >> nNumOfScalarDOF >> nNumOfVectorDOF >> nNodeID >> vCoord[0] >> vCoord[1] >> vCoord[2];
            
            if(sType=="sv"||sType=="SV"){
                nType=pmw::NodeType::ScalarVector;
            }else if(sType=="s"||sType=="S"){
                nType=pmw::NodeType::Scalar;
            }else if(sType=="v"||sType=="V"){
                nType=pmw::NodeType::Vector;
            }
            else{
                mpLogger->Info(Utility::LoggerMode::Error,"NodeType mismatch, at FileReaderNode");
            }
            
            ////debug
            // mpLogger->Monitor(Utility::LoggerMode::MWDebug, nNodeID, vCoord, "Node");
            
            if(!mpFactory) mpLogger->Info(Utility::LoggerMode::MWDebug, "Factory => NULL, at FileReaderNode");

            // Node生成
            mpFactory->GeneNode(mgLevel, nMeshID, nNodeID, vCoord, nType, nNumOfScalarDOF, nNumOfVectorDOF);
            mpFactory->setIDBucketNode(mgLevel, nMeshID, nNodeID, nCount);
            nCount++;
        };
        
        //// mpFactory->setupBucketNode(mgLevel, nMeshID);// Bucket Node (in Mesh):: Nodeを全てセットした後にBucketを,一括処理
        mpFactory->setupNode(mgLevel, nMeshID);

        mpFactory->resizeAggregate(mgLevel, nMeshID, nCount);// Aggregate-Element, Aggregate-Nodeの領域確保
        mpFactory->GeneAggregate(mgLevel, nMeshID, nCount);  // Aggregate-Element, Aggregate-Nodeの生成

        return true;
    }else{
        return false;
    }
}


bool CFileReaderNode::Read_bin(ifstream& ifs)
{
    return true;
}

