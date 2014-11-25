#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "Entity.h"
#include "Model.h"
#include "Shader.h"

class Actor : public Entity {
  private:
    shared_ptr<ShaderProgram> mShader;
  public:
    ModelID model_id;
    Actor( float px, float py, float pz, float ox, float oy, float oz,
           shared_ptr<ShaderProgram> shader, ModelID mid = 0)
        : Entity( px, py, pz, ox, oy, oz),mShader{shader}, model_id{ mid } {};
    shared_ptr<ShaderProgram> getShader () const
    {
	return mShader;
    }
    void setShader ( shared_ptr<ShaderProgram> shader )
    {
	mShader = shader;
    }
};

#endif

