/*
 * ShaderManager.cpp
 *
 *  Created on: 2016-3-16
 *      Author: Administrator
 */
#include "ShaderManager.h"
ShaderManager::ShaderManager() {

}
ShaderManager::~ShaderManager() {

}
void ShaderManager::InitShaders() {
	PanoramaProgram =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	PanoramaProgram3DV =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	float eye = Eye.x;\n	highp vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n	texc = texc + eye * vec2(0.0,0.5);\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	//用于放大毛
	PanoramaProgramVRP =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	float eye = Eye.x;\n	highp vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n	texc = vec2( texc.x * 2.0 , texc.y );\n	texc = clamp(texc,vec2(0.0,0.0),vec2(0.5,1.0));\n	texc = texc + eye * vec2(0.5,0.0);\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n   if (all(lessThanEqual(vec2(texc.x), vec2(0.0 + Eye.x * 0.5))))\n            texc = vec2(0.25, 0.0);\n   if (all(greaterThanEqual(vec2(texc.x), vec2(0.5 + Eye.x * 0.5))))\n            texc = vec2(0.25, 0.0);\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	FadedPanoramaProgram =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n   oTexCoord = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform sampler2D Texture1;\nuniform lowp vec4 UniformColor;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	lowp vec4 staticColor = texture2D( Texture1, oTexCoord );\n	lowp vec4 movieColor = texture2D( Texture0, oTexCoord );\n	gl_FragColor = UniformColor * mix( movieColor, staticColor, staticColor.w );\n}\n");

	blackProgram =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nattribute vec4 Position;\nvoid main()\n{\ngl_Position = Mvpm * Position;\n}\n",
					"void main(){gl_FragColor = vec4(1,1,1,1);}");

}
void ShaderManager::CleanUpShaders() {
	DeleteProgram(PanoramaProgram);
	DeleteProgram(PanoramaProgram3DV);
	DeleteProgram(PanoramaProgramVRP);
	DeleteProgram(FadedPanoramaProgram);
	DeleteProgram(blackProgram);
}
GlProgram* ShaderManager::GetShaderByID(SHADER_ID id) {

	return 0;
}

