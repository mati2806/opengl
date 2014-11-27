#include "Modelindexed.h"

ModelIndexed::ModelIndexed()
{
}

void ModelIndexed::render() const
{
    //  What Shaders do we use for this model?
    //  Based on the entity state.

    //  Bind to that shader setup and program

    glBindVertexArray( vaos[0] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->index_buffers[0] );
    glDrawElements (GL_LINES, this->indexes.size(), GL_UNSIGNED_SHORT, 0);

    //  Unbind from that shader
}

void ModelIndexed::setup_render_model()
{
    this->vaos.resize( 1 );
    glGenVertexArrays( this->vaos.size( ), &this->vaos[0] );
    if ( this->vaos[0] == 0 )
    {
        cerr << this->name << " - Did not get a valid Vertex Attribute Object" << endl;
    }
    glBindVertexArray( this->vaos[0] );

    //  Buffer Data
    this->buffers.resize( 1 );
    glGenBuffers( this->buffers.size( ), &this->buffers[0] );
    glBindBuffer( GL_ARRAY_BUFFER, this->buffers[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * this->vertices.size( ), &this->vertices[0],
                  GL_STATIC_DRAW );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET( 0 ) );
    glEnableVertexAttribArray( vPosition );
    glBindVertexArray( 0 );

    this->index_buffers.resize( 1 );
    glGenBuffers(this->index_buffers.size(), &this->index_buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*this->indexes.size(), &this->indexes[0], GL_STATIC_DRAW);

}
