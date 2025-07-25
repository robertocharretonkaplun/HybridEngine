#include "ModelLoader.h"
#include "OBJ_Loader.h"

MeshComponent 
ModelLoader::LoadOBJModel(const std::string& filePath) {
	MeshComponent mesh;
	objl::Loader loader;

	if (loader.LoadFile(filePath)) {
		mesh.m_name = filePath;

		// Reserve capacity for the vectors
		mesh.m_vertex.reserve(loader.LoadedVertices.size());
		mesh.m_index.reserve(loader.LoadedIndices.size());

		// Load the vertices
		for (auto& vertex : loader.LoadedVertices) {
			mesh.m_vertex.emplace_back(SimpleVertex{
				{ vertex.Position.X, vertex.Position.Y, vertex.Position.Z },
				{ vertex.TextureCoordinate.X, 1.0f - vertex.TextureCoordinate.Y }
				});
		}

		// Load the indices
		for (auto index : loader.LoadedIndices) {
			mesh.m_index.push_back(index);
		}

		mesh.m_numVertex = mesh.m_vertex.size();
		mesh.m_numIndex = mesh.m_index.size();
	}

	return mesh;
}
bool
ModelLoader::InitializeFBXManager() {
	// Initialize the FBX SDK manager
	lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		ERROR("ModelLoader", "FbxManager::Create()", "Unable to create FBX Manager!");
		return false;
	}
	else {
		MESSAGE("ModelLoader", "ModelLoader", "Autodesk FBX SDK version " << lSdkManager->GetVersion())
	}

	// Create an IOSettings object
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an FBX Scene
	lScene = FbxScene::Create(lSdkManager, "MyScene");
	if (!lScene) {
		ERROR("ModelLoader", "FbxScene::Create()", "Unable to create FBX Scene!");
		return false;
	}
	else {
		MESSAGE("ModelLoader", "ModelLoader", "FBX Scene created successfully.")
	}
	return true;
}

bool
ModelLoader::LoadFBXModel(const std::string& filePath) {
	// 01. Initialize the SDK from FBX Manager
	if (InitializeFBXManager()) {
		// 02. Create an importer using the SDK manager
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
		if (!lImporter) {
			ERROR("ModelLoader", "FbxImporter::Create()", "Unable to create FBX Importer!");
			return false;
		}
		else {
			MESSAGE("ModelLoader", "ModelLoader", "FBX Importer created successfully.");
		}

		// 03. Use the first argument as the filename for the importer
		if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
			ERROR("ModelLoader", "FbxImporter::Initialize()",
				"Unable to initialize FBX Importer! Error: " << lImporter->GetStatus().GetErrorString());
			lImporter->Destroy();
			return false;
		}
		else {
			MESSAGE("ModelLoader", "ModelLoader", "FBX Importer initialized successfully.");
		}

		// 04. Import the scene from the file into the scene
		if (!lImporter->Import(lScene)) {
			ERROR("ModelLoader", "FbxImporter::Import()",
				"Unable to import FBX Scene! Error: " << lImporter->GetStatus().GetErrorString());
			lImporter->Destroy();
			return false;
		}
		else {
			MESSAGE("ModelLoader", "ModelLoader", "FBX Scene imported successfully.");
			modelName = lImporter->GetFileName();
		}

		// 05. Destroy the importer
		lImporter->Destroy();
		MESSAGE("ModelLoader", "ModelLoader", "FBX Importer destroyed successfully.");

		// 06. Process the model from the scene
		FbxNode* lRootNode = lScene->GetRootNode();

		if (lRootNode) {
			MESSAGE("ModelLoader", "ModelLoader", "Processing model from the scene root node.");
			for (int i = 0; i < lRootNode->GetChildCount(); i++) {
				ProcessFBXNode(lRootNode->GetChild(i));
			}
			return true;
		}
		else {
			ERROR("ModelLoader", "FbxScene::GetRootNode()",
				"Unable to get root node from FBX Scene!");
			return false;
		}
	}
	return false;
}

void
ModelLoader::ProcessFBXNode(FbxNode* node) {
	// 01. Process all the node's meshes
	if (node->GetNodeAttribute()) {
		if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			ProcessFBXMesh(node);
		}
	}

	// 02. Recursively process each child node
	for (int i = 0; i < node->GetChildCount(); i++) {
		ProcessFBXNode(node->GetChild(i));
	}
}

void ModelLoader::ProcessFBXMesh(FbxNode* node)
{
}

void ModelLoader::ProcessFBXMaterials(FbxSurfaceMaterial* material)
{
}
