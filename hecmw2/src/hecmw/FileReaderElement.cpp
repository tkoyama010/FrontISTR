//
//	FileReaderElement.cpp
//
//				2008.04.20
//				2008.12.08
//				k.Takeda
#include "ElementType.h"
#include "FileReaderElement.h"
using namespace FileIO;

// construct & destruct
//
CFileReaderElement::CFileReaderElement()
{
}
CFileReaderElement::~CFileReaderElement()
{
}


bool CFileReaderElement::Read(ifstream& ifs, string& sLine)
{
    string sTypeName;
    uiint   nElementID, numOfElem, nMeshID, maxID, minID;
    vint   vLocalNode;
    uiint   mgLevel(0);
    uiint  i;
    
    if(TagCheck(sLine, FileBlockName::StartElement()) ){// スタート タグ
        
        ////debug
        //cout << "FileReaderElement::Read" << endl;

        sLine = getLineSt(ifs);

        istringstream iss(sLine.c_str());
        iss >> numOfElem >> nMeshID >> maxID >> minID;

        mpFactory->reserveElement(mgLevel, nMeshID, numOfElem);
        mpFactory->initBucketElement(mgLevel, nMeshID, maxID, minID);

        uiint nCount(0);// Index for mvElement
        //
        // 要素　読み込み -> Element生成, Bucketセットアップ(ID,Index)
        //
        while(!ifs.eof()){
            sLine = getLineSt(ifs);
            if(TagCheck(sLine, FileBlockName::EndElement()) ) break; // エンド タグ

            istringstream iss(sLine.c_str());
            // iss -> Token 
            iss >> sTypeName;

            vLocalNode.clear();

            // 要素タイプ別処理
            // --------------
            // 1次要素
            if(sTypeName=="Tetra"){
                vLocalNode.resize(4);

                // Element_ID, 局所ノードID, 局所ノードID, 局所ノードID, 局所ノードID
                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }
                
                //// debug
                //mpLogger->Monitor(Utility::LoggerMode::MWDebug, nElementID, vLocalNode, sTypeName);


                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Tetra,vLocalNode);// Tetra
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);//Bucket (ID & Index)
                nCount++;//次のループで、BucketへIndexをセットするためカウントアップ
            }
            if(sTypeName=="Hexa"){
                vLocalNode.resize(8);

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Hexa,vLocalNode);// Hexa
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Prism"){
                vLocalNode.resize(6);

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i];}

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Prism,vLocalNode);// Prism
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Triangle"){
                vLocalNode.resize(3);

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i];}

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Triangle,vLocalNode);// Triangle
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Quad"){
                vLocalNode.resize(4);

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Quad,vLocalNode);// Quad
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Beam"){
                vLocalNode.resize(2);

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Beam,vLocalNode);// Beam
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }

            // 2次要素
            if(sTypeName=="Tetra2"){
                vLocalNode.resize(pmw::NumberOfNode::Tetra2());

                // Element_ID, 局所ノードID, 局所ノードID, 局所ノードID, 局所ノードID
                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                //// debug
                //mpLogger->Monitor(Utility::LoggerMode::MWDebug, nElementID, vLocalNode, sTypeName);


                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Tetra2, vLocalNode);// Tetra2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);//Bucket (ID & Index)
                nCount++;//次のループで、BucketへIndexをセットするためカウントアップ
            }
            if(sTypeName=="Hexa2"){
                vLocalNode.resize(pmw::NumberOfNode::Hexa2());

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Hexa2, vLocalNode);// Hexa2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;

                //cout << "FileReaderElement::Read  GeneElement End" << endl;
            }
            if(sTypeName=="Prism2"){
                vLocalNode.resize(pmw::NumberOfNode::Prism2());

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i];}

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Prism2, vLocalNode);// Prism2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Triangle2"){
                vLocalNode.resize(pmw::NumberOfNode::Triangle2());

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i];}

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Triangle2, vLocalNode);// Triangle2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Quad2"){
                vLocalNode.resize(pmw::NumberOfNode::Quad2());

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Quad2, vLocalNode);// Quad2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
            if(sTypeName=="Beam2"){
                vLocalNode.resize(pmw::NumberOfNode::Beam2());

                iss >> nElementID;
                for(i=0; i< vLocalNode.size(); i++){ iss >> vLocalNode[i]; }

                mpFactory->GeneElement(mgLevel, nMeshID, nElementID, pmw::ElementType::Beam2, vLocalNode);// Beam2
                mpFactory->setIDBucketElement(mgLevel, nMeshID, nElementID, nCount);
                nCount++;
            }
        };
        //// Bucket setup (All in-One 処理)
        // mpFactory->setupBucketElement(mgLevel, nMeshID);// IDからIndex取得のHash
        mpFactory->setupElement(mgLevel, nMeshID);// <= CMesh::numOfElementの設定
        
        return true;
    }else{
        return false;
    }
}


bool CFileReaderElement::Read_bin(ifstream& ifs)
{
    return true;
}


