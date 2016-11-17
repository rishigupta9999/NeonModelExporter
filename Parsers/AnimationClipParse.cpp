#define NO_LIBXML
#include "FCollada.h"

#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationClip.h"
#include "FCDocument/FCDAnimationChannel.h"
#include "FCDocument/FCDAnimationKey.h"
#include "FCDocument/FCDAnimation.h"

#include "FCDocument/FCDExtra.h"

#include "AnimationClipParse.h"
#include "AnimationClip.h"
#include "Animation.h"
#include "Logging.h"

#include "Skeleton.h"

extern FUObjectRef<FCDocument> gDocument;

AnimationClip* ReadAnimationClip(FCDAnimationClip* inAnimationClip, Skeleton* inSkeleton)
{
    AnimationClip* retClip = new AnimationClip;
    
    retClip->SetAnimationClipName(inAnimationClip->GetName().c_str());
    
    int numAnimations = inAnimationClip->GetAnimationCount();
    
    if (numAnimations == 0)
    {
        printf("No animations found in animation clip %s.  Skipping.\n", inAnimationClip->GetName().c_str());
        goto fail;
    }
    
    for (int curAnimationIndex = 0; curAnimationIndex < numAnimations; curAnimationIndex++)
    {
        FCDAnimation* curAnimation = inAnimationClip->GetAnimation(curAnimationIndex);
        
        Joint*  targetJoint = NULL;
        int     componentIndex = 0;
        char    animationTargetName[NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH];
        
        bool success = FindAnimationTarget(curAnimation, inSkeleton, &targetJoint, &componentIndex, animationTargetName);
        
        if (!success)
        {
            goto fail;
        }
        
        Animation* newAnimation = new Animation;
        
        newAnimation->SetTarget(targetJoint, componentIndex, animationTargetName);
        newAnimation->SetAnimationName(curAnimation->GetName().c_str());
        
        retClip->AddAnimation(newAnimation);
                
        int channelCount = curAnimation->GetChannelCount();
        
        for (int curChannelIndex = 0; curChannelIndex < channelCount; curChannelIndex++)
        {
            FCDAnimationChannel* curChannel = curAnimation->GetChannel(curChannelIndex);
            
            int curveCount = curChannel->GetCurveCount();
            
            for (int curCurveIndex = 0; curCurveIndex < curveCount; curCurveIndex++)
            {
                AnimationCurve* newCurve = new AnimationCurve;
                newAnimation->AddCurve(newCurve);
                
                FCDAnimationCurve* curCurve = curChannel->GetCurve(curCurveIndex);
                int keyframeCount = curCurve->GetKeyCount();
                
                for (int curKeyframeIndex = 0; curKeyframeIndex < keyframeCount; curKeyframeIndex++)
                {
                    AnimationKeyframe* keyframe = ExtractKeyframe(curCurve->GetKey(curKeyframeIndex));
                    
                    if (keyframe == NULL)
                    {
                        printf("Failure encountered extracting a keyframe.  Aborting this animation clip.\n");
                        goto fail;
                    }
                    
                    newCurve->AddKeyframe(keyframe);
                }
            }
        }
    }
    
    return retClip;
    
fail:
    delete retClip;
    return NULL;
}

AnimationClip* ReadAnimation(FCDAnimationLibrary* inAnimationLibrary, FCDAnimation** inAnimationList, int inNumAnimations, Skeleton* inSkeleton)
{
    AnimationClip* retClip = new AnimationClip;
    
    retClip->SetAnimationClipName(inSkeleton->GetSkeletonName());
       
    if (inNumAnimations == 0)
    {
        printf("No animations found.  Skipping.\n");
        goto fail;
    }

    for (int curAnimationIndex = 0; curAnimationIndex < inNumAnimations; curAnimationIndex++)
    {
        FCDAnimation* curAnimation = inAnimationList[curAnimationIndex];
        
        Joint*  targetJoint = NULL;
        int     componentIndex = 0;
        char    animationTargetName[NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH];
        
        bool success = FindAnimationTarget(curAnimation, inSkeleton, &targetJoint, &componentIndex, animationTargetName);
        
        if (!success)
        {
            continue;
        }
        
        Animation* newAnimation = new Animation;
        
        newAnimation->SetTarget(targetJoint, componentIndex, animationTargetName);
        newAnimation->SetAnimationName(curAnimation->GetName().c_str());
                
        int channelCount = curAnimation->GetChannelCount();
        
        for (int curChannelIndex = 0; curChannelIndex < channelCount; curChannelIndex++)
        {
            FCDAnimationChannel* curChannel = curAnimation->GetChannel(curChannelIndex);
            
            int curveCount = curChannel->GetCurveCount();
            
            if (    ((componentIndex != w) && (curveCount != 1)) ||
                    ((componentIndex == w) && (curveCount != 3))    )
            {
                printf("This animation does not have an appropriate number of curves.  Aborting.\n");
                goto fail;
            }
            
            for (int curCurveIndex = 0; curCurveIndex < curveCount; curCurveIndex++)
            {
                AnimationCurve* newCurve = new AnimationCurve;
                newAnimation->AddCurve(newCurve);
                
                FCDAnimationCurve* curCurve = curChannel->GetCurve(curCurveIndex);
                int keyframeCount = curCurve->GetKeyCount();
                
                for (int curKeyframeIndex = 0; curKeyframeIndex < keyframeCount; curKeyframeIndex++)
                {
                    AnimationKeyframe* keyframe = ExtractKeyframe(curCurve->GetKey(curKeyframeIndex));
                    
                    if (keyframe == NULL)
                    {
                        printf("Failure encountered extracting a keyframe.  Aborting this animation clip.\n");
                        goto fail;
                    }
                    
                    newCurve->AddKeyframe(keyframe);
                }
            }
        }
        
        // Special test for zero translations.  Strip these out.
        bool allZero = true;
        bool testZero = false;
        
        if ((targetJoint->GetID() != 0) || (targetJoint->GetJointIdentifier() != 0))
        {
            if (newAnimation->GetNumCurves() == 3)
            {
                testZero = true;
                
                for (int curveIndex = 0; curveIndex < 3; curveIndex++)
                {
                    AnimationCurve* testCurve = newAnimation->GetCurve(curveIndex);
                    
                    int numKeyframes = testCurve->GetNumKeyframes();
                    
                    for (int curKeyframe = 0; curKeyframe < numKeyframes; curKeyframe++)
                    {
                        AnimationKeyframe* keyframe = testCurve->GetKeyframe(curKeyframe);
                        
                        if (fabsf(keyframe->GetValue()) > EPSILON)
                        {
                            allZero = false;
                            goto NON_ZERO;
                        }
                    }
                }
            }
        }
        
NON_ZERO:
        if (testZero && allZero)
        {
            printf("Zeroed out translation found for a joint other than the root.  Stripping this, as this is invalid\n");
        }
        else
        {
            retClip->AddAnimation(newAnimation);
        }
    }
    
    return retClip;
    
fail:
    delete retClip;
    return NULL;
}

bool FindAnimationTarget(FCDAnimation* inAnimation, Skeleton* inSkeleton, Joint** outJoint, int* outComponent, char* outTargetName)
{
    // Need to find what joint in the skeleton this animation is driving
    FCDExtra* extra = inAnimation->GetExtra();
    
    if (extra == NULL)
    {
        printf("No extra metadata attached to animation.  With no animation target, we can't export this animation.\n");
        return false;
    }
    
    int typeCount = extra->GetTypeCount();
    
    FCDEType* animTargetType = NULL;
    
    for (int curTargetIndex = 0; curTargetIndex < typeCount; curTargetIndex++)
    {
        FCDEType* curType = extra->GetType(curTargetIndex);
        
        const char* nameString = curType->GetName().c_str();
        
        // FAXAnimationImport attaches the animation target to the "AnimTarget" extra information.  Definitely
        // a hack - but we're not updating to a new FCollada mid-cycle.  Can fix this in the off-cycle.
        if (strcmp(nameString, "AnimTargets") == 0)
        {
            animTargetType = curType;
            break;
        }
    }
    
    if (animTargetType == NULL)
    {
        printf("Didn't find an AnimTarget.  Can't export this animation.\n");
        return false;
    }
    
    int techniqueCount = animTargetType->GetTechniqueCount();
    
    if (techniqueCount == 0)
    {
        printf("No technique count.  With no animation target, we can't export this animation.\n");
        return false;
    }
    
    FCDETechnique* targetTechnique = NULL;
    
    for (int curTechniqueIndex = 0; curTechniqueIndex < techniqueCount; curTechniqueIndex++)
    {
        FCDETechnique* curTechnique = animTargetType->GetTechnique(curTechniqueIndex);
        const char* profile = curTechnique->GetProfile();
        
        // This isn't a typo.  FAXAnimationImport actually attaches the target information under "TEMP".
        if (strcmp(profile, "TEMP") == 0)
        {
            targetTechnique = curTechnique;
            break;
        }
    }
    
    if (targetTechnique == NULL)
    {
        printf("Couldn't find a target technique.  With no animation target, we can't export this animation.\n");
        return false;
    }
    
    int childNodeCount = targetTechnique->GetChildNodeCount();
    
    if (childNodeCount == 0)
    {
        printf("No child nodes found.  With no animation target, we can't export this animation.\n");
        return false;
    }
    
    int pointerIndex = 0;
    
    for (int childNodeIndex = 0; childNodeIndex < childNodeCount; childNodeIndex++)
    {
        FCDENode* childNode = targetTechnique->GetChildNode(childNodeIndex);
        
        if (strcmp(childNode->GetName(), "pointer") == 0)
        {
            switch(pointerIndex)
            {
                case 0:
                {
                    GetJointAndComponent(inSkeleton, childNode->GetContent(), outJoint, outComponent, outTargetName);
                    
                    if (*outJoint == NULL)
                    {
                        NeonWarning("Failure getting joint or component for %s, this animation won't be exported.\n", childNode->GetContent());
                        return false;
                    }
                    
                    break;
                }
                
                case 1:
                {
                    if (*outComponent != w)
                    {
                        const char* content = childNode->GetContent();
                        
                        if (strcmp(content, ".ANGLE") != 0)
                        {
                            printf("Animation target isn't an angle.  Don't know what to do here\n");
                            return false;
                        }
                    }
                    
                    break;
                }
            }
            
            pointerIndex++;
        }
    }
    
    return true;
}

void GetJointAndComponent(Skeleton* inSkeleton, const char* inJointPath, Joint** outJoint, int* outComponent, char* outComponentName)
{
    char* jointName = NULL;
    char* componentName = NULL;
    Joint* joint = NULL;
    int component = 0;
        
    char* scratch = new char[strlen(inJointPath) + 1];
    strcpy(scratch, inJointPath);
    
    jointName = scratch;
    
    componentName = strstr(scratch, "/");
        
    if (componentName == NULL)
    {
        NeonWarning("Animation Channel target appears to be malformed.  Cannot export this animation.\n");
        *outJoint = NULL;
        return;
    }
    
    *componentName = 0;
    componentName++;
    
    assert(strlen(componentName) < NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH);
    
    fm::string targetString = fm::string(jointName);
    FCDEntity* targetEntity = gDocument->FindEntity(targetString);

    joint = inSkeleton->GetJointWithEntity(targetEntity);
    
    if (joint == NULL)
    {
        printf("Couldn't find joint named %s.  Cannot export this animation\n", inJointPath);
        goto fail;
    }
    
    if (strcmp(componentName, "rotateX") == 0)
    {
        component = x;
    }
    else if (strcmp(componentName, "rotateY") == 0)
    {
        component = y;
    }
    else if (strcmp(componentName, "rotateZ") == 0)
    {
        component = z;
    }
    else if (strcmp(componentName, "translate") == 0)
    {
        component = w;
    }
    else
    {
        NeonWarning("Unrecognized component %s for animation target.  Cannot export this animation\n", componentName);
        goto fail;
    }
    
    strncpy(outComponentName, componentName, NEON21_MODELEXPORTER_TARGET_TRANSFORM_NAME_LENGTH);
    
    *outJoint = joint;
    *outComponent = component;
    return;
    
fail:
    *outJoint = NULL;
    return;
}

AnimationKeyframe* ExtractKeyframe(FCDAnimationKey* inKey)
{
    FUDaeInterpolation::Interpolation interpolationType = (FUDaeInterpolation::Interpolation)inKey->interpolation;
    
    AnimationKeyframe* animationKeyframe = NULL;
    
    if (interpolationType == FUDaeInterpolation::BEZIER)
    {
        BezierAnimationKeyframe* bezierKeyframe = new BezierAnimationKeyframe;
        FCDAnimationKeyBezier* fcdBezierKeyframe = (FCDAnimationKeyBezier*)inKey;
        
        bezierKeyframe->SetInTangent(fcdBezierKeyframe->inTangent.x, fcdBezierKeyframe->inTangent.y);
        bezierKeyframe->SetOutTangent(fcdBezierKeyframe->outTangent.x, fcdBezierKeyframe->outTangent.y);
        
        animationKeyframe = bezierKeyframe;
    }
    else
    {
        printf("Unsupported keyframe type.\n");
        return NULL;
    }
    
    animationKeyframe->SetTime(inKey->input);
    animationKeyframe->SetValue(inKey->output);
    
    return animationKeyframe;
}