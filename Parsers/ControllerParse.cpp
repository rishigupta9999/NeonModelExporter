#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

#include "main.h"

#define NO_LIBXML
#include "FCollada.h"

#include "FCDocument/FCDSceneNode.h"

#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDGeometry.h"

#include "ControllerParse.h"
#include "GeometryParse.h"

#include "Mesh.h"

#include "NeonMath.h"
#include "Logging.h"

void BuildSceneNodePointers(FCDSceneNode* inSceneNode, FCDSceneNode** outJointNodeList)
{
    
}

void CountSceneNodes(FCDSceneNode* inSceneNode, int* inOutNumSceneNodes)
{
    // Count ourselves
    (*inOutNumSceneNodes)++;
    
    int numChildren = inSceneNode->GetChildrenCount();
    
    for (int curChild = 0; curChild < numChildren; curChild++)
    {
        FCDSceneNode* child = inSceneNode->GetChild(curChild);
        
        CountSceneNodes(child, inOutNumSceneNodes);
    }
}

Skeleton* ReadController(FCDControllerInstance* inControllerInstance)
{
    FCDController* controller = (FCDController*)inControllerInstance->GetEntity();
    
    if (!controller->IsSkin())
    {
        printf("This controller is not a skin and cannot be exported.\n");
        return NULL;
    }
        
    int jointCount = inControllerInstance->GetJointCount();
    
    FCDSceneNode** jointNodes = new FCDSceneNode*[jointCount];
    
    // Save pointers to all joints
    for (int curJoint = 0; curJoint < jointCount; curJoint++)
    {
        FCDSceneNode* joint = inControllerInstance->GetJoint(curJoint);
        
        if (joint == NULL)
        {
            printf("Null joint found, this shouldn't be possible.  Skipping export of this skeleton.\n");
            return NULL;
        }
        
        jointNodes[curJoint] = joint;
    }
    
    // Find the root, there should only be one
    int rootIndex = FindSkeletonRoot(inControllerInstance, jointNodes, jointCount);

#if 0    
    // Get pointers to all joints
    
    int numSceneNodes = 0;
    CountSceneNodes(jointNodes[rootIndex], &numSceneNodes);
    
    // Nodes of type JOINT are in the jointNodes list right now.  We also want nodes of type NODE as well, since these affect
    // the transform.
    BuildSceneNodePointers();
#endif        

    // Begin traversal of the skeleton

    Matrix44 identity;
    SetIdentity(&identity);
    
    Joint* rootJoint = CreateSkeletonFromRoot(rootIndex, NULL, jointNodes, jointCount, &identity, false, controller);
#if 0
    rootJoint->PrintHierarchy();
#endif    
    // Create Skeleton
    
    Skeleton* skeleton = new Skeleton;
    
    skeleton->SetRootJoint(rootJoint);
    skeleton->SetSkeletonName(controller->GetName().c_str());
        
    // Cleanup
    delete[] jointNodes;
    
    // Let's see if there's an associated mesh
    
    FCDSkinController* skinController = controller->GetSkinController();
    FCDEntity* geometry = skinController->GetTarget();
    
    if (geometry->GetType() == FCDEntity::GEOMETRY)
    {
        Mesh* retMesh = ReadGeometry(NULL, (FCDGeometry*)geometry);
        
        int numInfluences = skinController->GetInfluenceCount();
        
        uint32_t*               matrixCounts = new uint32_t[numInfluences];
        
        std::vector<int32_t>    matrixIndices;
        std::vector<float>      matrixWeights;
        
        // Extract the following information:
        // 1) Number of joints that each vertex is influenced by
        // 2) The indices for the joints that the vertex is influenced by
        // 3) The weights for each joint
        
        for (int curInfluence = 0; curInfluence < numInfluences; curInfluence++)
        {
            FCDSkinControllerVertex* vertex = skinController->GetVertexInfluence(curInfluence);
            
            matrixCounts[curInfluence] = vertex->GetPairCount();
            
            for (int curPair = 0; curPair < matrixCounts[curInfluence]; curPair++)
            {
                FCDJointWeightPair* pair = vertex->GetPair(curPair);
                
                matrixIndices.push_back(pair->jointIndex);
                matrixWeights.push_back(pair->weight);
            }
        }
        
        int numMatrixIndices = matrixIndices.size();
        uint32_t* matrixIndexStream = new uint32[numMatrixIndices];
        
        for (int curIndex = 0; curIndex < numMatrixIndices; curIndex++)
        {
            matrixIndexStream[curIndex] = matrixIndices[curIndex];
        }
        
        int numMatrixWeights = matrixWeights.size();
        float* matrixWeightsStream = new float[numMatrixWeights];
        
        for (int curIndex = 0; curIndex < numMatrixWeights; curIndex++)
        {
            matrixWeightsStream[curIndex] = matrixWeights[curIndex];
        }
        
        retMesh->AddMatrixCounts(numInfluences, matrixCounts);
        retMesh->AddMatrixIndices(numMatrixIndices, matrixIndexStream);
        retMesh->AddMatrixWeights(numMatrixWeights, matrixWeightsStream);
        
        FMMatrix44 bindShapeMatrix = skinController->GetBindShapeTransform();
        
        Matrix44 bindShapeMatrixColMajor;
        
        for (int i = 0; i < 16; i++)
        {
            bindShapeMatrixColMajor.mMatrix[i] = bindShapeMatrix.m[i / 4][i % 4];
        }
        
        retMesh->SetBindShapeMatrix(&bindShapeMatrixColMajor);
        
        delete [] matrixCounts;
                
        CFArrayAppendValue(gMeshList, retMesh);
    }
        
    return skeleton;
}

int FindSkeletonRoot(FCDControllerInstance* inControllerInstance, FCDSceneNode** inJoints, int inNumJoints)
{
    int rootIndex = Joint::INVALID_JOINT_INDEX;
    
    for (int jointIndex = 0; jointIndex < inNumJoints; jointIndex++)
    {
        size_t parentCount = inJoints[jointIndex]->GetParentCount();
        
        for (int parentIndex = 0; parentIndex < parentCount; parentIndex++)
        {
            FCDSceneNode* parent = inJoints[jointIndex]->GetParent(parentIndex);
            
            if (!inControllerInstance->FindJoint(parent))
            {
                if (rootIndex == -1)
                {
                    rootIndex = jointIndex;
                    return rootIndex;
                }
                else
                {
                    printf("This skeleton has multiple roots, this won't be exported correcty.\n");
                }
            }
        }
    }
    
    return rootIndex;
}

int FindIndexForNode(FCDSceneNode* inNode, FCDSceneNode** inSceneNodes, int inNumNodes)
{
    for (int i = 0; i < inNumNodes; i++)
    {
        if (inSceneNodes[i] == inNode)
        {
            return i;
        }
    }
    
    return Joint::INVALID_JOINT_INDEX;
}

void GetTransformRecords(FCDSceneNode* inSceneNode, TransformRecord** outTransformRecords, int* outNumTransformRecords)
{
    int transformCount = inSceneNode->GetTransformCount();
    
    TransformRecord* transformRecords = new TransformRecord[transformCount];
    memset(transformRecords, 0, sizeof(TransformRecord) * transformCount);
    
    *outTransformRecords = transformRecords;
    
    int writeIndex = 0;
    
    for (int curTransformIndex = 0; curTransformIndex < transformCount; curTransformIndex++)
    {
        FCDTransform* curTransform = inSceneNode->GetTransform(curTransformIndex);
        TransformRecord* curRecord = &transformRecords[writeIndex];
        
        // Populate transform name
        fm::string transformName = curTransform->GetSubId();
        const char* transformString = transformName.c_str();
        
        strncpy(curRecord->mTransformName, transformString, NEON21_MODELEXPORTER_TRANSFORM_NAME_LENGTH);
        
        // Populate transform type
        FCDTransform::Type transformType = curTransform->GetType();
        
        switch(transformType)
        {
            case FCDTransform::TRANSLATION:
            {
                FCDTTranslation* translation = (FCDTTranslation*)curTransform;
                FMVector3 translationData = translation->GetTranslation();
                                
                curRecord->mTransformType = TRANSFORM_TYPE_TRANSLATION;
                
                curRecord->mTransformData.mMatrix[0] = translationData.x;
                curRecord->mTransformData.mMatrix[1] = translationData.y;
                curRecord->mTransformData.mMatrix[2] = translationData.z;
                curRecord->mTransformData.mMatrix[3] = 0.0;
                
                writeIndex++;
                
                break;
            }
            
            case FCDTransform::ROTATION:
            {
                FCDTRotation* rotation = (FCDTRotation*)curTransform;
                FMVector3 axis = rotation->GetAxis();
                float angle = rotation->GetAngle();
                
                curRecord->mTransformType = TRANSFORM_TYPE_ROTATION;
                
                curRecord->mTransformData.mMatrix[0] = axis.x;
                curRecord->mTransformData.mMatrix[1] = axis.y;
                curRecord->mTransformData.mMatrix[2] = axis.z;
                curRecord->mTransformData.mMatrix[3] = angle;
                
                writeIndex++;
                
                break;
            }
            
            case FCDTransform::SCALE:
            {
                FCDTScale* scale = (FCDTScale*)curTransform;
                FMVector3 scaleVec = scale->GetScale();
                
                // We don't support scales in the skeleton
                assert((scaleVec.x == 1.0) && (scaleVec.y == 1.0) && (scaleVec.z == 1.0));
                
                break;
            }
            
            default:
            {
                // Unknown transform.  How do we handle it?
                assert(false);
                break;
            }
        }
    }
    
    *outNumTransformRecords = writeIndex;
}

// inRootIndex: The index in the joint list (inSceneNodes) for this particular node we are processing
// inChild:     The SceneNode* corresponding to this joint.  This can be NULL, in which case we will check
//              inSceneNodes[inRootIndex] for the child.
// inSceneNodes:The list of SceneNodes corresponding to this skeleton.  Only SceneNodes of type JOINT are
//              in here.  SceneNodes of type NODE are not.  They will be found during regular traversal
//              and have their transforms coalesced with the children, but they will not be exported as
//              discrete joints.
// inNumNodes:  The size of inSceneNodes.
// inCoalesceMatrix:A matrix that should be coalesced with this node's transform.
// inValidCoalesceMatrix:Whether inCoalesceMatrix is valid and should be added to the node's transform records.
// inController:An FCollada skin controller object.

Joint* CreateSkeletonFromRoot(  int inRootIndex, FCDSceneNode* inChild, FCDSceneNode** inSceneNodes,
                                int inNumNodes, Matrix44* inCoalesceMatrix, bool inValidCoalesceMatrix, FCDController* inController)
{
    Joint*          joint = NULL;
    FCDSceneNode*   node = inChild;
    FCDSkinController* skinController = inController->GetSkinController();
    
    if (node == NULL)
    {
        node = inSceneNodes[inRootIndex];
    }
        
    Matrix44 inverseBindPoseMatrix;
    Matrix44 coalesceMatrix;
    bool     shouldCoalesce = false;
    
    if (inRootIndex != Joint::INVALID_JOINT_INDEX)
    {
        joint = new Joint;
        
        const FMMatrix44& bindPoseInverse = skinController->GetJoint(inRootIndex)->GetBindPoseInverse();
        LoadFromColMajorArrayOfArrays(&inverseBindPoseMatrix, (float(*)[4])bindPoseInverse.m);
        
        joint->SetID(inRootIndex);
        joint->SetInverseBindPose(&inverseBindPoseMatrix);
        joint->SetJointName(node->GetName().c_str());
        joint->SetJointEntity(node);
        
        if (inValidCoalesceMatrix)
        {
            TransformRecord* coalesceTransform = new TransformRecord;
            memset(coalesceTransform, 0, sizeof(TransformRecord));
            
            coalesceTransform->mTransformType = TRANSFORM_TYPE_MATRIX;
            coalesceTransform->mTransformName[0] = 0;
            CloneMatrix44(inCoalesceMatrix, &coalesceTransform->mTransformData);
            
            joint->AddTransform(coalesceTransform);
        }
        
        TransformRecord* records;
        int numRecords;
        
        GetTransformRecords(node, &records, &numRecords);
        
        for (int curTransform = 0; curTransform < numRecords; curTransform++)
        {
            joint->AddTransform(&records[curTransform]);
        }
        
        SetIdentity(&coalesceMatrix);
    }
    else if (inChild != NULL)
    {
        // This is a non-terminating joint that has no vertex data skinned to it.  Let's get its tranform
        // and coalesce it with all children.
        const FMMatrix44& nodeMatrix = inChild->ToMatrix();
        LoadFromColMajorArrayOfArrays(&coalesceMatrix, (float(*)[4])nodeMatrix.m);

        if (inValidCoalesceMatrix)
        {
            MatrixMultiply(inCoalesceMatrix, &coalesceMatrix, &coalesceMatrix);
        }

        shouldCoalesce = true;
    }
    else
    {
        printf("Neither a valid index, nor a valid scene node pointer were provided.  Can't continue.\n");
        return NULL;
    }
    
    
    // Traverse the children
    int numChildren = node->GetChildrenCount();
    
    for (int curChild = 0; curChild < numChildren; curChild++)
    {
        FCDSceneNode* child = node->GetChild(curChild);
        
        if (child == NULL)
        {
            printf("Null child node for parent %s.  You may have a badly exported skeleton.\n", node->GetName().c_str());
            continue;
        }
        
        int index = FindIndexForNode(child, inSceneNodes, inNumNodes);
        
        // JointIndex can be invalid when there's a dummy joint in the skeleton that has no geometry bound to it.  We
        // still need to export it so that the transform of child joints are accurate.
        if (index == Joint::INVALID_JOINT_INDEX)
        {
            // If there are no children, this is a dummy terminating joint.  It should have no impact on the final skeleton.
            if (child->GetChildrenCount() == 0)
            {
                continue;
            }
            else
            {
                // If a dummy node has more than one child, then CreateSkeletonFromRoot needs the ability to return multiple joints
                // (so all the dummy nodes's children can be attached to this joint, right now just the dummy node's one child
                // is passed back to be attached to this node)
                assert(child->GetChildrenCount() == 1);
            }
            
            // Otherwise we should process this node.  It's transform will be coalesced with the parent's transform when
            // calculating the children's transforms.
        }

        {
            Joint* childJoint = CreateSkeletonFromRoot(index, child, inSceneNodes, inNumNodes, &coalesceMatrix, shouldCoalesce, inController);
            
            if (joint == NULL)
            {
                assert(shouldCoalesce);
                joint = childJoint;
            }
            else
            {
                joint->GetChildren()->push_back(childJoint);
                NeonMessage("Adding joint %s to %s\n", childJoint->GetJointName(), joint->GetJointName());
            }
        }
    }

    return joint;
}