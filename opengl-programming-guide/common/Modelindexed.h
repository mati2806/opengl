#ifndef MODELINDEXED_H
#define MODELINDEXED_H

#include "Model.h"



class ModelIndexed : public Model
{
private:

public:
    ModelIndexed();
    void render( ) const;
    void setup_render_model( );
};

#endif // MODELINDEXED_H
