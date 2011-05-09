//
//  MeshFactory.h
//
//			2009.07.23
//			2008.11.05
//			k.Takeda
#ifndef MESH_FACTORY_HH_E74CAEBF_35A3_407b_A7B8_49D7A2DDDF8E
#define MESH_FACTORY_HH_E74CAEBF_35A3_407b_A7B8_49D7A2DDDF8E

#include "CommonStd.h"
#include "TypeDef.h"

#include "Logger.h"
#include "Element4Factory.h" // Elemnt => Hexa,Tetra,Prism,Pyramid,Quad,Triangle,Beam

#include "ElementProperty.h"// 頂点数,面数,辺数

#include "ScalarNode.h"
#include "VectorNode.h"
#include "ScalarVectorNode.h"
#include "NodeType.h"

#include "BoundaryNode.h"
#include "BoundaryFace.h"
#include "BoundaryVolume.h"
#include "BoundaryType.h"

#include "AggregateElement.h"
#include "AggregateNode.h"
#include "Mesh.h"
#include "AssyModel.h"
#include "GMGModel.h"

#include "Material.h"

namespace pmw{
class CMeshFactory
{
public:
    static CMeshFactory* Instance(){
        static CMeshFactory meshFactory;
        return &meshFactory;
    }
private:
    CMeshFactory(void);
public:
    //CMeshFactory(void);
    virtual ~CMeshFactory(void);

protected:
    CGMGModel    *mpGMGModel;

    CAssyModel   *mpTAssyModel;// AssembledMesh Model
    CMesh        *mpTMesh;     // each Mesh
    uint    mNumOfMGLevel;//マルチグリッド階層数 => 1.ならマルチグリッドなし

    Utility::CLogger *mpLogger;

public:
    // root Model
    virtual void setGMGModel(CGMGModel* pGMGModel){ mpGMGModel = pGMGModel;}
    // assembled Model
    void GeneAssyModel(const uint& num_of_mgLevel);// AssyModelをmgLevel数ぶんだけ生成
    // each Mesh in AssyModel at mgLevel
    void reserveMesh(const uint& mgLevel,const uint& num_of_mesh);
    void GeneMesh(const uint& mgLevel, const uint& mesh_id, const uint& index);// MeshをAssyModelに生成 at mgLevel


    // Refine for MultiGrid
    // --------------------
    void setMGLevel(const uint& num_of_mglevel){ mNumOfMGLevel= num_of_mglevel;}
    uint& getMGLevel(){ return mNumOfMGLevel;}
    void refineMesh();
protected:
    void dividHexa(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividTetra(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividPrism(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividPyramid(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividQuad(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividTriangle(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);
    void dividBeam(CElement* pElem, vector<CElement*>& vProgElem, uint& indexCount, CMesh* pProgMesh);

public:
    // Node
    // ----
    void reserveNode(const uint& mgLevel, const uint& mesh_id, const uint& num_of_node);
    void GeneNode(const uint& mgLevel, const uint& mesh_id, const uint& id, const vdouble& coord,
                  const uint& nodeType, const uint& numOfScaParam, const uint& numOfVecParam);
    void setupNode(const uint& mgLevel, const uint& mesh_id);//numOfNodeのセット

    // Aggregate-Element, Aggregate-Node
    //
    void reserveAggregate(const uint& mgLevel, const uint& mesh_id, const uint& num_of_node);
    void GeneAggregate(const uint& mgLevel, const uint& mesh_id, const uint& num_of_node);


    // Boundary
    // --------
    void reserveBoundaryNode(const uint& mgLevel, const uint& mesh_id, const uint& num_of_bnd);
    void reserveBoundaryFace(const uint& mgLevel, const uint& mesh_id, const uint& num_of_bnd);
    void reserveBoundaryVolume(const uint& mgLevel, const uint& mesh_id, const uint& num_of_bnd);
    void GeneBoundaryNode(const uint& mgLevel, const uint& mesh_id, const uint& id,
                          const uint& dof, const uint& bndType,const vdouble& vVal);
    void GeneBoundaryFace(const uint& mgLevel, const uint& mesh_id, const uint& elem_id, const uint& face_id,
                          const uint& dof, const uint& bndType,const vdouble& vVal);
    void GeneBoundaryVolume(const uint& mgLevel, const uint& mesh_id, const uint& id,
                          const uint& dof, const uint& bndType,const vdouble& vVal);

    // Element
    // -------
    void reserveElement(const uint& mgLevel, const uint& mesh_id, const uint& num_of_element);
    void GeneElement(const uint& mgLevel, const uint& mesh_id, const uint& id, const uint& type, const vint& node_id);
    void setupElement(const uint& mgLevel, const uint& mesh_id);//numOfElementのセット


    // Bucket(in Mesh) setup
    // ---------------------
    // Node
    void initBucketNode(const uint& mgLevel, const uint& mesh_id, const uint& maxID, const uint& minID);
    void setIDBucketNode(const uint& mgLevel, const uint& mesh_id, const uint& id, const uint& index);
    void setupBucketNode(const uint& mgLevel, const uint& mesh_id);// Nodeオブジェクト設定後に一度にBucketをセットアップ
    // Element
    void initBucketElement(const uint& mgLevel, const uint& mesh_id, const uint& maxID, const uint& minID);
    void setIDBucketElement(const uint& mgLevel, const uint& mesh_id, const uint& id, const uint& index);
    void setupBucketElement(const uint& mgLevel, const uint& mesh_id);// Elementオブジェクト設定後に一度にBucketをセットアップ

    // Bucket(in AssyModel) setup
    // --------------------------
    void setupBucketMesh(const uint& mgLevel,const uint& num_of_mesh, const uint& maxID, const uint& minID);


    // Material
    // --
    void reserveMaterial(const uint& res_size);
    void GeneMaterial(const uint& material_id, string& name, vuint& vType, vdouble& vValue);

};
}
#endif