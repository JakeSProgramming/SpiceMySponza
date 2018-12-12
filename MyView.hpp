#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class MyView : public tygra::WindowViewDelegate
{
public:

	MyView();

	~MyView();

	void setScene(const sponza::Context * scene);

private:

	void windowViewWillStart(tygra::Window * window) override;

	void windowViewDidReset(tygra::Window * window,
		int width,
		int height) override;

	void windowViewDidStop(tygra::Window * window) override;

	void windowViewRender(tygra::Window * window) override;

	void TextureUniforms(sponza::MaterialId materialID);
	void LightUniforms();

private:

	const sponza::Context * scene_;

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	GLuint diffuseTexture{ 0 };
	GLuint specularTexture{ 0 };

	enum VertexAttribIndexes {
		vertexPosition = 0,
		vertexNormal = 1,
		texCoord = 2,
		vertexColour = 3
	};
	enum FragmentDataIndexes {
		fragmentColour = 0
	};
	enum TextureIndexes {
		diffuseTextureT = 0,
		specularTextureT = 1
	};

	struct MyTexture
	{
		GLuint textureID{ 0 };
		std::string fileLocation;
	};


	struct Mesh
	{
		GLuint VBO{ 0 };
		GLuint positionVBO{ 0 };
		GLuint normalVBO{ 0 };
		GLuint texCoordVBO{ 0 };
		GLuint elementVBO{ 0 };
		GLuint vao{ 0 };

		int elementCount{ 0 };
		int meshID;
	};

	std::vector<Mesh> meshVector;

	GLuint diffuseId;
	GLuint specularId;

	MyTexture diff0T = { 0, "diff0.png"};
	MyTexture spec0T = { 0, "spec1.png"};
	MyTexture diff1T = { 0, "diff1.png"};
	MyTexture spec1T = { 0, "spec2.png"};

};