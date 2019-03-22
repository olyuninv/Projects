#include "CGObject.h"
namespace Lab3
{
	CGObject::CGObject()
	{
	}

	CGObject::~CGObject()
	{
	}

	void CGObject::Draw(opengl_utils glutils, GLuint programID)
	{
		int VBOindex = this->startVBO;
		int IBOindex = this->startIBO;
		for (int i = 0; i < this->Meshes.size(); i++) {

			if (programID == glutils.PhongProgramID)
			{
				glutils.linkCurrentBuffertoShader(this->VAOs[i], VBOindex, IBOindex);
			}
			else if (programID == glutils.ShaderWithTextureID)
			{
				glutils.linkCurrentBuffertoShaderReflectance(this->VAOs[i], VBOindex, IBOindex);
			}
			else if (programID == glutils.RefractionID)
			{
				glutils.linkCurrentBuffertoShaderRefraction(this->VAOs[i], VBOindex, IBOindex);
			}

			glDrawElements(GL_TRIANGLES, this->Meshes[i].Indices.size(), GL_UNSIGNED_INT, (void*)(IBOindex * sizeof(unsigned int)));
			VBOindex += Meshes[i].Vertices.size();
			IBOindex += Meshes[i].Indices.size();
		}
	}

	glm::mat4 CGObject::createTransform()
	{
		glm::mat4 localTransform = glm::mat4(1.0);
		localTransform = glm::translate(localTransform, this->position);
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.x + this->rotateAngles.x, glm::vec3(1, 0, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.y + this->rotateAngles.y, glm::vec3(0, 1, 0));
		localTransform = glm::rotate(localTransform, this->initialRotateAngle.z + this->rotateAngles.z, glm::vec3(0, 0, 1));
		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;
		return parentTransform * localTransform;
	}

	std::vector<TangentMesh> CGObject::computeTangentBasis(std::vector<objl::Mesh> Meshes)
	{
		std::vector<TangentMesh> tangentMeshes = std::vector<TangentMesh>();

		// Do computation for each mesh in the object
		for (int meshCounter = 0; meshCounter < Meshes.size(); meshCounter++)
		{
			objl::Mesh curr_mesh = Meshes[meshCounter];

			std::vector<glm::vec3> tangents = std::vector<glm::vec3>();
			std::vector<glm::vec3> bitangents = std::vector<glm::vec3>();
			tangentMeshes.push_back(TangentMesh(tangents, bitangents));

			for (int i = 0; i < curr_mesh.Indices.size(); i += 3) {

				// Shortcuts for vertices				
				glm::vec3 v0_vec = glm::vec3(curr_mesh.Vertices[curr_mesh.Indices[i + 0]].Position.X, curr_mesh.Vertices[curr_mesh.Indices[i + 0]].Position.Y, curr_mesh.Vertices[curr_mesh.Indices[i + 0]].Position.Z);
				glm::vec3 v1_vec = glm::vec3(curr_mesh.Vertices[curr_mesh.Indices[i + 1]].Position.X, curr_mesh.Vertices[curr_mesh.Indices[i + 1]].Position.Y, curr_mesh.Vertices[curr_mesh.Indices[i + 1]].Position.Z);
				glm::vec3 v2_vec = glm::vec3(curr_mesh.Vertices[curr_mesh.Indices[i + 2]].Position.X, curr_mesh.Vertices[curr_mesh.Indices[i + 2]].Position.Y, curr_mesh.Vertices[curr_mesh.Indices[i + 2]].Position.Z);
				glm::vec3 & v0 = v0_vec;
				glm::vec3 & v1 = v1_vec;
				glm::vec3 & v2 = v2_vec;

				// Shortcuts for UVs
				glm::vec2 uv0_vec = glm::vec2(curr_mesh.Vertices[curr_mesh.Indices[i + 0]].TextureCoordinate.X, curr_mesh.Vertices[curr_mesh.Indices[i + 0]].TextureCoordinate.Y);
				glm::vec2 uv1_vec = glm::vec2(curr_mesh.Vertices[curr_mesh.Indices[i + 1]].TextureCoordinate.X, curr_mesh.Vertices[curr_mesh.Indices[i + 1]].TextureCoordinate.Y);
				glm::vec2 uv2_vec = glm::vec2(curr_mesh.Vertices[curr_mesh.Indices[i + 2]].TextureCoordinate.X, curr_mesh.Vertices[curr_mesh.Indices[i + 2]].TextureCoordinate.Y);
				glm::vec2 & uv0 = uv0_vec;
				glm::vec2 & uv1 = uv1_vec;
				glm::vec2 & uv2 = uv2_vec;

				// Edges of the triangle : position delta
				glm::vec3 deltaPos1 = v1 - v0;
				glm::vec3 deltaPos2 = v2 - v0;

				// UV delta
				glm::vec2 deltaUV1 = uv1 - uv0;
				glm::vec2 deltaUV2 = uv2 - uv0;

				glm::vec3 tangent;
				glm::vec3 bitangent;

				if (glm::length(deltaUV1) > 0 && glm::length(deltaUV2) > 0)
				{
					float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
					tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
					tangent = glm::normalize(tangent);
					bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
					bitangent = glm::normalize(bitangent);
				}
				else
				{
					tangent = { 0.0, 0.0, 0.0 };
					bitangent = { 0.0, 0.0, 0.0 };
				}

				// Set the same tangent for all three vertices of the triangle.
				// They will be merged later, in vboindexer.cpp
				tangentMeshes[meshCounter].tangents.push_back(tangent);
				tangentMeshes[meshCounter].tangents.push_back(tangent);
				tangentMeshes[meshCounter].tangents.push_back(tangent);

				// Same thing for bitangents
				tangentMeshes[meshCounter].bitangents.push_back(bitangent);
				tangentMeshes[meshCounter].bitangents.push_back(bitangent);
				tangentMeshes[meshCounter].bitangents.push_back(bitangent);
			}
		}

		return tangentMeshes;
	}

	/// This function is copied from: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
	bool CGObject::is_near(float v1, float v2) {
		return fabs(v1 - v2) < 0.001f;
	}

	/// This function is based on code from: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
	bool CGObject::getSimilarVertexIndex(
		objl::Vertex & in_vertex,
		std::vector<objl::Vertex> & out_vertices,
		unsigned short & result
	) {

		// Lame linear search
		for (unsigned int i = 0; i < out_vertices.size(); i++) {
			if (
				is_near(in_vertex.Position.X, out_vertices[i].Position.X) &&
				is_near(in_vertex.Position.Y, out_vertices[i].Position.Y) &&
				is_near(in_vertex.Position.Z, out_vertices[i].Position.Z) &&
				is_near(in_vertex.TextureCoordinate.X, out_vertices[i].TextureCoordinate.X) &&
				is_near(in_vertex.TextureCoordinate.Y, out_vertices[i].TextureCoordinate.Y) &&
				is_near(in_vertex.Normal.X, out_vertices[i].Normal.X) &&
				is_near(in_vertex.Normal.Y, out_vertices[i].Normal.Y) &&
				is_near(in_vertex.Normal.Z, out_vertices[i].Normal.Z)
				) {
				result = i;
				return true;
			}
		}
		// No other vertex could be used instead.
		// Looks like we'll have to add it to the VBO.
		return false;
	}

	/// This function is based on code from: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
	void CGObject::recalculateVerticesAndIndexes(std::vector<objl::Mesh> Meshes,
		std::vector<objl::Mesh> &new_meshes,
		std::vector<TangentMesh> &new_tangentMeshes)
	{
		std::vector<TangentMesh> currTangentMeshes = computeTangentBasis(Meshes);
		std::vector<objl::Mesh> newMeshesObject = std::vector<objl::Mesh>();
		std::vector<TangentMesh> newTangentMeshesObject = std::vector<TangentMesh>();

		// Do computation for each mesh in the object
		for (int meshCounter = 0; meshCounter < Meshes.size(); meshCounter++)
		{
			objl::Mesh curr_mesh = Meshes[meshCounter];

			// Create new mesh - we will overwrite the existing mesh with it
			std::vector <objl::Vertex> newVertices = std::vector <objl::Vertex>();
			std::vector <GLuint> newIndices = std::vector <GLuint>();
			objl::Mesh new_mesh = objl::Mesh(newVertices, newIndices);

			// Create new tangent mesh - we will overwrite the existing mesh with it
			std::vector<glm::vec3> newTangents = std::vector<glm::vec3>();
			std::vector<glm::vec3> newBitangents = std::vector<glm::vec3>();
			TangentMesh new_tangentMesh = TangentMesh(newTangents, newBitangents);

			// For each input vertex
			for (unsigned int i = 0; i < curr_mesh.Indices.size(); i++) {

				// Try to find a similar vertex in out_XXXX
				unsigned short index;
				bool found = getSimilarVertexIndex(curr_mesh.Vertices[curr_mesh.Indices[i]], new_mesh.Vertices, index);

				if (found) { // A similar vertex is already in the VBO, use it instead !
					new_mesh.Indices.push_back(index);

					// Average the tangents and the bitangents
					new_tangentMesh.tangents[index] += currTangentMeshes[meshCounter].tangents[i];
					new_tangentMesh.bitangents[index] += currTangentMeshes[meshCounter].bitangents[i];
				}
				else { // If not, it needs to be added in the output data.
					new_mesh.Vertices.push_back(objl::Vertex(Meshes[meshCounter].Vertices[curr_mesh.Indices[i]]));
					new_tangentMesh.tangents.push_back(currTangentMeshes[meshCounter].tangents[i]);
					new_tangentMesh.bitangents.push_back(currTangentMeshes[meshCounter].bitangents[i]);
					new_mesh.Indices.push_back((unsigned short)new_mesh.Vertices.size() - 1);
				}
			}

			newMeshesObject.push_back(new_mesh);
			newTangentMeshesObject.push_back(new_tangentMesh);
		}

		new_meshes = newMeshesObject;
		new_tangentMeshes = newTangentMeshesObject;
	}

	TangentMesh::TangentMesh(std::vector<glm::vec3>& _tangents, std::vector<glm::vec3>& _bitangents)
	{
		this->tangents = _tangents;
		this->bitangents = _bitangents;
	}

	TangentMesh::~TangentMesh()
	{
	}

}