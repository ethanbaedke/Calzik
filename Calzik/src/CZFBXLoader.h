#pragma once

#include <fbxsdk.h>

class CZFBXLoader
{
public:
	CZFBXLoader();

private:
	void PrintNode(FbxNode* pNode);
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
};