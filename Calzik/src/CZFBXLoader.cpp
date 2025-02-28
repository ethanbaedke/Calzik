#include "CZFBXLoader.h"

#include "CZTexture.h"

#include <stack>
#include <DirectXMath.h>
#include <filesystem>

CZFBXLoader::CZFBXLoader()
{
}

CZFBXLoader::~CZFBXLoader()
{
    for (int i = 0; i < mCZMemory.size(); i++)
    {
        delete mCZMemory[i];
    }
}

std::vector<CZObject*> CZFBXLoader::LoadFBXFile(const char* filePath, ID3D11Device* device)
{
    // Initialize the SDK manager. This object handles memory management.
    FbxManager* fbxManager = FbxManager::Create();

    // Create the IO settings object.
    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    fbxManager->SetIOSettings(fbxIOSettings);

    // Create an importer using the SDK manager.
    FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");

    // Use the first argument as the filename for the importer.
    fbxImporter->Initialize(filePath, -1, fbxManager->GetIOSettings());

    // Create a new scene so that it can be populated by the imported file.
    FbxScene* fbxScene = FbxScene::Create(fbxManager, "CubeScene");

    // Import the contents of the file into the scene.
    fbxImporter->Import(fbxScene);

    // The file is imported, so get rid of the importer.
    fbxImporter->Destroy();

    std::vector<CZObject*> newObjs;

    // DFS through all nodes in the scene
    FbxNode* rootNode = fbxScene->GetRootNode();
    std::stack<FbxNode*> dfsStack;
    dfsStack.push(rootNode);
    while (!dfsStack.empty())
    {
        // Get the next node in the tree
        FbxNode* currentNode = dfsStack.top();
        dfsStack.pop();

        // Print the node
        PrintNode(currentNode);

        // Iterate over it's attributes
        for (int i = 0; i < currentNode->GetNodeAttributeCount(); i++)
        {
            FbxNodeAttribute* currentAttribute = currentNode->GetNodeAttributeByIndex(i);

            // Print the attribute
            PrintAttribute(currentAttribute);

            // Fill out a CZObject for the attribute if it's type is supported
            std::vector<CZObject*> objs = ResolveAttribute(currentAttribute, device);
            for (int i = 0; i < objs.size(); i++)
            {
                newObjs.push_back(objs[i]);
            }
        }

        for (int i = 0; i < currentNode->GetChildCount(); i++)
        {
            dfsStack.push(currentNode->GetChild(i));
        }
    }

    // Destroy the SDK manager and all the other objects it was handling.
    fbxManager->Destroy();

    return newObjs;
}

std::vector<CZObject*> CZFBXLoader::ResolveAttribute(FbxNodeAttribute* attribute, ID3D11Device* device)
{
    switch (attribute->GetAttributeType())
    {
    case FbxNodeAttribute::eMesh:
    {
        return LoadMesh(static_cast<FbxMesh*>(attribute), device);
    }
    default:
        return std::vector<CZObject*>();
    }
}

std::vector<CZObject*> CZFBXLoader::LoadMesh(FbxMesh* mesh, ID3D11Device* device)
{
    // A map holding each unique vertex data structure and its corresponding index
    std::unordered_map<CZMesh::Vertex, UINT> uniqueVertexMap;

    // Vertex and index lists to be filled out and passed to the mesh
    std::vector<CZMesh::Vertex> vertexList;
    std::vector<UINT> indexList;

    // Get all uv sets
    FbxStringList uvSetNames;
    mesh->GetUVSetNames(uvSetNames);

    // Grab the diffuse uv set
    FbxGeometryElementUV* diffuseUVElement = mesh->GetElementUV(uvSetNames[0]);

    // Get the reference mode of the uv set
    FbxGeometryElement::EReferenceMode diffuseUVRefMode = diffuseUVElement->GetReferenceMode();

    // Iterate over every polygon
    for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++)
    {
        // Currently, we are assuming the mesh is triangulated
        for (int vertIndex = 0; vertIndex < 3; vertIndex++)
        {
            int controlPointIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);

            // Get the vertex position
            FbxVector4 pos = mesh->GetControlPointAt(controlPointIndex);

            // Get the vertex UVs
            FbxVector2 uv;
            bool unmapped;
            mesh->GetPolygonVertexUV(polyIndex, vertIndex, diffuseUVElement->GetName(), uv, unmapped);

            // Fill out the vertex data
            CZMesh::Vertex vert = {
                {static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2])},
                {static_cast<float>(uv[0]), static_cast<float>(uv[1])}
            };

            // Add the data to the map if it doesn't already exist
            if (uniqueVertexMap.count(vert) == 0)
            {
                UINT index = vertexList.size();
                uniqueVertexMap[vert] = index;
                vertexList.push_back(vert);
            }

            indexList.push_back(uniqueVertexMap[vert]);
        }
    }

    // Grab the objects material
    FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(0);

    // Get the diffuse texture name off the material
    FbxProperty diffuseProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    FbxFileTexture* texture = diffuseProp.GetSrcObject<FbxFileTexture>(0);
    std::string texName = std::filesystem::path(texture->GetFileName()).filename().string();
    std::wstring texPath = L"png/" + std::wstring(texName.begin(), texName.end());

    // Create the texture object
    CZTexture* czTex = new CZTexture(device, texPath.c_str());
    mCZMemory.push_back(czTex);

    // Create the mesh object
    CZMesh* czMesh = new CZMesh(device, vertexList, indexList, czTex);
    mCZMemory.push_back(czMesh);

    std::vector<CZObject*> newObjs = { czTex, czMesh };

    return newObjs;
}

void CZFBXLoader::PrintNode(FbxNode* node) {
    printf("\nNODE\n");

    FbxDouble3 translation = node->LclTranslation.Get();
    FbxDouble3 rotation = node->LclRotation.Get();
    FbxDouble3 scaling = node->LclScaling.Get();

    printf("Name = '%s'\n", node->GetName());
    printf("Translation = (%f, %f, %f)\n", translation[0], translation[1], translation[2]);
    printf("Rotation = (%f, %f, %f)\n", rotation[0], rotation[1], rotation[2]);
    printf("Scaling = (%f, %f, %f)\n", scaling[0], scaling[1], scaling[2]);
}

void CZFBXLoader::PrintAttribute(FbxNodeAttribute* attribute) {
    printf("\n\tATTRIBUTE\n");

    FbxString attributeName = attribute->GetName();
    FbxString typeName = GetAttributeTypeName(attribute->GetAttributeType());
    printf("\tName = '%s'\n", attributeName.Buffer());
    printf("\tType = '%s'\n", typeName.Buffer());
}

FbxString CZFBXLoader::GetAttributeTypeName(FbxNodeAttribute::EType type) {
    switch (type)
    {
    case FbxNodeAttribute::eUnknown: return "Unknown";
    case FbxNodeAttribute::eNull: return "Null";
    case FbxNodeAttribute::eMarker: return "Marker";
    case FbxNodeAttribute::eSkeleton: return "Skeleton";
    case FbxNodeAttribute::eMesh: return "Mesh";
    case FbxNodeAttribute::eNurbs: return "Nurbs";
    case FbxNodeAttribute::ePatch: return "Patch";
    case FbxNodeAttribute::eCamera: return "Camera";
    case FbxNodeAttribute::eCameraStereo: return "CameraStereo";
    case FbxNodeAttribute::eCameraSwitcher: return "CameraSwitcher";
    case FbxNodeAttribute::eLight: return "Light";
    case FbxNodeAttribute::eOpticalReference: return "OpticalReference";
    case FbxNodeAttribute::eOpticalMarker: return "OpticalMarker";
    case FbxNodeAttribute::eNurbsCurve: return "NurbsCurve";
    case FbxNodeAttribute::eTrimNurbsSurface: return "TrimNurbsSurface";
    case FbxNodeAttribute::eBoundary: return "Boundary";
    case FbxNodeAttribute::eNurbsSurface: return "NurbsSurface";
    case FbxNodeAttribute::eShape: return "Shape";
    case FbxNodeAttribute::eLODGroup: return "LODGroup";
    case FbxNodeAttribute::eSubDiv: return "SubDiv";
    default: return "TypeNotFound";
    }
}