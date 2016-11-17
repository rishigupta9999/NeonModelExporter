#pragma once

#include <CoreServices/CoreServices.h>

class AnimationClip;
class Animation;

void WriteAnimationClips(CFMutableArrayRef inClipList);
void WriteAnimationClip(AnimationClip* inAnimationClip, FILE* inFile);
void WriteAnimation(Animation* inAnimation, FILE* inFile);