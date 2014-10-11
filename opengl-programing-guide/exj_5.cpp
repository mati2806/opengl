
/*  exj_5.cpp
 * James A. Feister - thegreatpissant@gmail.com
 * DONE - Break out different model types.
 * DONE - Add a simple render system, yes it is very simple
 * DONE - Add an actor a subclass of an entity
 * PROOF - Use std library to load shaders
 * PROOF - Rendering function in renderer only
 * PROOF - Independent model movement
 * PROOF - very simple scene graph of entities to render
 * TODO - Move the shaders out of here
 * TODO - Move any other OpenGL stuff out of here.
 * Proposed exj_5 - Physics engine
 * Proposed exj_5 - Selection
 * Proposed exj_5 - Display class, Oculus and traditional
 * Proposed exj_5 - Fix input system to be more fluent
 */

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <cstdlib>

using namespace std;

//  OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

// 3rd Party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

//  Engine parts
#include "common/Shader.h"
#include "common/Render.h"
#include "common/Model.h"
#include "common/Display.h"
#include "common/Actor.h"
#include "common/Camera.h"
#include "common/Model_vbotorus.h"

enum class queue_events {
    STRAFE_LEFT,
    STRAFE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    YAW_LEFT,
    YAW_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    PITCH_UP,
    PITCH_DOWN,
    COLOR_CHANGE,
    MODEL_CHANGE,
    APPLICATION_QUIT
};

queue<queue_events> gqueue;
Display display;
Renderer renderer;
Renderer renderer1;
shared_ptr<Camera> camera;
shared_ptr<Entity> selected;
vector<shared_ptr<Actor>> scene_graph;

//  Constants and Vars
//  @@TODO Should move into a variable system
Shader vertex_shader(GL_VERTEX_SHADER), fragment_shader(GL_FRAGMENT_SHADER);
ShaderProgram diffuse_shading;
glm::mat4 Projection;
glm::mat4 MVP;
glm::mat4 camera_matrix;
glm::mat3 NormalMatrix;
glm::mat4 ModelViewMatrix;
glm::vec3 Kd = glm::vec3(0.9f, 0.5f, 0.3f);
glm::vec3 Ld = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec4 LightPosition;


//  Function Declarations
void Init( );
void UpdateView( );
void GlutIdle( );
void GlutReshape( int newWidth, int newHeight );
void GlutDisplay( void );
void GlutKeyboard( unsigned char key, int x, int y );
void UpdatePerspective( );
void CleanupAndExit( );
//  Models
void GenerateModels( );
//  Entities
void GenerateEntities( );
//  View items
void InitializeView( );
//  Shaders
void GenerateShaders( );

//  Globalized user vars
GLfloat strafe{ 1.0f }, height{ 0.0f }, depth{ -25.0f }, rotate{ 0.0f };
GLint color = 1;

float dir = 1.0f;
float xpos = 2.0f;
float ypos = 0.0f;

// MAIN //
int main( int argc, char **argv ) {
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
    glutInitWindowSize( display.screen_width, display.screen_height );

    glutCreateWindow( argv[0] );
    if ( glewInit( ) ) {
        cerr << "Unable to initialize GLEW ... exiting " << endl;
        exit( EXIT_FAILURE );
    }

    //  Initialize common systems
    renderer.init( );

    //  Load our Application Items
    GenerateModels( );
    GenerateEntities( );
    GenerateShaders( );

    //  Boiler Plate
    glutIdleFunc( GlutIdle );
    glutReshapeFunc( GlutReshape );
    glutDisplayFunc( GlutDisplay );
    glutKeyboardFunc( GlutKeyboard );

    //  Go forth and loop
    glutMainLoop( );
}

void GenerateShaders( ) {
    //  Shaders
    ShaderInfo shader[] = { { GL_VERTEX_SHADER, "./shaders/diffuse_shading.vert" },
                            { GL_FRAGMENT_SHADER, "./shaders/diffuse_shading.frag" },
                            { GL_NONE, NULL } };


    vertex_shader.SourceFile("./shaders/diffuse_shading.vert");
    fragment_shader.SourceFile("./shaders/diffuse_shading.frag");
    try {
        vertex_shader.Compile();
        fragment_shader.Compile();
    }
    catch (ShaderProgramException excp) {
        cerr << excp.what() << endl;
        exit (EXIT_FAILURE);
    }
    diffuse_shading.addShader(vertex_shader.GetHandle());
    diffuse_shading.addShader(fragment_shader.GetHandle());
    try {
        diffuse_shading.link();
    }
    catch (ShaderProgramException excp) {
        cerr << excp.what () << endl;
        exit (EXIT_FAILURE);
    }

    diffuse_shading.unuse();
}

void InitializeView( ) {
    UpdateView( );
}

void GlutReshape( int newWidth, int newHeight ) {
    display.screen_width = newWidth;
    display.screen_height = newHeight;
    UpdatePerspective( );
    glutPostRedisplay( );
}


void GlutDisplay( void ) {
    static float bounce = 0.0f;
    static bool bounce_lr = true;  //  True = left; False = right;
    const float bounce_distance = 10.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model *= glm::rotate(-35.0f, glm::vec3(1.0f,0.0f,0.0f));
    model *= glm::rotate(35.0f, glm::vec3(0.0f,1.0f,0.0f));
    ModelViewMatrix = camera_matrix *  model;
    NormalMatrix = glm::mat3 (glm::vec3( ModelViewMatrix[0]), glm::vec3( ModelViewMatrix[1]), glm::vec3( ModelViewMatrix[2]));
    MVP = Projection * ModelViewMatrix;
    if (bounce_lr) {
        bounce -= 0.1f;
        if (bounce <  (-1.0f*bounce_distance)) {
            bounce_lr = false;
        }
    } else {
        bounce += 0.1f;
        if (bounce > bounce_distance ) {
            bounce_lr = true;
        }
    }
    LightPosition = camera_matrix * glm::vec4(bounce, 0.0f, -5.0f, 1.0f);

    diffuse_shading.use();
    diffuse_shading.setUniform("NormalMatrix", NormalMatrix);
    diffuse_shading.setUniform("ProjectionMatrix", Projection);
    diffuse_shading.setUniform("ModelViewMatrix", ModelViewMatrix);
    diffuse_shading.setUniform("MVP", MVP );
    diffuse_shading.setUniform("Kd", Kd);
    diffuse_shading.setUniform("Ld", Ld);
    diffuse_shading.setUniform("LightPosition", LightPosition);
    diffuse_shading.unuse();

    renderer.render( scene_graph );
}

void GlutKeyboard( unsigned char key, int x, int y ) {
    switch ( key ) {
    default:
        break;
    case 27:
    case 'q':
    case 'Q':
        gqueue.push( queue_events::APPLICATION_QUIT );
        break;
    case 'h':
    case 'H':
        gqueue.push( queue_events::YAW_LEFT );
        break;
    case 'l':
    case 'L':
        gqueue.push( queue_events::YAW_RIGHT );
        break;
    case 'a':
    case 'A':
        gqueue.push( queue_events::STRAFE_LEFT );
        break;
    case 'd':
    case 'D':
        gqueue.push( queue_events::STRAFE_RIGHT );
        break;
    case 's':
    case 'S':
        gqueue.push( queue_events::MOVE_BACKWARD );
        break;
    case 'w':
    case 'W':
        gqueue.push( queue_events::MOVE_FORWARD );
        break;
    case 'k':
    case 'K':
        gqueue.push( queue_events::MOVE_UP );
        break;
    case '-':
        gqueue.push( queue_events::PITCH_UP );
        break;
    case '+':
        gqueue.push( queue_events::PITCH_DOWN );
        break;
    case 'j':
    case 'J':
        gqueue.push( queue_events::MOVE_DOWN );
        break;
    case 'c':
    case 'C':
        gqueue.push( queue_events::COLOR_CHANGE );
        break;
    case 'm':
    case 'M':
        gqueue.push( queue_events::MODEL_CHANGE );
        break;
    }
}

void GlutIdle( ) {
    //  Pump the events loop
    while ( !gqueue.empty( ) ) {
        switch ( gqueue.front( ) ) {
        case queue_events::MOVE_FORWARD:
            selected->state.position_z += 1.0f;
            break;
        case queue_events::MOVE_BACKWARD:
            selected->state.position_z -= 1.0f;
            break;
        case queue_events::STRAFE_RIGHT:
            selected->state.position_x -= 1.0f;
            break;
        case queue_events::STRAFE_LEFT:
            selected->state.position_x += 1.0f;
            break;
        case queue_events::YAW_RIGHT:
            selected->state.orientation_y += 0.5f;
            break;
        case queue_events::YAW_LEFT:
            selected->state.orientation_y -= 0.5f;
            break;
        case queue_events::MOVE_UP:
            selected->state.position_y += 0.5f;
            break;
        case queue_events::MOVE_DOWN:
            selected->state.position_y -= 0.5f;
            break;
        case queue_events::PITCH_UP:
            selected->state.orientation_x += 0.5f;
            break;
        case queue_events::PITCH_DOWN:
            selected->state.orientation_x += -0.5f;
            break;
        case queue_events::COLOR_CHANGE:
            color = ( color >= 4 ? 1 : color + 1 );
            break;
        case queue_events::MODEL_CHANGE:
            break;
        case queue_events::APPLICATION_QUIT:
            CleanupAndExit( );
        }
        gqueue.pop( );
    }

    UpdateView( );
    glutPostRedisplay( );
}

void CleanupAndExit( )
{
    exit( EXIT_SUCCESS );
}

void GenerateModels( ) {
    int ext = 0;
    shared_ptr<Simple_equation_model_t> tmp;
    shared_ptr<VBOTorus> tmpt;

    //  Generate Torus
    tmpt = shared_ptr<VBOTorus> { new VBOTorus (0.7f, 0.3f, 50, 50) };
    tmpt->name = "vbo_torus";
    renderer.add_model( tmpt );

    //  Generate Some equation model
    for ( auto power_to :
    { 1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.1f, 2.2f, 2.3f, 3.5f, 4.0f } ) {
        tmp =
                shared_ptr<Simple_equation_model_t>{ new Simple_equation_model_t };
        float x = 0.0f;
        float z = 0.0f;
        tmp->numVertices = 600;
        tmp->vertices.resize( tmp->numVertices * 3 );
        for ( int i = 0; i < tmp->numVertices; i++, x += 0.1f, z += 0.05f ) {
            tmp->vertices[i * 3] = x;
            tmp->vertices[i * 3 + 1] = powf( x, power_to );
            tmp->vertices[i * 3 + 2] = 0.0f; // z;
            if ( z >= -1.0f )
                z = 0.0f;
        }
        tmp->name = "ex15_" + to_string( ext++ );
        tmp->renderPrimitive = GL_POINTS;
        tmp->setup_render_model( );
        renderer.add_model( tmp );
    }

}

void GenerateEntities( ) {

    //  Camera
    camera = shared_ptr<Camera>{ new Camera( strafe, height, depth, 0.0f, 0.0f,
                                             0.0f ) };

    //  Actors
    GLfloat a = 0.0f;
    for ( int i = 0; i < 3; i++, a += 10.0f ) {
        scene_graph.push_back( shared_ptr<Actor>{ new Actor(
                                                  a, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 ) } );
    }
    //  Selected Entity
    selected = camera;
}

void UpdateView( )
{
    glm::mat4 r_matrix =
            glm::rotate( glm::mat4 (), camera->state.orientation_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    r_matrix =
            glm::rotate( r_matrix, camera->state.orientation_y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    r_matrix =
            glm::rotate( r_matrix, camera->state.orientation_z, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    glm::vec4 cr = r_matrix * glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    glm::vec3 c_pos = glm::vec3( camera->state.position_x, camera->state.position_y, camera->state.position_z );
    camera_matrix = glm::lookAt(
                c_pos,
                c_pos + glm::vec3( cr.x, cr.y, cr.z ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
}

void UpdatePerspective( ) {
    Projection = glm::perspective( 75.0f, 4.0f / 3.0f, 0.1f, 100.0f );
}