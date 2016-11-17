#pragma once

#include <vector>

class Animation;

class AnimationClip
{
    public:
        static const int ANIMATION_CLIP_NAME_LENGTH = 64;
        
        AnimationClip();
        ~AnimationClip();
        
        void  AddAnimation(Animation* inAnimation);
        void  SetAnimationClipName(const char* inName);
        char* GetAnimationClipName();
        
        int   GetNumAnimations();
        Animation* GetAnimation(int inAnimationNum);
        
        void  DumpInfo();
        
    private:
        std::vector<Animation*> mAnimationList;
        char                    mAnimationClipName[ANIMATION_CLIP_NAME_LENGTH];
};