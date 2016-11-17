#include "AnimationClipSerialize.h"
#include "AnimationClip.h"
#include "Animation.h"
#include "Skeleton.h"

#include "ModelExporterDefines.h"

void WriteAnimationClips(CFMutableArrayRef inClipList)
{
    int numClips = CFArrayGetCount(inClipList);
    
    for (int curClipIndex = 0; curClipIndex < numClips; curClipIndex++)
    {
        AnimationClip* curClip = static_cast<AnimationClip*>(const_cast<void*>(CFArrayGetValueAtIndex(inClipList, curClipIndex)));
        
        if (curClip != NULL)
        {
            curClip->DumpInfo();

            char* animClipName = curClip->GetAnimationClipName();
            
            if (animClipName[0] == 0)
            {
                snprintf(animClipName, AnimationClip::ANIMATION_CLIP_NAME_LENGTH, "AnimationClip%d\n", curClipIndex);
            }
            
            char fileName[AnimationClip::ANIMATION_CLIP_NAME_LENGTH + 6];
            
            snprintf(fileName, sizeof(fileName), "%s.%s", animClipName, NEON21_MODELEXPORTER_ANIMATION_EXTENSION);
            
            FILE* curFile = fopen(fileName, "w+");
            
            WriteAnimationClip(curClip, curFile);
            
            fclose(curFile);
        }
    }
}

void WriteAnimationClip(AnimationClip* inAnimationClip, FILE* inFile)
{
    // Write out the animation clip header
    
    AnimationClipHeader clipHeader;
    
    clipHeader.mMajorVersion = NEON21_MODELEXPORTER_MAJOR_VERSION;
    clipHeader.mMinorVersion = NEON21_MODELEXPORTER_MINOR_VERSION;
    
    strncpy(clipHeader.mName, inAnimationClip->GetAnimationClipName(), NEON21_MODELEXPORTER_ANIMATION_CLIP_NAME_LENGTH);
    clipHeader.mNumAnimations = inAnimationClip->GetNumAnimations();
    
    fwrite(&clipHeader, sizeof(clipHeader), 1, inFile);
    
    for (int curAnimation = 0; curAnimation < clipHeader.mNumAnimations; curAnimation++)
    {
        WriteAnimation(inAnimationClip->GetAnimation(curAnimation), inFile);
    }
}

void WriteAnimation(Animation* inAnimation, FILE* inFile)
{
    AnimationHeader animHeader;
    
    strncpy(animHeader.mName, inAnimation->GetAnimationName(), NEON21_MODELEXPORTER_ANIMATION_NAME_LENGTH);
    animHeader.mNumCurves = inAnimation->GetNumCurves();
    
    Joint* targetJoint = NULL;
    int targetComponent = 0;
    char targetName[NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH];
    
    inAnimation->GetTarget(&targetJoint, &targetComponent, targetName);
    
    if (strlen(targetJoint->GetJointName()) >= NEON21_MODELEXPORTER_JOINT_NAME_LENGTH)
    {
        printf("Joint name %s is too long, this animation cannot be exported.\n", targetJoint->GetJointName());
        return;
    }
    else
    {
        strncpy(animHeader.mJointName, targetJoint->GetJointName(), NEON21_MODELEXPORTER_JOINT_NAME_LENGTH);
        animHeader.mComponent = targetComponent;
        strncpy(animHeader.mTargetTransformName, targetName, NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH);
    }
    
    fwrite(&animHeader, sizeof(animHeader), 1, inFile);
    
    for (int curCurveIndex = 0; curCurveIndex < animHeader.mNumCurves; curCurveIndex++)
    {
        AnimationCurve* curCurve = inAnimation->GetCurve(curCurveIndex);
        
        AnimationCurveHeader curveHeader;
        curveHeader.mNumKeyframes = curCurve->GetNumKeyframes();
        
        fwrite(&curveHeader, sizeof(curveHeader), 1, inFile);
        
        for (int curKeyframeIndex = 0; curKeyframeIndex < curveHeader.mNumKeyframes; curKeyframeIndex++)
        {
            AnimationKeyframe* curKeyframe = curCurve->GetKeyframe(curKeyframeIndex);
            
            AnimationKeyframeCommon commonKeyframeData;
            
            commonKeyframeData.mKeyframeType = curKeyframe->GetKeyframeType();
            commonKeyframeData.mKeyframeTime = curKeyframe->GetTime();
            commonKeyframeData.mKeyframeValue = curKeyframe->GetValue();
            
            fwrite(&commonKeyframeData, sizeof(AnimationKeyframeCommon), 1, inFile);
            
            switch(commonKeyframeData.mKeyframeType)
            {
                case NEON21_ANIMATION_KEYFRAME_BEZIER:
                {
                    BezierKeyframeData bezierKeyframeData;
                    BezierAnimationKeyframe* bezierKeyframe = (BezierAnimationKeyframe*)curKeyframe;
                    
                    bezierKeyframe->GetInTangent(&bezierKeyframeData.mInTangentX, &bezierKeyframeData.mInTangentY);
                    bezierKeyframe->GetOutTangent(&bezierKeyframeData.mOutTangentX, &bezierKeyframeData.mOutTangentY);
                    
                    fwrite(&bezierKeyframeData, sizeof(bezierKeyframeData), 1, inFile);
                    
                    break;
                }
                
                default:
                {
                    // Unknown animation format.  You are now the proud owner of a corrupt animation file.
                    assert(false);
                    break;
                }
            }
        }
    }
}