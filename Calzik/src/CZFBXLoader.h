#pragma once

#include "CZObject.h"
#include "CZMesh.h"

#include <d3d11.h>
#include <fbxsdk.h>
#include <vector>

class CZFBXLoader
{
public:
	CZFBXLoader();
	~CZFBXLoader();
	std::vector<CZObject*> LoadFBXFile(const char* filePath, ID3D11Device* device);

private:
	std::vector<CZObject*> mCZObjects;

	CZObject* ResolveAttribute(FbxNodeAttribute* attribute, ID3D11Device* device);
	CZMesh* LoadMesh(FbxMesh* mesh, ID3D11Device* device);

	void PrintNode(FbxNode* node);
	void PrintAttribute(FbxNodeAttribute* attribute);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
};