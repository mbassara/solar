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


GLShaderManager     shaderManager;          // Stock Shader Manager

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

GLTriangleBatch     emptyRingBatch;

GLFrame             cameraFrame;

M3DVector4f         vLightTransformed;
M3DMatrix44f        mCamera;

GLuint              uiTextures[12];
GLuint              skyBoxTexture;

GLuint  solarShader;         // The Solar shader
GLint   locColor;           // The location of the diffuse color
GLint   locLight;           // The location of the Light in eye coordinates
GLint   locMVP;             // The location of the ModelViewProjection matrix uniform
GLint   locMV;              // The location of the ModelView matrix uniform
GLint   locNM;              // The location of the Normal matrix uniform

const float mercuryOrbitInclination = 7.0f;
const float mercuryAxialTilt = -0.027f;
const float mercuryRadius = 0.06f;
const float mercuryOrbitRadius = 0.85f;

const float venusOrbitInclination = 3.4f;
const float venusAxialTilt = -2.64f;
const float venusRadius = 0.15f;
const float venusOrbitRadius = 1.6f;

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
    shaderManager.InitializeStockShaders();

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    gltMakeCube(skyBoxBatch, 40.0f);
    
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
        
    // Make 3 texture objects
    glGenTextures(12, uiTextures);
    
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
    LoadTGATexture("img/saturnringpattern.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[9]);
    LoadTGATexture("img/uranusmap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[10]);
    LoadTGATexture("img/neptunemap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[11]);
    LoadTGATexture("img/plutomap.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

    glGenTextures(1, &skyBoxTexture);
    
    glBindTexture(GL_TEXTURE_2D, skyBoxTexture);
    LoadTGATexture("img/skybox.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);


    solarShader = gltLoadShaderPairWithAttributes("src/SolarShader.vp", "src/SolarShader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                    GLT_ATTRIBUTE_TEXTURE0, "vTexCoords", GLT_ATTRIBUTE_NORMAL, "vNormal");

    locLight = glGetUniformLocation(solarShader, "vLightPosition");
    locMVP = glGetUniformLocation(solarShader, "mvpMatrix");
    locMV  = glGetUniformLocation(solarShader, "mvMatrix");
    locNM  = glGetUniformLocation(solarShader, "normalMatrix");
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
bool speedBoost = false;
bool stop = false;

void KeyDown(unsigned char key, int x, int y)
{
    if(key == 'f'){
        if(fullScreen) {
            glutReshapeWindow(1024, 600);
            fullScreen = false;
        }
        else {
            glutFullScreen();
            fullScreen = true;
        }
    }
    else if(key == 32){
        speedBoost = true;
    }
    else if(key == 's'){
        stop = true;
    }
    else if(key == 27){
        exit(0);
    }
}

void KeyUp(unsigned char key, int x, int y)
{
    if(key == 32){
        speedBoost = false;
    }
    else if(key == 's'){
        stop = false;
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
    
    if(upKey)
        cameraFrame.RotateWorld(upDown, vWorldXVect[0], vWorldXVect[1], vWorldXVect[2]); // rotate around X axis
    
    if(downKey)
        cameraFrame.RotateWorld(-upDown, vWorldXVect[0], vWorldXVect[1], vWorldXVect[2]); // rotate around X axis

    M3DVector3f vWorldZVect;
    M3DVector3f vLocalZVect;
    m3dLoadVector3(vLocalZVect, 0.0f, 0.0f, 1.0f); // load our left/right rotation vector
    cameraFrame.LocalToWorld(vLocalZVect, vWorldZVect, true); // transform it to world coordinates
    
    if(leftKey)
        cameraFrame.RotateWorld(-leftRight, vWorldZVect[0], vWorldZVect[1], vWorldZVect[2]); // rotate around Z axis
    
    if(rightKey)
        cameraFrame.RotateWorld(leftRight, vWorldZVect[0], vWorldZVect[1], vWorldZVect[2]); // rotate around Z axis
}

void MoveForward(float distance)
{
    M3DVector3f vWorldDistVect;
    M3DVector3f vLocalDistVect;
    m3dLoadVector3(vLocalDistVect, 0.0f, 0.0f, distance); // load our left/right rotation vector
    cameraFrame.LocalToWorld(vLocalDistVect, vWorldDistVect, true); // transform it to world coordinates
    
    cameraFrame.TranslateWorld(vWorldDistVect[0], vWorldDistVect[1], vWorldDistVect[2]); // rotate around Z axis

}

void RenderPlanet(GLTriangleBatch &planetBatch, float inclination, float orbitAngularPosition, float orbitRadius,
                    float axialTilt, float rotation, GLuint texture, GLfloat* color,
                    GLTriangleBatch* planetRingBatch = &emptyRingBatch, GLuint ringTexture = -1)
{
    modelViewMatrix.PushMatrix();

        modelViewMatrix.Rotate(90.0, 1.0f, 0.0f, 0.0f);
        modelViewMatrix.Rotate(inclination, 0.0f, 1.0f, 0.0f);
        modelViewMatrix.Rotate(orbitAngularPosition, 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Translate(orbitRadius, 0.0f, 0.0f);
        modelViewMatrix.Rotate(orbitAngularPosition * (-1.0), 0.0f, 0.0f, 1.0f);
        modelViewMatrix.Rotate(inclination * (-1.0), 0.0f, 1.0f, 0.0f);

        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(axialTilt, 0.0f, 1.0f, 0.0f);
            modelViewMatrix.Rotate(rotation, 0.0f, 0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, texture);

            static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glUseProgram(solarShader);
            glUniform1i(glGetUniformLocation(solarShader, "colorMap"), 0);
            glUniform3fv(locLight, 1, vLightTransformed);
            glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
            glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
            glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

            planetBatch.Draw();
            if(planetRingBatch != &emptyRingBatch){
                if(ringTexture != -1){
                    glBindTexture(GL_TEXTURE_2D, ringTexture);
                    glUniform1i(glGetUniformLocation(solarShader, "colorMap"), 0);
                }

                planetRingBatch->Draw();
                modelViewMatrix.Translate(0.0f, 0.0f, -0.001f);
                modelViewMatrix.Rotate(180.0, 0.0f, 1.0f, 0.0f);
                glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
                glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
                glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
                planetRingBatch->Draw();
            }
        modelViewMatrix.PopMatrix();

    modelViewMatrix.PopMatrix();
}
        
// Called to draw scene
void RenderScene(void)
{
    // Color values
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

	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    modelViewMatrix.PushMatrix();   

    float dist = speedBoost ? 0.05f : 0.005f;
    dist = stop ? 0.0f : dist;
    MoveForward(dist);
    
    cameraFrame.GetCameraMatrix(mCamera);
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    // Start position
    modelViewMatrix.Translate(0.0f, 0.0f, -11.0f);
        
    // Draw the skyBox
    glBindTexture(GL_TEXTURE_2D, skyBoxTexture);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 0);
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
    
    RenderPlanet(mercuryBatch, mercuryOrbitInclination, mercuryOrb, mercuryOrbitRadius, mercuryAxialTilt, mercuryRot, uiTextures[1], vWhite);

    /****************************
     *          VENUS           *
     ****************************/
    
    RenderPlanet(venusBatch, venusOrbitInclination, venusOrb, venusOrbitRadius, venusAxialTilt, venusRot, uiTextures[2], vWhite);

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

            glUseProgram(solarShader);
            glUniform1i(glGetUniformLocation(solarShader, "colorMap"), 0);
            glUniform3fv(locLight, 1, vLightTransformed);
            glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
            glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
            glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

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

        glUniform3fv(locLight, 1, vLightTransformed);
        glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
        glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
        glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

        moonBatch.Draw();

    modelViewMatrix.PopMatrix();

    /****************************
     *          MARS            *
     ****************************/
    
    RenderPlanet(marsBatch, marsOrbitInclination, marsOrb, marsOrbitRadius, marsAxialTilt, marsRot, uiTextures[5], vWhite);

    /****************************
     *         JUPITER          *
     ****************************/
    
    RenderPlanet(jupiterBatch, jupiterOrbitInclination, jupiterOrb, jupiterOrbitRadius, jupiterAxialTilt, jupiterRot, uiTextures[6], vWhite);

    /****************************
     *          SATURN          *
     ****************************/
    
    RenderPlanet(saturnBatch, saturnOrbitInclination, saturnOrb, saturnOrbitRadius, saturnAxialTilt, saturnRot, uiTextures[7], vWhite, &saturnRingBatch, uiTextures[8]);

    /****************************
     *          URANUS          *
     ****************************/
    
    RenderPlanet(uranusBatch, uranusOrbitInclination, uranusOrb, uranusOrbitRadius, uranusAxialTilt, uranusRot, uiTextures[9], vWhite, &uranusRingBatch, uiTextures[8]);

    /****************************
     *         NEPTUNE          *
     ****************************/
    
    RenderPlanet(neptuneBatch, neptuneOrbitInclination, neptuneOrb, neptuneOrbitRadius, neptuneAxialTilt, neptuneRot, uiTextures[10], vWhite);

    /****************************
     *          PLUTO           *
     ****************************/
    
    RenderPlanet(plutoBatch, plutoOrbitInclination, plutoOrb, plutoOrbitRadius, plutoAxialTilt, plutoRot, uiTextures[11], vWhite);

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
    glutKeyboardUpFunc(KeyUp);
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
