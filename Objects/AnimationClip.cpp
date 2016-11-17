#include "AnimationClip.h"
#include "Animation.h"
#include "Skeleton.h"
#include "Logging.h"

#include <assert.h>

AnimationClip::AnimationClip()
{
    memset(mAnimationClipName, 0, sizeof(mAnimationClipName));
}

AnimationClip::~AnimationClip()
{
    Animation* curAnimation;
    
    int count = mAnimationList.size();
    
    while(count != 0)
    {
        curAnimation = mAnimationList.front();
        
        mAnimationList.erase(mAnimationList.begin());
        
        delete curAnimation;
        
        count--;
    }
}

void AnimationClip::AddAnimation(Animation* inAnimation)
{
    mAnimationList.push_back(inAnimation);
}

void AnimationClip::SetAnimationClipName(const char* inName)
{
    if (strlen(inName) >= ANIMATION_CLIP_NAME_LENGTH)
    {
        NeonWarning("Animation clip name is longer than %d characters.  There may be errors in the export.\n", ANIMATION_CLIP_NAME_LENGTH);
    }
    
    strncpy(mAnimationClipName, inName, ANIMATION_CLIP_NAME_LENGTH);
}

char* AnimationClip::GetAnimationClipName()
{
    return mAnimationClipName;
}

int AnimationClip::GetNumAnimations()
{
    return mAnimationList.size();
}

Animation* AnimationClip::GetAnimation(int inAnimationNum)
{
    return mAnimationList.at(inAnimationNum);
}

void AnimationClip::DumpInfo()
{
    NeonMessage("Animation Clip name is %s\n", mAnimationClipName);
    
    int numAnimations = (int)mAnimationList.size();
    NeonMessage("Number of animations is %d\n", numAnimations);
    
    for (int curAnimationIndex = 0; curAnimationIndex < numAnimations; curAnimationIndex++)
    {
        Animation* curAnimation = mAnimationList.at(curAnimationIndex);
        
        NeonMessage("\tAnimation %d name is %s\n", curAnimationIndex, curAnimation->GetAnimationName());
        
        Joint* targetJoint;
        int targetComponent;
        char targetName[NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH];
        
        curAnimation->GetTarget(&targetJoint, &targetComponent, targetName);
        
        NeonMessage("\t\tTarget joint is %s\n", targetJoint->GetJointName());
        NeonMessage("\t\tTarget component is ");
        
        switch(targetComponent)
        {
            case 0:
            {
                NeonMessage("x");
                break;
            }
            
            case 1:
            {
                NeonMessage("y");
                break;
            }
            
            case 2:
            {
                NeonMessage("z");
                break;
            }
            
            case 3:
            {
                NeonMessage("translation");
                break;
            }
        }
        
        NeonMessage("\n");
        
        // TODO - Iterate through animation curves
        
        int numCurves = curAnimation->GetNumCurves();
        
        for (int curCurveIndex = 0; curCurveIndex < numCurves; curCurveIndex++)
        {
            NeonMessage("\t\tCurve %d\n", curCurveIndex);
            
            AnimationCurve* curCurve = curAnimation->GetCurve(curCurveIndex);
            
            int numKeyframes = curCurve->GetNumKeyframes();
            
            NeonMessage("\t\tNum keyframes is %d\n", numKeyframes);
            
            for (int keyframeIndex = 0; keyframeIndex < numKeyframes; keyframeIndex++)
            {
                AnimationKeyframe* curKeyframe = curCurve->GetKeyframe(keyframeIndex);
                
                NeonMessage("\t\t\tKeyframe %d type is", keyframeIndex);
                
                switch(curKeyframe->GetKeyframeType())
                {                
                    case NEON21_ANIMATION_KEYFRAME_BEZIER:
                    {
                        NeonMessage("Bezier");
                        break;
                    }
                    
                    default:
                    {
                        // Unknown keyframe type
                        assert(false);
                        break;
                    }
                }
                
                NeonMessage("\n");
                
                NeonMessage("\t\t\t\tTime: %f\n", curKeyframe->GetTime());
                NeonMessage("\t\t\t\tValue: %f\n", curKeyframe->GetValue());
                
                switch(curKeyframe->GetKeyframeType())
                {
                    case NEON21_ANIMATION_KEYFRAME_BEZIER:
                    {
                        BezierAnimationKeyframe* bezierKeyframe = (BezierAnimationKeyframe*)curKeyframe;
                        
                        float inX, inY, outX, outY;
                        
                        bezierKeyframe->GetInTangent(&inX, &inY);
                        bezierKeyframe->GetOutTangent(&outX, &outY);
                        
                        NeonMessage("\t\t\t\tIn Tangent: %f, %f\n", inX, inY);
                        NeonMessage("\t\t\t\tOut Tangent: %f, %f\n", outX, outY);

                        break;
                    }
                    
                    default:
                    {
                        // Unknown keyframe type
                        assert(false);
                        break;
                    }
                }
            }
        }
    }
}