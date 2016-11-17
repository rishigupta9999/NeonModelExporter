#include "Skeleton.h"

class FCDControllerInstance;

Skeleton* ReadController(FCDControllerInstance* inControllerInstance);

int     FindSkeletonRoot(FCDControllerInstance* inControllerInstance, FCDSceneNode** inJoints, int inNumJoints);
int     FindIndexForNode(FCDSceneNode* inNode, FCDSceneNode** inSceneNodes, int inNumNodes);

void    GetNetTransform(FCDSceneNode* inSceneNode, Matrix44* outTransform);
Joint*  CreateSkeletonFromRoot(  int inRootIndex, FCDSceneNode* inChild, FCDSceneNode** inSceneNodes,
                                 int inNumNodes, Matrix44* inCoalesceMatrix, bool inValidCoalesceMatrix, FCDController* inController);
