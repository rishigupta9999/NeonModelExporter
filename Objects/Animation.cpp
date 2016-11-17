#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Animation.h"

#pragma mark Animation

Animation::Animation()
{
    mTargetJoint = NULL;
    mTargetComponent = 0;
    mTargetName[0] = 0;
}

Animation::~Animation()
{
}

void Animation::GetTarget(Joint** outJoint, int* outComponent, char* outTargetName)
{
    *outJoint = mTargetJoint;
    *outComponent = mTargetComponent;
    strncpy(outTargetName, mTargetName, NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH);
}

void Animation::SetTarget(Joint* inJoint, int inComponent, char* inAnimationTargetName)
{
    mTargetJoint = inJoint;
    mTargetComponent = inComponent;
    strncpy(mTargetName, inAnimationTargetName, NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH);
}

char* Animation::GetAnimationName()
{
    return mAnimationName;
}

void Animation::SetAnimationName(const char* inName)
{
    if (strlen(inName) >= ANIMATION_NAME_LENGTH)
    {
        printf( "Animation name %s is longer than %d characters.  There may be unexpected consequences in the export.\n",
                inName, ANIMATION_NAME_LENGTH );
    }
    
    strncpy(mAnimationName, inName, ANIMATION_NAME_LENGTH);
}

void Animation::AddCurve(AnimationCurve* inCurve)
{
    mAnimationCurves.push_back(inCurve);
}

int Animation::GetNumCurves()
{
    return mAnimationCurves.size();
}

AnimationCurve* Animation::GetCurve(int inIndex)
{
    return mAnimationCurves.at(inIndex);
}

#pragma mark AnimationCurve

AnimationCurve::AnimationCurve()
{
}

AnimationCurve::~AnimationCurve()
{
    AnimationKeyframe* curKeyframe;
    
    int count = mAnimationKeyframes.size();
    
    while(count != 0)
    {
        curKeyframe = mAnimationKeyframes.front();
        
        mAnimationKeyframes.erase(mAnimationKeyframes.begin());
        
        delete curKeyframe;
        
        count--;
    }
}

void AnimationCurve::AddKeyframe(AnimationKeyframe* inKeyframe)
{
    mAnimationKeyframes.push_back(inKeyframe);
}

int AnimationCurve::GetNumKeyframes()
{
    return mAnimationKeyframes.size();
}

AnimationKeyframe* AnimationCurve::GetKeyframe(int inIndex)
{
    return mAnimationKeyframes.at(inIndex);
}

#pragma mark AnimationKeyframe

AnimationKeyframe::AnimationKeyframe()
{
    mTime = 0.0;
    mValue = 0.0;
}

AnimationKeyframe::~AnimationKeyframe()
{
}

void AnimationKeyframe::SetTime(float inTime)
{
    mTime = inTime;
}

void AnimationKeyframe::SetValue(float inValue)
{
    mValue = inValue;
}

float AnimationKeyframe::GetTime()
{
    return mTime;
}

float AnimationKeyframe::GetValue()
{
    return mValue;
}

KeyframeType AnimationKeyframe::GetKeyframeType()
{
    return mKeyframeType;
}

#pragma mark BezierAnimationKeyframe

BezierAnimationKeyframe::BezierAnimationKeyframe()
{
    mKeyframeType = NEON21_ANIMATION_KEYFRAME_BEZIER;
}

BezierAnimationKeyframe::~BezierAnimationKeyframe()
{
}

void BezierAnimationKeyframe::SetInTangent(float inX, float inY)
{
    mInTangent.mVector[x] = inX;
    mInTangent.mVector[y] = inY;
}

void BezierAnimationKeyframe::SetOutTangent(float inX, float inY)
{
    mOutTangent.mVector[x] = inX;
    mOutTangent.mVector[y] = inY;
}

void BezierAnimationKeyframe::GetInTangent(float* outX, float* outY)
{
    *outX = mInTangent.mVector[x];
    *outY = mInTangent.mVector[y];
}

void BezierAnimationKeyframe::GetOutTangent(float* outX, float* outY)
{
    *outX = mOutTangent.mVector[x];
    *outY = mOutTangent.mVector[y];
}
