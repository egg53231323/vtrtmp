#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

enum SHADER_ID
{
	PanoramaProgram=0,
	PanoramaProgramVRP,
	PanoramaProgram3DV,
	BlackProgram
};

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();
	void InitShaders();
	void CleanUpShaders();
	GlProgram* GetShaderByID(SHADER_ID id);

	GlProgram			PanoramaProgram;
	GlProgram			PanoramaProgramVRP;
	GlProgram			PanoramaProgram3DV;
	GlProgram			FadedPanoramaProgram;
	GlProgram			blackProgram;
};


#endif
