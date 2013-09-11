// SphereWorld.cpp
// OpenGL SuperBible
// New and improved (performance) sphere world
// Program by Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <iostream>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLShaderManager     shaderManager;          // Shader Manager
GLMatrixStack       modelViewMatrix;        // Modelview Matrix
GLMatrixStack       projectionMatrix;       // Projection Matrix
GLFrustum           viewFrustum;            // View Frustum
GLGeometryTransform transformPipeline;      // Geometry Transform Pipeline

GLBatch     skyBoxBatch;

GLTriangleBatch     sunBatch;
GLTriangleBatch     mercuryBatch;
GLTriangleBatch     venusBatch;
GLTriangleBatch     earthBatch;
GLTriangleBatch     moonBatch;
GLTriangleBatch     marsBatch;
GLTriangleBatch     jupiterBatch;
GLTriangleBatch     saturnBatch;
GLTriangleBatch     saturnRingBatch;
GLTriangleBatch     uranusBatch;
GLTriangleBatch     uranusRingBatch;
GLTriangleBatch     neptuneBatch;
GLTriangleBatch     plutoBatch;

GLBatch             floorBatch;
GLFrame             cameraFrame;

GLuint              uiTextures[11];
GLuint              skyBoxTexture;

const float mercuryOrbitInclination = 7.0f;
const float mercuryAxialTilt = -0.027f;
const float mercuryRadius = 0.06f;
const float mercuryOrbitRadius = 0.85f;

const float venusOrbitInclination = 3.4f;
const float venusAxialTilt = -2.64f;
const float venusRadius = 0.15f;
const float venusOrbitRadius = 1.4f;

const float earthOrbitInclination = 0.00005f;
const float earthAxialTilt = -23.44f;
const float earthRadius = 0.15f;
const float earthOrbitRadius = 2.2f;

const float moonOrbitInclination = 5.145f;
const float moonAxialTilt = -5.0f;
const float moonRadius = 0.04f;
const float moonOrbitRadius = 0.2f;

const float marsOrbitInclination = 1.85f;
const float marsAxialTilt = -25.19f;
const float marsRadius = 0.1f;
const float marsOrbitRadius = 3.0f;

const float jupiterOrbitInclination = 1.305f;
const float jupiterAxialTilt = -3.12f;
const float jupiterRadius = 0.4f;
const float jupiterOrbitRadius = 4.5f;

const float saturnOrbitInclination = 2.484f;
const float saturnAxialTilt = -26.73f;
const float saturnRadius = 0.3f;
const float saturnOrbitRadius = 6.0f;
const float saturnRingInnerRadius = 0.35f;
const float saturnRingOuterRadius = 0.65f;

const float uranusOrbitInclination = 0.77f;
const float uranusAxialTilt = 97.77f;
const float uranusRadius = 0.25f;
const float uranusOrbitRadius = 7.0f;
const float uranusRingInnerRadius = 0.3f;
const float uranusRingOuterRadius = 0.4f;

const float neptuneOrbitInclination = 1.769f;
const float neptuneAxialTilt = -29.58f;
const float neptuneRadius = 0.25f;
const float neptuneOrbitRadius = 8.0f;

const float plutoOrbitInclination = 17.09f;
const float plutoAxialTilt = -119.591f;
const float plutoRadius = 0.04f;
const float plutoOrbitRadius = 9.0f;
    
    
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    // Read the texture bits
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL) 
        return false;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
                 eFormat, GL_UNSIGNED_BYTE, pBits);
    
    free(pBits);

    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
            
    return true;
}
        
//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
{
    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    gltMakeCube(skyBoxBatch, 20.0f);
    
    gltMakeSphere(sunBatch, 0.5f, 40, 20);
    gltMakeSphere(mercuryBatch, mercuryRadius, 16, 8);
    gltMakeSphere(venusBatch, venusRadius, 20, 10);
    gltMakeSphere(earthBatch, earthRadius, 20, 10);
    gltMakeSphere(moonBatch, moonRadius, 16, 8);
    gltMakeSphere(marsBatch, marsRadius, 20, 10);
    gltMakeSphere(jupiterBatch, jupiterRadius, 30, 15);
    gltMakeSphere(saturnBatch, saturnRadius, 30, 15);
    gltMakeDisk(saturnRingBatch, saturnRingInnerRadius, saturnRingOuterRadius, 30, 15);
    gltMakeSphere(uranusBatch, uranusRadius, 30, 15);
    gltMakeDisk(uranusRingBatch, uranusRingInnerRadius, uranusRingOuterRadius, 30, 15);
    gltMakeSphere(neptuneBatch, neptuneRadius, 30, 15);
    gltMakeSphere(plutoBatch, plutoRadius, 16, 8);
        
    floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0f; x <= 20.0f; x+= 0.5)
    {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
    }
    floorBatch.End();    
    
    // Make 3 texture objects
    glGenTextures(11, uiTextures);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    LoadTGATexture("img/sunmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    LoadTGATexture("img/mercurymap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    LoadTGATexture("img/venusmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[3]);
    LoadTGATexture("img/earthmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[4]);
    LoadTGATexture("img/moonmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[5]);
    LoadTGATexture("img/marsmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[6]);
    LoadTGATexture("img/jupitermap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[7]);
    LoadTGATexture("img/saturnmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[8]);
    LoadTGATexture("img/uranusmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[9]);
    LoadTGATexture("img/neptunemap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[10]);
    LoadTGATexture("img/plutomap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

    glGenTextures(1, &skyBoxTexture);
    
    glBindTexture(GL_TEXTURE_2D, skyBoxTexture);
    bool res = LoadTGATexture("img/skybox.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
}

////////////////////////////////////////////////////////////////////////
// Do shutdown for the rendering context
void ShutdownRC(void)
{
    glDeleteTextures(11, uiTextures);
    glDeleteTextures(1, &skyBoxTexture);
}


///////////////////////////////////////////////////
// Screen changes size or is initialized
void ChangeSize(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	
    // Create the projection matrix, and load it on the projection matrix stack
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // Set the transformation pipeline to use the two matrix stacks 
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

bool fullScreen = false;

void KeyDown(unsigned char key, int x, int y)
{
    if(key == 32){
        if(fullScreen) {
            glutReshapeWindow(1024, 600);
            fullScreen = false;
        }
        else {
            glutFullScreen();
            fullScreen = true;
        }
    }
}

bool leftKey = false;
bool rightKey = false;
bool upKey = false;
bool downKey = false;

void SpecialKeyDown(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
        upKey = true;
    
    if(key == GLUT_KEY_DOWN)
        downKey = true;
    
    if(key == GLUT_KEY_LEFT)
        leftKey = true;
    
    if(key == GLUT_KEY_RIGHT)
        rightKey = true;
}


void SpecialKeyUp(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
        upKey = false;
    
    if(key == GLUT_KEY_DOWN)
        downKey = false;
    
    if(key == GLUT_KEY_LEFT)
        leftKey = false;
    
    if(key == GLUT_KEY_RIGHT)
        rightKey = false;
}


void IdleFunc()
{
    float upDown = float(m3dDegToRad(1.0f));
    float leftRight = float(m3dDegToRad(3.0f));

    M3DVector3f vWorldXVect;
    M3DVector3f vLocalXVect;
    m3dLoadVector3(vLocalXVect, 1.0f, 0.0f, 0.0f); // load our up/down rotation vector
    cameraFrame.LocalToWorld(vLocalXVect, vWorldXVect, true); // transform it to world coordinates

    M3DVector3f vWorldYVect;
    M3DVector3f vLocalYVect;
    m3dLoadVector3(vLocalYVect, 0.0f, 0.0f, 1.0f); // load our left/right rotation vector
    cameraFrame.LocalToWorld(vLocalYVect, vWorldYVect, true); // transform it to world coordinates
    
    if(upKey)
        cameraFrame.RotateWorld(upDown, vWorldXVect[0], vWorldXVect[1], vWorldXVect[2]); // rotate around X axis
    
    if(downKey)
        cameraFrame.RotateWorld(-upDown, vWorldXVect[0], vWorldXVect[1], vWorldXVect[2]); // rotate around X axis
    
    if(leftKey)
        cameraFrame.RotateWorld(-leftRight, vWorldYVect[0], vWorldYVect[1], vWorldYVect[2]); // rotate around Y axis
    
    if(rightKey)
        cameraFrame.RotateWorld(leftRight, vWorldYVect[0], vWorldYVect[1], vWorldYVect[2]); // rotate around Y axis
}
        
// Called to draw scene
void RenderScene(void)
{
    // Color values
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    static GLfloat vSunColor[] = { 0.94f, 1.0f, 0.17f, 1.0f };
    static GLfloat vEarthColor[] = { 0.17f, 0.54f, 1.0f, 1.0f };
    static GLfloat vMoonColor[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    static GLfloat vLightPos[] = { 0.0f, 0.0f, -11.0f, 1.0f };

    // Time Based animation
	static CStopWatch	rotTimer;
    float sunRot = rotTimer.GetElapsedSeconds() * 35.0f;

    float mercuryOrb = sunRot * (3.5);
    float mercuryRot = sunRot * (-5.0);

    float venusOrb = sunRot * (2.0);
    float venusRot = sunRot * (4.0);

    float earthOrb = sunRot * (0.5);
    float earthRot = sunRot * (-7.0);

    float moonOrb = sunRot * (-2.0);

    float marsOrb = sunRot * (0.4);
    float marsRot = sunRot * (3.0);

    float jupiterOrb = sunRot * (0.32);
    float jupiterRot = sunRot * (-2.0);

    float saturnOrb = sunRot * (0.27);
    float saturnRot = sunRot * (3.0);

    float uranusOrb = sunRot * (0.22);
    float uranusRot = sunRot * (-4.0);

    float neptuneOrb = sunRot * (0.18);
    float neptuneRot = sunRot * (2.0);

    float plutoOrb = sunRot * (0.15);
    float plutoRot = sunRot * (1.0);

    float forward = 0.01f;
	
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Get the light position in eye space
    M3DVector4f vLightTransformed;
    M3DMatrix44f mCamera;
    modelViewMatrix.GetMatrix(mCamera);
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
	
    // Save the current modelview matrix (the identity matrix)
    modelViewMatrix.PushMatrix();   

    cameraFrame.MoveForward(forward);
    
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    // Start position
    modelViewMatrix.Translate(0.0f, 0.0f, -11.0f);
        
    // Draw the skyBox
    glBindTexture(GL_TEXTURE_2D, skyBoxTexture);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 0);
    // floorBatch.Draw();
    skyBoxBatch.Draw();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /****************************
     *           SUN            *
     ****************************/
    modelViewMatrix.PushMatrix();
    
        // Apply a rotation and draw the Sun
        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(sunRot, 0.0f, 0.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE,
                                     transformPipeline.GetModelViewProjectionMatrix(),
                                     0);
        sunBatch.Draw();
    modelViewMatrix.PopMatrix();

    /****************************
     *         MERCURY          *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(mercuryOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(mercuryOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(mercuryOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(mercuryOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(mercuryOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(mercuryAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(mercuryRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            earthBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *          VENUS           *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(venusOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(venusOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(venusOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(venusOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(venusOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(venusAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(venusRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            earthBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *          EARTH           *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(earthOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(earthOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(earthOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(earthOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(earthOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(earthAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(earthRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[3]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            earthBatch.Draw();
        modelViewMatrix.PopMatrix();

        /****************************
         *          MOON            *
         ****************************/
        modelViewMatrix.Rotate(moonOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(moonOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(moonOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(moonOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(moonOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(moonAxialTilt, 0.0f, 1.0f, 0.0f);
        glBindTexture(GL_TEXTURE_2D, uiTextures[4]);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatrix.GetMatrix(),
                                     transformPipeline.GetProjectionMatrix(),
                                     vLightTransformed, 
                                     vWhite,
                                     0);
        moonBatch.Draw();

    modelViewMatrix.PopMatrix();

    /****************************
     *          MARS            *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(marsOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(marsOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(marsOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(marsOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(marsOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(marsAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(marsRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[5]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            marsBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *         JUPITER          *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(jupiterOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(jupiterOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(jupiterOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(jupiterOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(jupiterOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(jupiterAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(jupiterRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[6]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            jupiterBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *          SATURN          *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(saturnOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(saturnOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(saturnOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(saturnOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(saturnOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(saturnAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(saturnRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[7]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            saturnBatch.Draw();
            saturnRingBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *          URANUS          *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(uranusOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(uranusOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(uranusOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(uranusOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(uranusOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(uranusAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(uranusRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[8]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            uranusBatch.Draw();
            uranusRingBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *         NEPTUNE          *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(neptuneOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(neptuneOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(neptuneOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(neptuneOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(neptuneOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(neptuneAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(neptuneRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[9]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            neptuneBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

    /****************************
     *          PLUTO           *
     ****************************/
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(plutoOrbitInclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(plutoOrb, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(plutoOrbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(plutoOrb * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(plutoOrbitInclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(plutoAxialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(plutoRot, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, uiTextures[10]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                         modelViewMatrix.GetMatrix(),
                                         transformPipeline.GetProjectionMatrix(),
                                         vLightTransformed, 
                                         vWhite,
                                         0);
            plutoBatch.Draw();
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();

	// Restore the previous modleview matrix (the identity matrix)
	// modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();    
    // Do the buffer Swap
    glutSwapBuffers();
        
    // Tell GLUT to do it again
    glutPostRedisplay();
}




int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("Solar System v0.1");
    
    glutIdleFunc(IdleFunc); 
    glutKeyboardFunc(KeyDown);
    glutSpecialFunc(SpecialKeyDown);
    glutSpecialUpFunc(SpecialKeyUp);
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}
