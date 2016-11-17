#pragma once

#include "NeonMath.h"
#include "ModelExporterDefines.h"
#include <vector>

class Joint;
class AnimationKeyframe;
class AnimationCurve;

class Animation
{
    public:
        static const int ANIMATION_NAME_LENGTH = 256;
        
        Animation();
        ~Animation();
        
        void GetTarget(Joint** outJoint, int* outComponent, char* outTargetName);
        void SetTarget(Joint* inJoint, int inComponent, char* inAnimationTargetName);
        void SetAnimationName(const char* inName);
        char* GetAnimationName();
        
        void AddCurve(AnimationCurve* inCurve);
        int  GetNumCurves();
        AnimationCurve* GetCurve(int inIndex);
                
    private:
        Joint*  mTargetJoint;
        int     mTargetComponent;
        char    mTargetName[NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH];
        char    mAnimationName[ANIMATION_NAME_LENGTH];
        
        std::vector<AnimationCurve*> mAnimationCurves;
};

class AnimationCurve
{
    public:
        AnimationCurve();
        ~AnimationCurve();
        
        void AddKeyframe(AnimationKeyframe* inKeyframe);
        int  GetNumKeyframes();
        AnimationKeyframe* GetKeyframe(int inIndex);

    private:
        std::vector<AnimationKeyframe*> mAnimationKeyframes;
};

class AnimationKeyframe
{
    public:
        AnimationKeyframe();
        virtual ~AnimationKeyframe();
        
        void SetTime(float inTime);
        void SetValue(float inValue);
        
        float GetTime();
        float GetValue();
        
        KeyframeType GetKeyframeType();
        
    protected:
        float                   mTime;
        float                   mValue;
        KeyframeType            mKeyframeType;
};

class BezierAnimationKeyframe : public AnimationKeyframe
{
    public:
        BezierAnimationKeyframe();
        virtual ~BezierAnimationKeyframe();
        
        void SetInTangent(float inX, float inY);
        void SetOutTangent(float inX, float inY);
        
        void GetInTangent(float* outX, float* outY);
        void GetOutTangent(float* outX, float* outY);
        
    private:
        Vector2 mInTangent;
        Vector2 mOutTangent;
};