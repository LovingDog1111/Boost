#pragma once
#include <glm/glm.hpp>
#include "../Core/GLMatrix.h"
#include "../GlobalInstance.h"
class Matrix {
   public:
   static glm::mat4 getMatrixCorrection(GLMatrix mat) {
        glm::mat4 toReturn;

        for(int i = 0; i < 4; i++) {
            toReturn[i][0] = mat.matrix[0 + i];
            toReturn[i][1] = mat.matrix[4 + i];
            toReturn[i][2] = mat.matrix[8 + i];
            toReturn[i][3] = mat.matrix[12 + i];
        }

        return toReturn;
    }
   static bool WorldToScreen(
       Vec3<float> pos,
                       Vec2<float> &screen) {  
       if (!GI::getLocalPlayer()) {  // –ﬁ∏¥£∫”¶∏√ «!GI::getLocalPlayer()
           return false;
        }
        if(!GI::getLevelRenderer())
           return false;
        Vec2<float> displaySize = GI::getGuiData()->windowSizeReal;
        Vec3<float> origin = GI::getLevelRenderer()->renderplayer->origin;

        pos = pos.sub(origin);

        glm::mat4x4 matx = Matrix::getMatrixCorrection(GI::getClientInstance()->getglMatrix());

        glm::vec4 transformedPos = matx * glm::vec4(pos.x, pos.y, pos.z, 1.0f);

        if(transformedPos.z > 0)
            return false;

        float mX = displaySize.x / 2.0f;
        float mY = displaySize.y / 2.0f;

        screen.x = mX + (mX * transformedPos.x / -transformedPos.z *
                         GI::getLevelRenderer()->getrenderplayer()->getFovX());
        screen.y = mY - (mY * transformedPos.y / -transformedPos.z *
                         GI::getLevelRenderer()->getrenderplayer()->getFovY());

        return true;
    }
};