#include "CZFBXLoader.h"

#include <stack>
#include <DirectXMath.h>

CZFBXLoader::CZFBXLoader()
{
}

CZFBXLoader::~CZFBXLoader()
{
    for (int i = 0; i < mCZObjects.size(); i++)
    {
        delete mCZObjects[i];
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

    std::vector<CZObject*> czObjects;

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
            CZObject* obj = ResolveAttribute(currentAttribute, device);
            if (obj != nullptr)
                czObjects.push_back(obj);
        }

        for (int i = 0; i < currentNode->GetChildCount(); i++)
        {
            dfsStack.push(currentNode->GetChild(i));
        }
    }

    // Destroy the SDK manager and all the other objects it was handling.
    fbxManager->Destroy();

    return czObjects;
}

CZObject* CZFBXLoader::ResolveAttribute(FbxNodeAttribute* attribute, ID3D11Device* device)
{
    switch (attribute->GetAttributeType())
    {
    case FbxNodeAttribute::eMesh:
    {
        CZMesh* mesh = LoadMesh(static_cast<FbxMesh*>(attribute), device);
        mCZObjects.push_back(mesh);
        return mesh;
    }
    default:
        return nullptr;
    }
}

CZMesh* CZFBXLoader::LoadMesh(FbxMesh* mesh, ID3D11Device* device)
{
    // Load the xyz of the vertices into a contiguous list
    std::vector<float> vertexPositions = std::vector<float>(mesh->GetControlPointsCount() * 3);
    for (int i = 0; i < mesh->GetControlPointsCount(); i++)
    {
        FbxVector4 position = mesh->GetControlPointAt(i);
        int baseIndex = i * 3;
        vertexPositions[baseIndex] = position[0];
        vertexPositions[baseIndex + 1] = position[1];
        vertexPositions[baseIndex + 2] = position[2];
    }

    // Load indices into a contiguous list
    std::vector<UINT> indices = std::vector<UINT>(mesh->GetPolygonCount() * 3);
    for (int i = 0; i < mesh->GetPolygonCount(); i++)
    {
        int baseIndex = i * 3;

        // Iterate over the three indices of the triangle
        for (int f = 0; f < 3; f++)
        {
            indices[baseIndex + f] = mesh->GetPolygonVertex(i, f);
        }
    }

    // Create and return the mesh object
    CZMesh* czMesh = new CZMesh(vertexPositions, indices, device);
    return czMesh;
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