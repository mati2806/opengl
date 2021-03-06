#include "Render.h"

void Renderer::Render( std::vector<shared_ptr<Actor>> &actors ) {

    static const GLfloat one = 1.0f;
    static const glm::vec3 clear_color = glm::vec3(0.3, 0.3, 0.3);

    glClearBufferfv (GL_COLOR, 0, &clear_color[0]);
    glClearBufferfv (GL_DEPTH, 0, &one);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);


    for ( auto a : actors ) {
        get_model (a->model_id)->render( );
    }
    glBindVertexArray( 0 );
}

void Renderer::add_model( shared_ptr<Model> model ) {
    std::cout << "Renderer added Model id " << GID << " Name " << model->name << std::endl;
    models[GID] = model;
    ++GID;
}
shared_ptr<Model> Renderer::get_model (ModelID mid) {
    if (models.empty())
    {
        std::cerr << "No models available" << std::endl;
        std::exit (EXIT_FAILURE);
    }
    if (models.find (mid) == models.end())
        return models[0];
    return models[mid];
}

void Renderer::init( ) {
    glEnable(GL_DEPTH_TEST);
}
