/***************************************************************************
 *   Copyright (C) 2012 by Mathias Paulin                                  *
 *   Mathias.Paulin@irit.fr                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "rendersystem/renderer.h"

#include "gl_utils/opengl.h"
#include "fileloaders/objloader.h"
#include "fileloaders/fileloader.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

// =============================================================================
namespace RenderSystem {
// =============================================================================

void Renderer::initRessources()
{
    glAssert( glEnable(GL_DEPTH_TEST) );
    glAssert( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );

    initView();
    initGeometry();
    initShaders();
}

//------------------------------------------------------------------------------

void Renderer::initView(){
    glm::vec3 eye(1.f, 0.25, 1.f);
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    mViewMatrix = glm::lookAt(eye,center,up);
}

//------------------------------------------------------------------------------

void printShaderInfoLog(GLint shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;
    glAssert(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen));
    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glAssert(glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog));
        std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
        delete [] infoLog;
    }
}

//------------------------------------------------------------------------------

void printProgramInfoLog(GLint program)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;
    glAssert(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen));
    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glAssert(glGetProgramInfoLog(program, infoLogLen, &charsWritten, infoLog));
        std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
        delete [] infoLog;
    }
}

//------------------------------------------------------------------------------

void Renderer::initShaders(){
    char* vertexShaderSource = Loaders::Text::loadFile("../shaders/vertexdefault.glsl");
    glAssert(mVertexShaderId = glCreateShader(GL_VERTEX_SHADER));
    glAssert( glShaderSource(mVertexShaderId, 1, (const GLchar **)&vertexShaderSource, NULL) );
    glAssert(glCompileShader(mVertexShaderId));
    GLint compiled;
    glAssert(glGetShaderiv(mVertexShaderId, GL_COMPILE_STATUS, &compiled));
    if (!compiled) {
        std::cerr << " Vertex shader not compiled : " << std::endl;
        printShaderInfoLog(mVertexShaderId);
    }

    char* fragmentShaderSource;
    fragmentShaderSource = Loaders::Text::loadFile("../shaders/fragmentdefault.glsl");    
    glAssert(mFragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER));
    glAssert( glShaderSource(mFragmentShaderId, 1, (const GLchar **)&fragmentShaderSource, NULL) );
    glAssert(glCompileShader(mFragmentShaderId));
    glAssert(glGetShaderiv(mFragmentShaderId, GL_COMPILE_STATUS, &compiled));
    if (!compiled) {
        std::cerr << " fragment shader not compiled : " << std::endl;
        printShaderInfoLog(mFragmentShaderId);
    }
    
    glAssert(mProgram = glCreateProgram());
    glAssert( glAttachShader(mProgram, mVertexShaderId) );
    glAssert( glAttachShader(mProgram, mFragmentShaderId) );
    glAssert(glBindAttribLocation(mProgram, 0, "inPosition"));
    glAssert(glBindAttribLocation(mProgram, 1, "inNormal"));
    glAssert(glBindAttribLocation(mProgram, 2, "inTexCoord"));
    glAssert( glLinkProgram(mProgram) );
    GLint linked;
    glAssert(glGetProgramiv(mProgram, GL_LINK_STATUS, &linked));
    if (!linked) {
        std::cerr << "Program not linked" << std::endl;
        printProgramInfoLog(mProgram);
    }
}

//------------------------------------------------------------------------------

void Renderer::clearShaders()
{
    glAssert( glUseProgram(mProgram) );
    glAssert( glDetachShader(mProgram, mVertexShaderId  ) );
    glAssert( glDetachShader(mProgram, mFragmentShaderId) );
    glAssert( glDeleteShader(mVertexShaderId  ) );
    glAssert( glDeleteShader(mFragmentShaderId) );

    glAssert( glDeleteProgram(mProgram) );
}

//------------------------------------------------------------------------------

void Renderer::render()
{
    glAssert ( glClearColor ( 0.f, 0.3f, 0.3f, 0.f ) );
    glAssert ( glClear ( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ) );
    glm::mat4x4 localprojectionMatrix = glm::perspective( 90.f, (float)mWidth/(float)mHeight, 0.1f, 100.0f );
    glm::mat4x4 localmodelViewMatrix = mViewMatrix;
    glAssert( glUseProgram(mProgram) );

    GLint transformationLoc[4];
    glAssert( transformationLoc[0] = glGetUniformLocation(mProgram, "modelViewMatrix") );
    glAssert( transformationLoc[1] = glGetUniformLocation(mProgram, "projectionMatrix") );
    glAssert( transformationLoc[2] = glGetUniformLocation(mProgram, "normalMatrix") );
    glAssert( transformationLoc[3] = glGetUniformLocation(mProgram, "MVP") );

    GLint materialLoc[4];
    glAssert( materialLoc[0] = glGetUniformLocation(mProgram, "materialKd") );
    glAssert( materialLoc[1] = glGetUniformLocation(mProgram, "materialKs") );
    glAssert( materialLoc[2] = glGetUniformLocation(mProgram, "materialNs") );

    glm::vec3 keyLightPosition = glm::vec3(localmodelViewMatrix*glm::vec4(0.5f, 0.5f, 1.f, 1.f));
    glm::vec3 keyLightColor    = glm::vec3(0.8f, 0.2f, 0.2f); // Key light
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "keyLightPosition"), 1, glm::value_ptr(keyLightPosition) ) );
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "keyLightColor"), 1, glm::value_ptr(keyLightColor) ) );

    glm::vec3 fillLightPosition =  glm::vec3(localmodelViewMatrix*glm::vec4(1.f, 0.f, 0.f, 1.f));
    glm::vec3 fillLightColor =  glm::vec3(0.2f, 0.8f, 0.2f);
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "fillLightPosition"), 1, glm::value_ptr(fillLightPosition) ) );
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "fillLightColor"), 1, glm::value_ptr(fillLightColor) ) );

    glm::vec3 backLightPosition =  glm::vec3(localmodelViewMatrix*glm::vec4(-0.5f, 0.5f, -1.f, 1.f));
    glm::vec3 backLightColor =  glm::vec3(0.2f, 0.2f, 0.8f); // Back light
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "backLightPosition"), 1, glm::value_ptr(backLightPosition) ) );
    glAssert( glUniform3fv( glGetUniformLocation(mProgram, "backLightColor"), 1, glm::value_ptr(backLightColor) ) );

    draw_list_mesh(localmodelViewMatrix, localprojectionMatrix, transformationLoc, materialLoc);
}

//------------------------------------------------------------------------------

#define BUFFER_OFFSET(i) ((GLvoid*)(i))

/// @ingroup RenderSystem
/// A mesh with OpenGL rendering capabilities.
class MyGLMesh : public Loaders::Mesh {
private :
    GLuint mVertexArrayObject;
    enum {VBO_VERTICES, VBO_INDICES};
    GLuint mVertexBufferObjects[2];

public:
    MyGLMesh (const Loaders::Mesh &mesh)
        : Loaders::Mesh(mesh)
    {

    }

    MyGLMesh (const std::vector<float>& vertexBuffer,
              const std::vector<int>& triangleBuffer,
              bool hasNormals = true,
              bool hasTextureCoords = true )
        : Loaders::Mesh(vertexBuffer, triangleBuffer, std::vector<int>() , hasNormals, hasTextureCoords)
    {

    }

    /// Build VartexArrayObjects for the mesh.
    void compileGL() {
        
        glAssert( glGenVertexArrays(1, &mVertexArrayObject) );
        glAssert( glGenBuffers(2, mVertexBufferObjects) );
        glAssert( glBindVertexArray(mVertexArrayObject) );
        glAssert( glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObjects[VBO_VERTICES]) );
        glAssert( glBufferData(GL_ARRAY_BUFFER, mNbVertices * sizeof(Vertex),  &(mVertices[0]), GL_STATIC_DRAW) );

        GLuint stride = sizeof(Vertex);
        GLboolean normalized = GL_FALSE;
        GLenum type = GL_FLOAT;
        // position
        GLuint index = 0;
        GLint size = 3;
        GLvoid *pointer = BUFFER_OFFSET(0);
        glAssert( glVertexAttribPointer(index, size, type, normalized, stride, pointer) );
        glAssert( glEnableVertexAttribArray(index) );

        // normal
        index = 1;
        size = 3;
        pointer = BUFFER_OFFSET(sizeof(glm::vec3));
        glAssert( glVertexAttribPointer(index, size, type, normalized, stride, pointer) );
        glAssert( glEnableVertexAttribArray(index) );

        // texcoord
        index = 2;
        size = 2;
        pointer = BUFFER_OFFSET(2*sizeof(glm::vec3));
        glAssert( glVertexAttribPointer(index, size, type, normalized, stride, pointer) );
        glAssert( glEnableVertexAttribArray(index) );

        glAssert( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVertexBufferObjects[VBO_INDICES]) );
        glAssert( glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*mNbTriangles * sizeof(int),  &(mTriangles[0]), GL_STATIC_DRAW) );
        glAssert( glBindVertexArray(0) );
    }

    /// Draws the VartexArrayObjects of the mesh.
    void drawGL()
    {
        glAssert( glBindVertexArray(mVertexArrayObject) );
        glAssert( glDrawElements(GL_TRIANGLES, 3*mNbTriangles, GL_UNSIGNED_INT, NULL) );
    }
	
	virtual ~MyGLMesh()
    {
        glAssert( glBindBuffer(GL_ARRAY_BUFFER, 0) );
        glAssert( glBindVertexArray(0) );
        glAssert( glDeleteVertexArrays(1, &mVertexArrayObject) );
        glAssert( glDeleteBuffers(2, mVertexBufferObjects) );
    }

};

//------------------------------------------------------------------------------

class ParametricMesh : public Loaders::Mesh {
public:
    ParametricMesh() : Mesh() { }

    void generateMesh(int res_u, int res_v)
    {
        assert(res_u > 0); assert(res_v > 0);
        mTriangles.clear();
        mVertices.clear();

        // Sample surface
        double step_u = 1. / (double)res_u;
        double step_v = 1. / (double)res_v;
        for(int j = 0; j < (res_v+1); j++)
            for(int i = 0; i < (res_u+1); i++)
                mVertices.push_back( evalSurface( ((double)i)*step_u, ((double)j)*step_v) );

        // Compute faces
        for(int i = 0; i < res_u; i++)
        {
            for(int j = 0; j < res_v; j++)
            {
                int a = toLinear(i  , j  , res_u+1);
                int b = toLinear(i+1, j  , res_u+1);
                int c = toLinear(i+1, j+1, res_u+1);
                int d = toLinear(i  , j+1, res_u+1);

                mTriangles.push_back( TriangleIndex(a, b, c) );
                mTriangles.push_back( TriangleIndex(a, c, d) );
            }
        }

        mNbVertices  = mVertices. size();
        mNbTriangles = mTriangles.size();

        mHasNormal        = true;
        mHasTextureCoords = true;
    }

protected:
    virtual Vertex evalSurface(float u, float v) const
    {
        assert(u >= 0.f); assert(u <= 1.f);
        assert(v >= 0.f); assert(v <= 1.f);

        Vertex vert;
        vert.position = glm::vec3(u-0.5f, v-0.5f, 0.f);
        vert.normal   = glm::vec3(0.f   , 0.f   , 1.f);
        vert.texcoord = glm::vec2(u, v);
        return vert;
    }

    int toLinear(int x, int y, int width) const{
        return y*width + x;
    }
};

//------------------------------------------------------------------------------

class ParametricSphere : public ParametricMesh {
public:
    ParametricSphere() : ParametricMesh() { }

protected:
    Vertex evalSurface(float u, float v) const
    {
        assert(u >= 0.f); assert(u <= 1.f);
        assert(v >= 0.f); assert(v <= 1.f);

        Vertex vert;
        vert.texcoord = glm::vec2(u, v);

        u  = u * (2. * M_PI);
        v = v * M_PI;
        float x = cos(u)*sin(v);
        float y = sin(u)*sin(v);
        float z = cos(v);

        vert.position = glm::vec3(x, y, z);
        vert.normal   = glm::vec3(x, y, z);
        return vert;
    }
};

//------------------------------------------------------------------------------

class ParametricCylindre : public ParametricMesh {
public:
    ParametricCylindre() : ParametricMesh() { }

protected:
    Vertex evalSurface(float u, float v) const
    {
        assert(u >= 0.f); assert(u <= 1.f);
        assert(v >= 0.f); assert(v <= 1.f);

        Vertex vert;
        vert.texcoord = glm::vec2(u, v);

        u  = u * (2. * M_PI);

        float x = cos(u);
        float y = sin(u);
        float z = v;

        vert.position = glm::vec3(x, y, z  );
        vert.normal   = glm::vec3(x, y, 0.f);
        return vert;
    }
};

//------------------------------------------------------------------------------

class ParametricCone : public ParametricMesh {
public:
    ParametricCone() : ParametricMesh() { }

protected:
    Vertex evalSurface(float u, float v) const
    {
        assert(u >= 0.f); assert(u <= 1.f);
        assert(v >= 0.f); assert(v <= 1.f);

        Vertex vert;
        vert.texcoord = glm::vec2(u, v);

        u  = u * (2. * M_PI);

        float x = v*cos(u);
        float y = v*sin(u);
        float z = v;

        vert.position = glm::vec3(x, y, z  );
        vert.normal   = -glm::normalize( glm::cross(glm::vec3(cos(u), sin(u), 1.f), glm::vec3(-sin(u)*v, cos(u)*v, 0.f)));
        glm::normalize(vert.normal);
        return vert;
    }
};

//------------------------------------------------------------------------------

class MyGLMaterial {
    glm::vec3 mKd;
    glm::vec3 mKs;
    float mNs;

public:
    MyGLMaterial(glm::vec3 kd, glm::vec3 ks, float n) : mKd(kd), mKs(ks), mNs(n) {}

    void bindGL(int parameterLocation[3]){
        glAssert( glUniform3fv( parameterLocation[0], 1, glm::value_ptr(mKd)) );
        glAssert( glUniform3fv( parameterLocation[1], 1, glm::value_ptr(mKs)) );
        glAssert( glUniform1f( parameterLocation[2], mNs) );
    }
};

//------------------------------------------------------------------------------

class MyGlEntity {
    MyGLMesh* mTheMesh;
    MyGLMaterial* mTheMaterial;
    glm::mat4 mModelToSceneMatrix;
public:
    MyGlEntity(MyGLMesh* theMesh,
               MyGLMaterial* theMaterial,
               const glm::mat4& theTransformation)
        : mTheMesh(theMesh)
        , mTheMaterial(theMaterial)
        , mModelToSceneMatrix(theTransformation)
    {

    }

    void drawGL(const glm::mat4x4& sceneToViewMatrix,
                const glm::mat4x4& projectionMatrix,
                int transformLocation[4],
                int materialLocation[3])
    {
        glm::mat4x4 localprojectionMatrix = projectionMatrix;
        glm::mat4x4 localmodelViewMatrix = sceneToViewMatrix*mModelToSceneMatrix;
        glm::mat4x4 normalMatrix = glm::transpose(glm::inverse(localmodelViewMatrix));
        glm::mat4x4 MVP = localprojectionMatrix * localmodelViewMatrix;
        glAssert( glUniformMatrix4fv( transformLocation[0],  1, GL_FALSE, glm::value_ptr(localmodelViewMatrix)));
        glAssert( glUniformMatrix4fv( transformLocation[1] , 1, GL_FALSE, glm::value_ptr(localprojectionMatrix)));
        glAssert( glUniformMatrix4fv( transformLocation[2] , 1, GL_FALSE, glm::value_ptr(normalMatrix)));
        glAssert( glUniformMatrix4fv( transformLocation[3] , 1, GL_FALSE, glm::value_ptr(MVP)));
        mTheMaterial->bindGL(materialLocation);
        mTheMesh->drawGL();
    }
};

// -----------------------------------------------------------------------------

void Renderer::initGeometry()
{
    std::vector<Loaders::Mesh*> objects;
    Loaders::Obj_mtl::ObjLoader loader;
    QString reason;
    if (!loader.load("../data/camel.obj",reason))
        std::cerr << "File not loaded. Reason : " << std::endl << reason.toStdString() << std::endl;
    else
        loader.getObjects(objects);
    
    mDefaultMaterial = new MyGLMaterial(glm::vec3(0.8f, 0.8f, 0.8f), //kd
                                        glm::vec3(1.0f, 1.0f, 1.0f), //ks
                                        25.f); //n

    for (std::vector<Loaders::Mesh*>::iterator it = objects.begin(); it != objects.end(); ++it )
    {
        MyGLMesh* newObject = new MyGLMesh(*(*it));
        mMeshes.push_back( newObject );
        glm::mat4 theTransformation(1.0f);
        MyGlEntity* newEntity = new MyGlEntity(newObject, mDefaultMaterial, theTransformation);
        mEntities.push_back(newEntity);
        delete (*it);
    }

    ParametricMesh* plane    = new ParametricMesh();
    ParametricMesh* sphere   = new ParametricSphere();
    ParametricMesh* cylinder = new ParametricCylindre();
    ParametricMesh* cone     = new ParametricCone();

    plane->generateMesh(25,  50 );
    sphere->generateMesh(50,  25 );
    cylinder->generateMesh(100, 100);
    cone->generateMesh(100, 100);

    MyGLMesh* glPlane = new MyGLMesh(*plane);
    MyGLMesh* glSphere = new MyGLMesh(*sphere);
    MyGLMesh* glCylinder = new MyGLMesh(*cylinder);
    MyGLMesh* glCone = new MyGLMesh(*cone);
    mMeshes.push_back( glPlane );
    mMeshes.push_back( glSphere );
    mMeshes.push_back( glCylinder );
    mMeshes.push_back( glCone );
    delete plane;
    delete sphere;
    delete cylinder;
    delete cone;


    // Un plan
    glm::mat4 theTransformation(1.0f);
    {
        theTransformation = glm::rotate(theTransformation, -90.0f, glm::vec3(1.f,0.f,0.f));
        theTransformation = glm::scale(theTransformation, glm::vec3(10.f,10.f,1.f));
        theTransformation = glm::translate(theTransformation, glm::vec3(0.f,0.f,-0.490f));

        MyGlEntity * newEntity = new MyGlEntity(glPlane, mDefaultMaterial, theTransformation);
        mEntities.push_back(newEntity);
    }


    // Construction du repère
    theTransformation = glm::scale(glm::mat4(1.0f), glm::vec3(1.f,1.f,1.f));

    // La sphère pour noter l'Origine
    MyGLMaterial *centerMaterial = new MyGLMaterial(glm::vec3(1.f, 1.f, 0.01f), glm::vec3(1.0f, 1.0f, 1.0f), 25.f);
    glm::mat4 sphereTransformation = glm::scale(theTransformation, glm::vec3(0.1f,0.1f,0.1f));
    MyGlEntity * newEntity = new MyGlEntity(glSphere, centerMaterial, sphereTransformation);
    mEntities.push_back(newEntity);

    // axe Z
    {
        MyGLMaterial *zAxisMaterial = new MyGLMaterial(glm::vec3(0.01f, 0.01f, 1.0f), glm::vec3(1.f, 1.f, 1.f), 50.f);

        glm::mat4 cylinderTransformation = glm::scale(theTransformation, glm::vec3(0.03f, 0.03f, 0.9f));
        newEntity = new MyGlEntity(glCylinder, zAxisMaterial, cylinderTransformation);
        mEntities.push_back(newEntity);

        glm::mat4 coneTransformation = glm::rotate(theTransformation, 180.0f, glm::vec3(0.f, 1.f, 0.f));
        coneTransformation = glm::scale(coneTransformation, glm::vec3(0.06f,0.06f,0.2f));
        coneTransformation = glm::translate(coneTransformation, glm::vec3(0.f,0.f,-5.f));

        newEntity = new MyGlEntity(glCone, zAxisMaterial, coneTransformation);
        mEntities.push_back(newEntity);
    }

    // axe X
    theTransformation = glm::rotate(theTransformation, 90.0f, glm::vec3(0.f, 1.f, 0.f));
    {
        MyGLMaterial *xAxisMaterial = new MyGLMaterial(glm::vec3(1.0f, 0.01f, 0.01f), glm::vec3(1.f, 1.f, 1.f), 50.f);

        glm::mat4 cylinderTransformation = glm::scale(theTransformation, glm::vec3(0.03f, 0.03f, 0.9f));
        newEntity = new MyGlEntity(glCylinder, xAxisMaterial, cylinderTransformation);
        mEntities.push_back(newEntity);

        glm::mat4 coneTransformation = glm::rotate(theTransformation, 180.0f, glm::vec3(0.f, 1.f, 0.f));
        coneTransformation = glm::scale(coneTransformation, glm::vec3(0.06f,0.06f,0.2f));
        coneTransformation = glm::translate(coneTransformation, glm::vec3(0.f,0.f,-5.f));

        newEntity = new MyGlEntity(glCone, xAxisMaterial, coneTransformation);
        mEntities.push_back(newEntity);
    }

    // axe Y
    theTransformation = glm::rotate(theTransformation, -90.0f, glm::vec3(1.f, 0.f, 0.f));
    {
        MyGLMaterial *yAxisMaterial = new MyGLMaterial(glm::vec3(0.01f, 1.f, 0.01f), glm::vec3(1.f, 1.f, 1.f), 50.f);

        glm::mat4 cylinderTransformation = glm::scale(theTransformation, glm::vec3(0.03f, 0.03f, 0.9f));
        newEntity = new MyGlEntity(glCylinder, yAxisMaterial, cylinderTransformation);
        mEntities.push_back(newEntity);

        glm::mat4 coneTransformation = glm::rotate(theTransformation, 180.0f, glm::vec3(0.f, 1.f, 0.f));
        coneTransformation = glm::scale(coneTransformation, glm::vec3(0.06f,0.06f,0.2f));
        coneTransformation = glm::translate(coneTransformation, glm::vec3(0.f,0.f,-5.f));

        newEntity = new MyGlEntity(glCone, yAxisMaterial, coneTransformation);
        mEntities.push_back(newEntity);
    }

    for (std::vector<MyGLMesh*>::iterator it = mMeshes.begin(); it != mMeshes.end(); ++it ) {
        (*it)->compileGL();
    }

}

// -----------------------------------------------------------------------------

void Renderer::draw_list_mesh(const glm::mat4& localmodelViewMatrix,
                              const glm::mat4& localprojectionMatrix,
                              int transformationLoc[4],
                              int materialLoc[4] )
{
    for (std::vector<MyGlEntity*>::iterator it = mEntities.begin(); it != mEntities.end(); ++it ) {
        (*it)->drawGL(localmodelViewMatrix, localprojectionMatrix, transformationLoc, materialLoc);
    }
}

// -----------------------------------------------------------------------------

int Renderer::handleMouseEvent(const MouseEvent & event){
    static int modifiers = 0;
    static int x = 0;
    static int y = 0;
    static int button = 0;
    static bool moved = false;

    if (event.click) {
        x = event.x;
        y = event.y;
        button = event.button;
        modifiers = event.modifiers;
    }

    if (event.button == MouseEvent::MOVE) {
        float dx = (float)(event.x - x) / (float) mWidth;
        float dy = (float)(event.y - y) / (float) mHeight;
        switch (button) {
            case MouseEvent::LEFT : {
                glm::vec3 xvec = glm::vec3(glm::row(mViewMatrix, 0));
                glm::vec3 yvec = glm::vec3(glm::row(mViewMatrix, 1));
                mViewMatrix = glm::rotate(mViewMatrix, dy*360.f, xvec);
                mViewMatrix = glm::rotate(mViewMatrix, dx*360.f, yvec);
            }
            break;
            case MouseEvent::RIGHT : {
                glm::vec3 xvec = glm::vec3(glm::row(mViewMatrix, 0)) * dx * 10.f;
                glm::vec3 yvec = glm::vec3(glm::row(mViewMatrix, 1)) * dy * 10.f;
                glm::vec3 tvec = xvec+yvec;
                mViewMatrix = glm::translate(mViewMatrix, tvec);
            }
            break;
            case MouseEvent::MIDDLE : {
                glm::vec3 zvec = glm::vec3(glm::row(mViewMatrix, 2)) * dy * 10.f;
                mViewMatrix = glm::translate(mViewMatrix, zvec);
            }
            break;
        }
        x = event.x;
        y = event.y;
        moved = true;
        return 1;
    } else {
        moved = false;
    }

    if (event.button == MouseEvent::WHEEL) {
        return 1;
    }

    if (!event.click) {
        button = 0;
        modifiers = 0;
    }

    return 1;
}

// -----------------------------------------------------------------------------

void Renderer::setViewport(int width, int height) {
    mWidth = width;
    mHeight = height;
    glAssert( glViewport(0, 0, mWidth, mHeight) );
}

// -----------------------------------------------------------------------------

int Renderer::handleKeyEvent(char key) {

    switch (key) {
    case 'w':
        glAssert( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
    break;
    case 'f':
        glAssert( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
    break;
    }
    return 1;
}

// -----------------------------------------------------------------------------

Renderer::~Renderer()
{
    for (unsigned i = 0; i < mEntities.size(); ++i)
        delete mEntities[i];

    for (unsigned i = 0; i < mMeshes.size(); ++i)
        delete mMeshes[i];

    clearShaders();

    delete mDefaultMaterial;
}

// -----------------------------------------------------------------------------

// We need to run init glew outside the cpp of QT because GLEW headers aren't
// compatible with <QOpenGLContext>
void initGlew()
{
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // Problem: glewInit failed, something is seriously wrong.
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    glCheckError();
}

} // =============================================================================
