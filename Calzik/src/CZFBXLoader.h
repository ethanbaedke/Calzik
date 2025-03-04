#pragma once

#include "CZObject.h"
#include "CZMesh.h"
#include "CZLight.h"

#include <d3d11.h>
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>

class CZFBXLoader
{
public:
	CZFBXLoader();
	~CZFBXLoader();
	std::vector<CZObject*> LoadFBXFile(const char* filePath, ID3D11Device* device);

private:
	std::vector<CZObject*> mCZMemory;

	std::vector<CZObject*> ResolveAttribute(FbxNodeAttribute* attribute, ID3D11Device* device);
	std::vector<CZObject*> LoadMesh(FbxMesh* mesh, ID3D11Device* device);
	std::vector<CZObject*> LoadLight(FbxLight* light, ID3D11Device* device);

	void PrintNode(FbxNode* node);
	void PrintAttribute(FbxNodeAttribute* attribute);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
};

namespace std {
	template <>
	struct hash<CZMesh::Vertex> {
		size_t operator()(const CZMesh::Vertex& v) const {
			return hash<float>()(v.Position.x) ^ hash<float>()(v.Position.y) ^ hash<float>()(v.Position.z) ^
				hash<float>()(v.Normal.x) ^ hash<float>()(v.Normal.y) ^ hash<float>()(v.Normal.z) ^
				hash<float>()(v.Tangent.x) ^ hash<float>()(v.Tangent.y) ^ hash<float>()(v.Tangent.z) ^
				hash<float>()(v.UV.x) ^ hash<float>()(v.UV.y);
		}
	};
}

static bool operator==(const CZMesh::Vertex& v1, const CZMesh::Vertex& v2)
{
	return v1.Position.x == v2.Position.x && v1.Position.y == v2.Position.y && v1.Position.z == v2.Position.z &&
		v1.Normal.x == v2.Normal.x && v1.Normal.y == v2.Normal.y && v1.Normal.z == v2.Normal.z &&
		v1.Tangent.x == v2.Tangent.x && v1.Tangent.y == v2.Tangent.y && v1.Tangent.z == v2.Tangent.z &&
		v1.UV.x == v2.UV.x && v1.UV.y == v2.UV.y;
}