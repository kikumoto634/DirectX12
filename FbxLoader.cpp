#include "FbxLoader.h"
#include <cassert>

/// <summary>
/// 静的メンバ変数の実態
/// </summary>
const std::string FbxLoader::baseDirectory = "Resources/";

const std::string FbxLoader::defaultTextureFileName = "white1x1.png";

using namespace DirectX;

FbxLoader* FbxLoader::GetInstance()
{
    static FbxLoader instance;
    return &instance;
}

void FbxLoader::Initialize(ID3D12Device *device)
{
    //再初期化チェック
    assert(fbxManager == nullptr);

    this->device = device;

    //マネージャー生成
    fbxManager = FbxManager::Create();

    //fbxマネージャの入出力設定
    FbxIOSettings* ios = FbxIOSettings::Create(fbxManager,IOSROOT);
    fbxManager->SetIOSettings(ios);

    //FBXインポータの生成
    fbxImporter = FbxImporter::Create(fbxManager, "");
}

Model* FbxLoader::LoadModeFromFile(const string &modelName)
{
    //モデルと同じ名前にファイルから読込
    const string directoryPath = baseDirectory + modelName + "/";

    //拡張子.fbxを付加
    const string fileName = modelName + ".fbx";

    //連結したフルパスを得る
    const string fullpath = directoryPath + fileName;

    //ファイル名を指定してFBXファイルを読み込む
    if(!fbxImporter->Initialize(fullpath.c_str(), -1, fbxManager->GetIOSettings()))
    {
        assert(0);
    }

    //シーン生成
    FbxScene* fbxScene = FbxScene::Create(fbxManager,"fbxScene");

    //ファイルからロードしたFBXの情報をシーンにインポート
    fbxImporter->Import(fbxScene);

    //モデル生成
    Model*model = new Model();
    model->name = modelName;

    //FBXノードの数を取得
    int nodeCount = fbxScene->GetNodeCount();
    //あらかじめ必要数分のメモリを確保することで、アドレスがずれるのを予防
    model->nodes.reserve(nodeCount);

    //ルートノードから順に解析してモデルに流し込む
    ParseNodeRecursive(model,fbxScene->GetRootNode());
    
    //バッファ生成
    model->CreateBuffers(device);

    //Fbxシーンの解放
    fbxScene->Destroy();

    return model;
}

void FbxLoader::ParseNodeRecursive(Model *model, FbxNode *fbxNode, Node* parent)
{
    //モデルにノードを追加
    model->nodes.emplace_back();
    Node& node = model->nodes.back();

    //ノード名を取得
    string name = fbxNode->GetName();

    //FBXノードのローカル移動情報
    FbxDouble3 rotation = fbxNode->LclRotation.Get();
    FbxDouble3 scaling = fbxNode->LclScaling.Get();
    FbxDouble3 translation = fbxNode->LclTranslation.Get();

    //形式変換して代入
    node.rotation = {(float)rotation[0], (float)rotation[1], (float)rotation[2], 0.0f};
    node.scaling = {(float)scaling[0], (float)scaling[1], (float)scaling[2], 0.0f};
    node.translation = {(float)translation[0], (float)translation[1], (float)translation[2], 0.0f};

    //回転角をDegree(度)からラジアンに変換
    node.rotation.m128_f32[0] = XMConvertToRadians(node.rotation.m128_f32[0]);
    node.rotation.m128_f32[1] = XMConvertToRadians(node.rotation.m128_f32[1]);
    node.rotation.m128_f32[2] = XMConvertToRadians(node.rotation.m128_f32[2]);

    //スケール、回転、平行移動行列の計算
    XMMATRIX matScaling, matRotation, matTranslation;
    matScaling = XMMatrixScalingFromVector(node.scaling);
    matRotation = XMMatrixRotationRollPitchYawFromVector(node.rotation);
    matTranslation = XMMatrixTranslationFromVector(node.translation);

    //ローカル変形行列の計算
    node.transform = XMMatrixIdentity();
    node.transform *= matScaling;
    node.transform *= matRotation;
    node.transform *= matTranslation;

    //モデルにノードを追加(Todo)
     
    //グローバル変形行列の計算
    node.globalTransform =node.transform;
    if(parent)
    {
        node.parent= parent;
        //親の変形行列を乗算
        node.globalTransform *= parent->globalTransform;
    }
    
    //FBXノードの情報を解析してノードに記録(Todo)
    //FBXノードのメッシュ情報を解析
    FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();

    if(fbxNodeAttribute)
    {
        if(fbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            model->meshNode = &node;
            ParseMesh(model, fbxNode);
        }
    }

    //子ノードに対して再帰呼び出し
    for(int i = 0; i < fbxNode->GetChildCount(); i++)
    {
        ParseNodeRecursive(model, fbxNode->GetChild(i), &node);
    }
}

void FbxLoader::ParseMesh(Model *model, FbxNode *fbxNode)
{
    //ノードのメッシュを取得
    FbxMesh* fbxMesh = fbxNode->GetMesh();

    //頂点情報読み取り
    ParseMeshVertices(model, fbxMesh);

    //面を構成するデータの読み取り
    ParseMeshFaces(model,fbxMesh);

    //マテリアルの読み取り
    ParseMaterial(model, fbxNode);
}

void FbxLoader::ParseMeshVertices(Model *model, FbxMesh *fbxMesh)
{
    auto& vertices = model->vertices;

    //頂点データの数
    const int controlPointCount = fbxMesh->GetControlPointsCount();
    //必要数だけ頂点データ配列を確保
    Model::VertexPosNormalUv vert{};
    model->vertices.resize(controlPointCount, vert);

    //FBXメッシュの頂点座標配列を取得
    FbxVector4* pCoord = fbxMesh->GetControlPoints();

    //FBXメッシュの前兆店座標をモデル内の配列にコピーする
    for(int i = 0; i < controlPointCount; i++)
    {
        Model::VertexPosNormalUv& vertex = vertices[i];

        //座標コピー
        vertex.pos.x = (float)pCoord[i][0];
        vertex.pos.y = (float)pCoord[i][1];
        vertex.pos.z = (float)pCoord[i][2];
    }
}

void FbxLoader::ParseMeshFaces(Model *model, FbxMesh *fbxMesh)
{
    auto& vertices = model->vertices;
    auto& indices = model->indices;

    //1ファイルに複数メッシュのモデルは非対応
    assert(indices.size() == 0);
    //面の数
    const int polygonCount = fbxMesh->GetPolygonCount();
    //UVデータの数
    const int textureUVCount = fbxMesh->GetTextureUVCount();

    //UV名リスト
    FbxStringList uvNames;
    fbxMesh->GetUVSetNames(uvNames);

    //面ごとの情報読み取り
    for(int i = 0; i < polygonCount; i++)
    {
        //面を構成する頂点情報の数を取得(3なら三角形ポリゴン)
        const int polygonSize = fbxMesh->GetPolygonSize(i);
        assert(polygonSize <= 4);

        //1頂点ずつ処理
        for(int j = 0; j < polygonSize; j++)
        {
            //FBX頂点配列のインデックス
            int index = fbxMesh->GetPolygonVertex(i,j);
            assert(index >= 0);


            //頂点法線読み込み
            Model::VertexPosNormalUv& vertex = vertices[index];
            FbxVector4 normal;
            if(fbxMesh->GetPolygonVertexNormal(i,j,normal))
            {
                vertex.normal.x = (float)normal[0];
                vertex.normal.y = (float)normal[1];
                vertex.normal.z = (float)normal[2];
            }

            //テクスチャUV読み込み
            if(textureUVCount > 0)
            {
                FbxVector2 uvs;
                bool lUnmappedUV;
                //0番決め打ちで読み込み
                if(fbxMesh->GetPolygonVertexUV(i,j, uvNames[0], uvs, lUnmappedUV))
                {
                    vertex.uv.x = (float)uvs[0];
                    vertex.uv.y = (float)uvs[1];
                }
            }

            //インデックス配列に頂点インデックス追加
            //3頂点目までなら
            if(j < 3)
            {
                //1点追加し、他の2点と三角形を構築する
                indices.push_back(index);
            }
            //4頂点目
            else
            {
                //3頂点追加し、四角形の0,1,2,3の内,2,3,0で三角形を構築する
                int index2 = indices[indices.size() - 1];
                int index3 = index;
                int index0 = indices[indices.size() - 3];
                indices.push_back(index2);
                indices.push_back(index3);
                indices.push_back(index0);
            }
        }
    }
}

void FbxLoader::ParseMaterial(Model *model, FbxNode *fbxNode)
{
    const int materialCount = fbxNode->GetMaterialCount();
    if(materialCount > 0)
    {
        //先頭のマテリアルを取得
        FbxSurfaceMaterial* material = fbxNode->GetMaterial(0);
        //テクスチャを読み込んだかどうかを表すフラグ
        bool textureLoaded = false;

        if(material)
        {
            //FbxSurfaseLambertクラスかどうかを調べる
            if(material->GetClassId().Is(FbxSurfaceLambert::ClassId))
            {
                FbxSurfaceLambert* lambert = static_cast<FbxSurfaceLambert*>(material);

                //環境光係数
                FbxPropertyT<FbxDouble3> ambient = lambert->Ambient;
                model->ambient.x = (float)ambient.Get()[0];
                model->ambient.y = (float)ambient.Get()[1];
                model->ambient.z = (float)ambient.Get()[2];

                //拡散反射光係数
                FbxPropertyT<FbxDouble3> diffuse = lambert->Diffuse;
                model->diffuse.x = (float)diffuse.Get()[0];
                model->diffuse.y = (float)diffuse.Get()[1];
                model->diffuse.z = (float)diffuse.Get()[2];
            }

            //ディフューズテクスチャを取り出す
            const FbxProperty diffuseProperty = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
            if(diffuseProperty.IsValid())
            {
                const FbxFileTexture* texture = diffuseProperty.GetSrcObject<FbxFileTexture>();
                if(texture)
                {
                    const char* filepath = texture->GetFileName();
                    //ファイルパスからファイル名抽出
                    string path_str(filepath);
                    string name = ExtractFileName(path_str);
                    //テクスチャ読み込み
                    LoadTexture(model, baseDirectory + model->name + "/" + name);
                    textureLoaded = true;
                }
            }
        }
        //テクスチャがない場合白テクスチャを貼る
        if(!textureLoaded)
        {
            LoadTexture(model, baseDirectory + defaultTextureFileName);
        }
    }
}

void FbxLoader::LoadTexture(Model *model, const std::string &fullpath)
{
    HRESULT result = S_FALSE;
    //WICテクスチャのロード
    TexMetadata& metadata = model->metadata;
    ScratchImage& scratchImg = model->scratchImg;
    //ユニコード文字列に変換
    wchar_t wfilepath[128];
    MultiByteToWideChar(CP_ACP,0, fullpath.c_str(), -1, wfilepath, _countof(wfilepath));
    result = LoadFromWICFile(wfilepath, WIC_FLAGS_NONE, &metadata,scratchImg);
    if(FAILED(result))
    {
        assert(0);
    }
}

std::string FbxLoader::ExtractFileName(const std::string &path)
{
    size_t pos1;
    //区切り文字 "\\"が出てくる一番最後の部分を検索
    pos1 = path.rfind("\\");
    if(pos1 != string::npos)
    {
        return path.substr(pos1 + 1, path.size() - pos1 - 1);
    }

    //区切り文字 "/" が出てくる一番最後の部分を検索
    pos1 = path.rfind("/");
    if(pos1 != string::npos)
    {
        return path.substr(pos1+ 1, path.size() - pos1 - 1);
    }

    return path;
}

void FbxLoader::Finalize()
{
    fbxImporter->Destroy();
    fbxManager->Destroy();
}
