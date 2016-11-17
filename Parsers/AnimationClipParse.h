#pragma once

#include "FCDocument/FCDocument.h"

class Skeleton;
class Joint;
class Animation;
class AnimationClip;
class AnimationKeyframe;
class FCDAnimationClip;
class FCDAnimationKey;

AnimationClip* ReadAnimationClip(FCDAnimationClip* inAnimationClip, Skeleton* inSkeleton);
AnimationClip* ReadAnimation(FCDAnimationLibrary* inAnimationLibrary, FCDAnimation** inAnimationList, int inNumAnimations, Skeleton* inSkeleton);
bool FindAnimationTarget(FCDAnimation* inAnimation, Skeleton* inSkeleton, Joint** outJoint, int* outComponent, char* outTargetName);

void GetJointAndComponent(Skeleton* inSkeleton, const char* inJointPath, Joint** outJoint, int* outComponent, char* outComponentName);
AnimationKeyframe* ExtractKeyframe(FCDAnimationKey* inKey);