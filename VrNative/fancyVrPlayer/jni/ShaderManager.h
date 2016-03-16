#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "GlProgram.h"

namespace OVR
{
	class GlProgram;
}
enum SHADER_ID
{
	PanoramaProgram=0,
	PanoramaProgramVRP,
	PanoramaProgram3DV,
	BlackProgram,


	SHADER_ID_COUNT
};

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();
	void InitShaders();
	void CleanUpShaders();
	OVR::GlProgram* GetShaderByID(SHADER_ID id);

	OVR::GlProgram m_Program[SHADER_ID_COUNT];

};


#endif
