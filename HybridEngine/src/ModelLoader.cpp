#include "ModelLoader.h"
#include "OBJ_Loader.h"

MeshComponent
ModelLoader::LoadOBJModel(const std::string& filePath) {
	MeshComponent mesh;
	objl::Loader loader;

	if (!loader.LoadFile(filePath)) {
		return mesh;
	}

	mesh.m_name = filePath;

	const unsigned int numVertices = loader.LoadedVertices.size();
	const unsigned int numIndices = loader.LoadedIndices.size();

	// Reservar memoria exacta para evitar reallocs
	mesh.m_vertex.resize(numVertices);
	mesh.m_index = std::move(loader.LoadedIndices);  // Mover directamente

	// Usar acceso por índice para evitar cópias extra
	for (unsigned int i = 0; i < numVertices; ++i) {
		const auto& v = loader.LoadedVertices[i];

		mesh.m_vertex[i] = SimpleVertex{
			{ v.Position.X, v.Position.Y, v.Position.Z },
			{ v.TextureCoordinate.X, 1.0f - v.TextureCoordinate.Y }
		};
	}

	mesh.m_numVertex = numVertices;
	mesh.m_numIndex = numIndices;

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

void 
ModelLoader::ProcessFBXMesh(FbxNode* node) {
	// 01. Get the mesh from the node. If there is no mesh, exit early.
	FbxMesh* mesh = node->GetMesh();
	if (!mesh) return;

	std::vector<SimpleVertex> vertices;
	std::vector<unsigned int> indices;

	// 02. Process vertices: extract positions from control points.
	for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
		SimpleVertex vertex;
		FbxVector4* controlPoint = mesh->GetControlPoints();
		vertex.Pos = XMFLOAT3((float)controlPoint[i][0],
													(float)controlPoint[i][1],
													(float)controlPoint[i][2]);
		vertices.push_back(vertex);
	}

	// 03. Process UV coordinates if available.
	if (mesh->GetElementUVCount() > 0) {
		FbxGeometryElementUV* uvElement = mesh->GetElementUV(0);
		FbxGeometryElement::EMappingMode mappingMode = uvElement->GetMappingMode();
		FbxGeometryElement::EReferenceMode referenceMode = uvElement->GetReferenceMode();
		int polyIndexCounter = 0; // Counter for polygon vertex indexing when mapping by polygon vertex.

		// 03.1 Iterate through each polygon in the mesh.
		for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
			int polySize = mesh->GetPolygonSize(polyIndex);

			// 03.1.1 Process each vertex in the polygon.
			for (int vertIndex = 0; vertIndex < polySize; vertIndex++) {
				int controlPointIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
				int uvIndex = -1;

				// 03.1.1.1 Handle UV mapping mode: by control point.
				if (mappingMode == FbxGeometryElement::eByControlPoint) {
					if (referenceMode == FbxGeometryElement::eDirect) {
						uvIndex = controlPointIndex;
					}
					else if (referenceMode == FbxGeometryElement::eIndexToDirect) {
						uvIndex = uvElement->GetIndexArray().GetAt(controlPointIndex);
					}
				}
				// 03.1.1.2 Handle UV mapping mode: by polygon vertex.
				else if (mappingMode == FbxGeometryElement::eByPolygonVertex) {
					if (referenceMode == FbxGeometryElement::eDirect || referenceMode == FbxGeometryElement::eIndexToDirect) {
						uvIndex = uvElement->GetIndexArray().GetAt(polyIndexCounter);
						polyIndexCounter++;
					}
				}

				// 03.1.1.3 If a valid UV index is found, set the texture coordinate.
				if (uvIndex != -1) {
					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
					vertices[controlPointIndex].Tex = XMFLOAT2((float)uv[0], -(float)uv[1]);
				}
			}
		}
	}

	// 04. Process indices: extract polygon vertex indices.
	for (int i = 0; i < mesh->GetPolygonCount(); i++) {
		for (int j = 0; j < mesh->GetPolygonSize(i); j++) {
			indices.push_back(mesh->GetPolygonVertex(i, j));
		}
	}

	// 05. Create a MeshComponent and populate it with the processed data.
	MeshComponent meshData;
	meshData.m_name = node->GetName();
	meshData.m_vertex = vertices;
	meshData.m_index = indices;
	meshData.m_numVertex = vertices.size();
	meshData.m_numIndex = indices.size();

	// 06. Add the processed mesh data to the collection.
	meshes.push_back(meshData);
}

void 
ModelLoader::ProcessFBXMaterials(FbxSurfaceMaterial* material) {
	if (material) {
		FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (prop.IsValid()) {
			int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			for (int i = 0; i < textureCount; ++i) {
				FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(i));
				if (texture) {
					textureFileNames.push_back(texture->GetName());
				}
			}
		}
	}
}
