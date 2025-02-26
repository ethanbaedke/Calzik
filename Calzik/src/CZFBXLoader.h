#pragma once
#include <fbxsdk.h>

#include "CZMesh.h"

class CZFBXLoader
{
public:
	CZFBXLoader();
	CZMesh* LoadFBXFile(const char* filePath);

private:
	void PrintNode(FbxNode* pNode);
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
};