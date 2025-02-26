#include "CZFBXLoader.h"

#include <stack>

CZFBXLoader::CZFBXLoader()
{
}

CZMesh* CZFBXLoader::LoadFBXFile(const char* filePath)
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

    // DFS through all nodes in the scene
    FbxNode* rootNode = fbxScene->GetRootNode();
    std::stack<FbxNode*> dfsStack;
    dfsStack.push(rootNode);
    while (!dfsStack.empty())
    {
        // Get the next node in the tree
        FbxNode* currentNode = dfsStack.top();
        dfsStack.pop();

        // Print the node and its attributes
        PrintNode(currentNode);
        for (int i = 0; i < currentNode->GetNodeAttributeCount(); i++)
        {
            FbxNodeAttribute* currentAttribute = currentNode->GetNodeAttributeByIndex(i);
            PrintAttribute(currentAttribute);
        }

        for (int i = 0; i < currentNode->GetChildCount(); i++)
        {
            dfsStack.push(currentNode->GetChild(i));
        }
    }

    // Destroy the SDK manager and all the other objects it was handling.
    fbxManager->Destroy();

    return nullptr;
}

void CZFBXLoader::PrintNode(FbxNode* pNode) {
    printf("\nNODE\n");

    FbxDouble3 translation = pNode->LclTranslation.Get();
    FbxDouble3 rotation = pNode->LclRotation.Get();
    FbxDouble3 scaling = pNode->LclScaling.Get();

    printf("Name = '%s'\n", pNode->GetName());
    printf("Translation = (%f, %f, %f)\n", translation[0], translation[1], translation[2]);
    printf("Rotation = (%f, %f, %f)\n", rotation[0], rotation[1], rotation[2]);
    printf("Scaling = (%f, %f, %f)\n", scaling[0], scaling[1], scaling[2]);
}

void CZFBXLoader::PrintAttribute(FbxNodeAttribute* pAttribute) {
    printf("\n\tATTRIBUTE\n");

    FbxString attributeName = pAttribute->GetName();
    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
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