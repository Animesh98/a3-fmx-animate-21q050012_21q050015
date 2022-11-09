#include "main.hpp"
#include "bike.hpp"
#include "rider.hpp"
#include <GLFW/glfw3.h>

#define MAX_BIKE_VBO_BYTES 1024000

GLuint shader_program, vbo, vao, uModelViewProjectMatrix_id, uNormalMatrix_id, uViewMatrix_id, position_id, color_id, normal_id, uLightSpaceMatrix_id, uModelMatrix_id, uShadowMap_id;

glm::mat4 view_matrix;
glm::mat4 ortho_matrix;
glm::mat4 projection_matrix;
glm::mat4 modelviewproject_matrix;
glm::mat4 rotation_matrix;
glm::mat3 normal_matrix;

HierarchyNode *bike, *curr_node;
std::vector<AnimationEntity> entities;
int entity_idx = 0;

void initShadersGL(void) {
    std::string vertex_shader_file("shadow_mapping_vs.glsl");
    std::string fragment_shader_file("shadow_mapping_fs.glsl");

    std::vector<GLuint> shaderList;
    shaderList.push_back(csX75::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
    shaderList.push_back(csX75::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

    shader_program = csX75::CreateProgramGL(shaderList);
    position_id = glGetAttribLocation(shader_program, "vPosition");
    /*
    color_id = glGetAttribLocation(shader_program, "vColor");
    normal_id = glGetAttribLocation(shader_program, "vNormal");
    uModelViewProjectMatrix_id = glGetUniformLocation(shader_program, "uModelViewProjectMatrix");
    uNormalMatrix_id = glGetUniformLocation(shader_program, "uNormalMatrix");
    uViewMatrix_id = glGetUniformLocation(shader_program, "uViewMatrix");
    */
    uLightSpaceMatrix_id = glGetUniformLocation(shader_program, "uLightSpaceMatrix");
}

void initVertexBufferGL(void) {
    //Ask GL for a Vertex Attribute Object (vao)
    glGenVertexArrays (1, &vao);
    //Set it as the current array to be used by binding it
    glBindVertexArray (vao);
    //Ask GL for a Vertex Buffer Object (vbo)
    glGenBuffers (1, &vbo);
    //Set it as the current buffer to be used by binding it
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
 
    glBufferData(GL_ARRAY_BUFFER, MAX_BIKE_VBO_BYTES, NULL, GL_STATIC_DRAW);
    
    std::map<std::string, GLuint> gl_info;
    gl_info["uniform_xform_id"] = uModelViewProjectMatrix_id;
    gl_info["normal_matrix_id"] = uViewMatrix_id;
    gl_info["view_matrix_id"] = uViewMatrix_id;
    gl_info["light_space_matrix_id"] = uLightSpaceMatrix_id;
    gl_info["shadow_map_id"] = uShadowMap_id;
    gl_info["shadow_light_space_matrix_id"] = uLightSpaceMatrix_id;
    bike = build_humanoid(gl_info);
    bike->prepare_vbo();
    entities.push_back(AnimationEntity("standalone_bike", bike));
    curr_node = bike;
    std::cout << "VBO successfully initialized\n";
    
    // Enable the vertex attribute
    // Excellent answer -- https://stackoverflow.com/a/39684775
    glEnableVertexAttribArray (position_id);
    glVertexAttribPointer (position_id, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(float), BUFFER_OFFSET(0));

    /*
    glEnableVertexAttribArray(color_id);
    glVertexAttribPointer(color_id, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));

    glEnableVertexAttribArray(normal_id);
    glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(float), BUFFER_OFFSET(3 * 2 * sizeof(float)));
    */
}

void renderGL(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    view_matrix = glm::lookAt(glm::vec3(0.0,0.0,VIEW_PADDING*DRAW_MIN),glm::vec3(0.0,0.0,0.0),glm::vec3(0.0,1.0,0.0));

    ortho_matrix = glm::ortho(
                       VIEW_PADDING * DRAW_MIN, VIEW_PADDING * DRAW_MAX,
                       VIEW_PADDING * DRAW_MIN, VIEW_PADDING * DRAW_MAX,
                       10 * VIEW_PADDING * DRAW_MIN, 10 * VIEW_PADDING * DRAW_MAX
                   );
    projection_matrix = glm::frustum(-1,1,-1,1,1,10);
    */
        view_matrix = glm::lookAt(glm::vec3(0.f, 0.f, 1060.f),glm::vec3(0.0,0.0,0.0),glm::vec3(0.0,1.0,0.0));
        projection_matrix = glm::ortho(
                2120.f, -2120.f,
                -2120.f, 2120.f,
                 0.f, 3180.f
        );
        ortho_matrix = projection_matrix;
    if(true) 
        modelviewproject_matrix = projection_matrix * view_matrix;
    else
        modelviewproject_matrix = ortho_matrix * view_matrix;

    rotation_matrix = glm::rotate(glm::mat4(1), xrot, glm::vec3(1, 0, 0));
    rotation_matrix = glm::rotate(rotation_matrix, yrot, glm::vec3(0, 1, 0));
    rotation_matrix = glm::rotate(rotation_matrix, zrot, glm::vec3(0, 0, 1));

    modelviewproject_matrix *= rotation_matrix;

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    viewproject = modelviewproject_matrix;
    viewmatrix = modelviewproject_matrix;
    hierarchy_matrix_stack = glm::mat4(1);
    bike->render_dag(true);
}

int main(int argc, char** argv) {
    //! The pointer to the GLFW window
    GLFWwindow* window;

    //! Setting up the GLFW Error callback
    glfwSetErrorCallback(csX75::error_callback);

    //! Initialize GLFW
    if (!glfwInit())
        return -1;

    //We want OpenGL 4.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //This is for MacOSX - can be omitted otherwise
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //We don't want the old OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //! Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "CS475/CS675 OpenGL Framework", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    //! Make the window's context current
    glfwMakeContextCurrent(window);

    //Initialize GLEW
    //Turn this on to get Shader based OpenGL
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        //Problem: glewInit failed, something is seriously wrong.
        std::cerr<<"GLEW Init Failed : %s"<<std::endl;
    }

    //Print and see what context got enabled
  	std::cout<<"Vendor: "<<glGetString (GL_VENDOR)<<std::endl;
  	std::cout<<"Renderer: "<<glGetString (GL_RENDERER)<<std::endl;
  	std::cout<<"Version: "<<glGetString (GL_VERSION)<<std::endl;
  	std::cout<<"GLSL Version: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<std::endl;

    //Keyboard Callback
    glfwSetKeyCallback(window, csX75::key_callback);
    //Framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, csX75::framebuffer_size_callback);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    //Initialize GL state
    csX75::initGL();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initShadersGL();
    initVertexBufferGL();
    
    // Loop until the user closes the window
    while (glfwWindowShouldClose(window) == 0) {

        // Render here
        renderGL();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

