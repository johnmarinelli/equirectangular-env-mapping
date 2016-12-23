#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/app/Platform.h"

#include "vmath.h"

#include "object.h"
#include "shader.h"
#include "sb6ktx.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class EquirectangularApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
  
  void loadShaders();
  
  gl::GlslProgRef mGlsl;
  
  GLuint texEnvmap;
  GLuint envMaps[3];
  
  int envMapIndex;
  
  struct {
    GLint mvMatrix;
    GLint projMatrix;
  } uniforms;
  
  sb6::object object;
  CameraPersp camera;
  
  glm::mat4 mvMatrix;
};

void EquirectangularApp::setup()
{
  envMapIndex = 0;
  auto platform = ci::app::Platform::get();

  // fs::path::string() returns a local copy,
  // so if you chain .c_str() to it,
  // you get a pointer to invalid memory
  auto texPath1 = platform->getResourcePath("equirectangularmap1.ktx").string();
  auto texPath1Chars = texPath1.c_str();

  envMaps[0] = sb6::ktx::file::load(texPath1Chars);
  
  auto texPath2 = platform->getResourcePath("mountaincube.ktx").string();
  auto texPath2Chars = texPath2.c_str();
  
  envMaps[1] = sb6::ktx::file::load(texPath2Chars);
  
  texEnvmap = envMaps[1];
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  auto objPath = platform->getResourcePath("dragon.sbm").string();
  auto objPathChars = objPath.c_str();
  object.load(objPathChars);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  
  loadShaders();
}

void EquirectangularApp::loadShaders()
{
  mGlsl = gl::GlslProg::create(loadResource("render.vs.glsl"), loadResource("render.fs.glsl"));
  uniforms.mvMatrix = mGlsl->getUniformLocation("mv_matrix");
  uniforms.projMatrix = mGlsl->getUniformLocation("proj_matrix");

  //glUniform1i(glGetUniformLocation(mGlsl->getHandle(), "tex_envmap"), 1);
}

void EquirectangularApp::mouseDown( MouseEvent event )
{
  envMapIndex = (envMapIndex + 1) % 3;
  texEnvmap = envMaps[envMapIndex];
  loadShaders();
}

void EquirectangularApp::update()
{
  auto elapsed = getElapsedFrames() * 0.001f;
  camera.setPerspective(60.f, getWindowAspectRatio(), 0.1f, 1000.f);
  
  auto translateMat = glm::translate(glm::vec3{0.f, 0.f, -15.f});
  auto rotateMat = glm::rotate(elapsed, glm::vec3{1.f, 0.f, 0.f});
  auto rotateMat2 = glm::rotate(elapsed * 1.1f, glm::vec3{0.f, 1.f, 0.f});
  auto translateMat2 = glm::translate(glm::vec3{0.f, -4.f, 0.f});
  
  mvMatrix = translateMat * rotateMat * rotateMat2 * translateMat2 * glm::mat4{};
}

void EquirectangularApp::draw()
{
  static const GLfloat gray[] = { 0.0f, 0.0f, 0.0f, 1.f };
  static const GLfloat ones[] = { 1.f };
  
  glClearBufferfv(GL_COLOR, 0, gray);
  glClearBufferfv(GL_DEPTH, 0, ones);
  
  // bind current texture map
  //glBindTexture(GL_TEXTURE_2D, texEnvmap);
  
  gl::viewport(0, 0, getWindowWidth(), getWindowHeight());
  
  mGlsl->bind();
  
  glUniformMatrix4fv(uniforms.mvMatrix, 1, GL_FALSE, glm::value_ptr(mvMatrix));
  glUniformMatrix4fv(uniforms.projMatrix, 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
 
  object.render();
}

CINDER_APP( EquirectangularApp, RendererGl )
