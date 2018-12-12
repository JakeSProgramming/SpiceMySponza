#include "MyView.hpp"
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);

	GLint compile_status = GL_TRUE;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string
		= tygra::createStringFromFile("resource:///sponza_vs.glsl");
	const char * vertex_shader_code = vertex_shader_string.c_str();
	glShaderSource(vertex_shader, 1,
			(const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_string
		= tygra::createStringFromFile("resource:///sponza_fs.glsl");
	const char * fragment_shader_code = fragment_shader_string.c_str();
	glShaderSource(fragment_shader, 1,
		(const GLchar **)&fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	// Create shader program & shader in variables
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);
	glBindAttribLocation(shader_program_, vertexPosition, "vertexPosition");
	glBindAttribLocation(shader_program_, vertexNormal, "vertexNormal");
	glBindAttribLocation(shader_program_, texCoord, "texCoord");
	glDeleteShader(vertex_shader);

	glAttachShader(shader_program_, fragment_shader);
	glBindFragDataLocation(shader_program_, fragmentColour, "fragmentColour");
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program_);

	GLint link_status = GL_FALSE;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(shader_program_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	/*
		The framework provides a builder class that allows access to all the mesh data	
	*/

	sponza::GeometryBuilder builder;
	const auto& source_meshes = builder.getAllMeshes();

	// We can loop through each mesh in the scene
	for each (const sponza::Mesh& source in source_meshes)
	{
		Mesh MyMesh;

		MyMesh.meshID = source.getId();

		const auto& positions = source.getPositionArray();
		const auto& normals = source.getNormalArray();
		const auto& elements = source.getElementArray();
		const auto& texCoords = source.getTextureCoordinateArray();

		glGenBuffers(1, &MyMesh.positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.positionVBO);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &MyMesh.normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.normalVBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &MyMesh.texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &MyMesh.elementVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MyMesh.elementVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kNullId);

		MyMesh.elementCount = (int)elements.size();

		glGenVertexArrays(1, &MyMesh.vao);
		glBindVertexArray(MyMesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MyMesh.elementVBO);
		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.positionVBO);
		glEnableVertexAttribArray(vertexPosition);
		glVertexAttribPointer(vertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.normalVBO);
		glEnableVertexAttribArray(vertexNormal);
		glVertexAttribPointer(vertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

		glBindBuffer(GL_ARRAY_BUFFER, MyMesh.texCoordVBO);
		glEnableVertexAttribArray(texCoord);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

		glBindBuffer(GL_ARRAY_BUFFER, kNullId);
		glBindVertexArray(kNullId);

		meshVector.push_back(MyMesh);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	tygra::Image diff0 = tygra::createImageFromPngFile("resource:///diff0.png");
	tygra::Image diff1 = tygra::createImageFromPngFile("resource:///diff1.png");
	tygra::Image spec0 = tygra::createImageFromPngFile("resource:///spec1.png");
	tygra::Image spec1 = tygra::createImageFromPngFile("resource:///spec2.png");

	if (diff0.doesContainData())
	{
		glGenTextures(1, &diff0T.textureID);
		glBindTexture(GL_TEXTURE_2D, diff0T.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diff0.width(), diff0.height(), 0, pixel_formats[diff0.componentsPerPixel()],
			diff0.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, diff0.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, kNullId);
	}

	if (diff1.doesContainData())
	{
		glGenTextures(1, &diff1T.textureID);
		glBindTexture(GL_TEXTURE_2D, diff1T.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diff1.width(), diff1.height(), 0, pixel_formats[diff1.componentsPerPixel()],
			diff1.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, diff1.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, kNullId);
	}

	if (spec0.doesContainData())
	{
		glGenTextures(1, &spec0T.textureID);
		glBindTexture(GL_TEXTURE_2D, spec0T.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spec0.width(), spec0.height(), 0, pixel_formats[spec0.componentsPerPixel()],
			spec0.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, spec0.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, kNullId);
	}

	if (spec1.doesContainData())
	{
		glGenTextures(1, &spec1T.textureID);
		glBindTexture(GL_TEXTURE_2D, spec1T.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spec1.width(), spec1.height(), 0, pixel_formats[spec1.componentsPerPixel()],
			spec1.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, spec1.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, kNullId);
	}
	
}

void MyView::windowViewDidReset(tygra::Window * window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	glDeleteProgram(shader_program_);
	for (int i = 0; i < meshVector.size(); i++)
	{
		glDeleteBuffers(1, &meshVector[i].positionVBO);
		glDeleteBuffers(1, &meshVector[i].elementVBO);
		glDeleteVertexArrays(1, &meshVector[i].vao);
	}
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear buffers from previous frame
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program_);
	 
	// Compute viewport
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	//The code above is supplied for you and already works

	//Compute projection matrix
	float farPlaneDist = scene_->getCamera().getFarPlaneDistance();
	float nearPlaneDist = scene_->getCamera().getNearPlaneDistance();
	float fov = scene_->getCamera().getVerticalFieldOfViewInDegrees();

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspect_ratio, nearPlaneDist, farPlaneDist);

	//Compute view matrix
	const auto& cameraPosition = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& lookAtPosition = (const glm::vec3&)scene_->getCamera().getPosition() + (const glm::vec3&)scene_->getCamera().getDirection();
	const auto& worldUpPosition = (const glm::vec3&)scene_->getUpDirection();

	// Compute camera view matrix and combine with projection matrix
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition, lookAtPosition, worldUpPosition);

	//Create combined view * projection matrix and pass to shader as a uniform
	glm::mat4 combinedMatrix = projectionMatrix * viewMatrix;

	GLuint combinedMatrixID = glGetUniformLocation(shader_program_, "combinedMatrix");
	glUniformMatrix4fv(combinedMatrixID, 1, GL_FALSE, glm::value_ptr(combinedMatrix));

	glm::vec3 ambIntensity = (const glm::vec3&)scene_->getAmbientLightIntensity();
	GLuint ambIntensityID = glGetUniformLocation(shader_program_, "ambientIntensity");
	glUniform3f(ambIntensityID, ambIntensity.x, ambIntensity.y, ambIntensity.z);


	//Render each mesh
	for (const Mesh mesh : meshVector) 
	{
		const auto& instances = scene_->getInstancesByMeshId(mesh.meshID);

		for (int i = 0; i < instances.size(); i++)
		{
			sponza::Matrix4x3 testMatrix = scene_->getInstanceById(instances[i]).getTransformationMatrix();

			glm::mat4 modelMatrix = (const glm::mat4x3&) testMatrix;

			//Pass to shader as Uniform
			GLuint modelMatrixID = glGetUniformLocation(shader_program_, "modelMatrix");
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			//Get all material Values
			const auto& materialID = scene_->getInstanceById(instances.at(i)).getMaterialId();
			const auto& material = scene_->getMaterialById(materialID);
			const auto& matDiffuse = material.getDiffuseColour();
			const auto& matSpecular = material.getSpecularColour();
			const auto& matAmbient = material.getAmbientColour();
			const auto& matShiny = material.getShininess();
			
			//Pass to shader as Uniform
			GLuint matDiffId = glGetUniformLocation(shader_program_, "myMat.matDiffuse");
			glUniform3f(matDiffId, matDiffuse.x, matDiffuse.y, matDiffuse.z);

			GLuint matSpecId = glGetUniformLocation(shader_program_, "myMat.matSpecular");
			glUniform3f(matSpecId, matSpecular.x, matSpecular.y, matSpecular.z);

			GLuint matAmbId = glGetUniformLocation(shader_program_, "myMat.matAmbient");
			glUniform3f(matAmbId, matAmbient.x, matAmbient.y, matAmbient.z);

			GLuint matShinyId = glGetUniformLocation(shader_program_, "myMat.matShiny");
			glUniform1f(matShinyId, matShiny);

			LightUniforms();
			TextureUniforms(materialID);

			//Render Mesh
			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_INT, 0);
		}
	}
}

void MyView::TextureUniforms(sponza::MaterialId materialID)
{
	bool hasDiffuse = true;
	bool hasSpecular = true;

	glActiveTexture(GL_TEXTURE0 + diffuseTextureT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + specularTextureT);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::string diffName = scene_->getMaterialById(materialID).getDiffuseTexture();
	std::string specName = scene_->getMaterialById(materialID).getSpecularTexture();

	if (diffName == diff0T.fileLocation)
	{
		glActiveTexture(GL_TEXTURE0 + diffuseTextureT);
		glBindTexture(GL_TEXTURE_2D, diff0T.textureID);
		GLuint diffuseId = glGetUniformLocation(shader_program_, "diffuseTexture");
		glUniform1i(diffuseId, diffuseTextureT);

		GLuint hasDiffID = glGetUniformLocation(shader_program_, "hasDiffuse");
		glUniform1i(hasDiffID, hasDiffuse);
	}
	else if (diffName == diff1T.fileLocation)
	{
		glActiveTexture(GL_TEXTURE0 + diffuseTextureT);
		glBindTexture(GL_TEXTURE_2D, diff1T.textureID);
		GLuint diffuseId1 = glGetUniformLocation(shader_program_, "diffuseTexture");
		glUniform1i(diffuseId1, diffuseTextureT);

		GLuint hasDiffID = glGetUniformLocation(shader_program_, "hasDiffuse");
		glUniform1i(hasDiffID, hasDiffuse);
	}
	else
	{
		hasDiffuse = false;
		GLuint hasDiffID = glGetUniformLocation(shader_program_, "hasDiffuse");
		glUniform1i(hasDiffID, hasDiffuse);
	}


	if (specName == spec0T.fileLocation)
	{
		glActiveTexture(GL_TEXTURE0 + specularTextureT);
		glBindTexture(GL_TEXTURE_2D, spec0T.textureID);
		GLuint specularID = glGetUniformLocation(shader_program_, "specularTexture");
		glUniform1i(specularID, specularTextureT);

		GLuint hasSpecID = glGetUniformLocation(shader_program_, "hasSpecular");
		glUniform1i(hasSpecID, hasSpecular);
	}
	else if (specName == spec1T.fileLocation)
	{
		glActiveTexture(GL_TEXTURE0 + specularTextureT);
		glBindTexture(GL_TEXTURE_2D, spec1T.textureID);
		GLuint specularID1 = glGetUniformLocation(shader_program_, "specularTexture");
		glUniform1i(specularID1, specularTextureT);

		GLuint hasSpecID = glGetUniformLocation(shader_program_, "hasSpecular");
		glUniform1i(hasSpecID, hasSpecular);
	}
	else
	{
		hasSpecular = false;
		GLuint hasSpecID = glGetUniformLocation(shader_program_, "hasSpecular");
		glUniform1i(hasSpecID, hasSpecular);
	}
}

void MyView::LightUniforms()
{
	const auto& lights = scene_->getAllLights();

	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightPosition;
		float lightRange;
		glm::vec3 lightIntensity;

		lightPosition = glm::vec3(lights[i].getPosition().x, lights[i].getPosition().y, lights[i].getPosition().z);
		lightIntensity = glm::vec3(lights[i].getIntensity().x, lights[i].getIntensity().y, lights[i].getIntensity().z);
		lightRange = lights[i].getRange();

		GLuint lightPositionID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].lightPosition").c_str());
		glUniform3f(lightPositionID, lightPosition.x, lightPosition.y, lightPosition.z);

		GLuint lightRangeID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].lightRange").c_str());
		glUniform1f(lightRangeID, lightRange);

		GLuint lightIntensityID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].lightIntensity").c_str());
		glUniform3f(lightIntensityID, lightIntensity.x, lightIntensity.y, lightIntensity.z);

		if (i < 2 || i % 2 == 0)
		{
			GLuint coneAngleID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].coneAngle").c_str());
			glUniform1f(coneAngleID, 180.f);
		}
		else
		{
			GLuint coneAngleID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].coneAngle").c_str());
			glUniform1f(coneAngleID, 25.f);
		}

		GLuint lightDirID = glGetUniformLocation(shader_program_, ("myLights[" + std::to_string(i) + "].lightDirection").c_str());
		glUniform3f(lightDirID, 0, -1, 0);
	}

	int numLights = lights.size();
	GLuint numLightsID = glGetUniformLocation(shader_program_, "numLights");
	glUniform1i(numLightsID, numLights);
}
