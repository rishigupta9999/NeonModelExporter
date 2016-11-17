#include <assert.h>

#include "Skeleton.h"

int Joint::mNextIdentifier = 0;

static const char* sTransformTypeStrings[TRANSFORM_TYPE_MAX] = {    "Invalid",
                                                                    "Rotation",
                                                                    "Translation",
                                                                    "Matrix" };
                                                                    
TransformRecord sExpectedJoints[] = { { TRANSFORM_TYPE_MATRIX,      { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 }, "" },
                                      { TRANSFORM_TYPE_TRANSLATION, { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "translate" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "jointOrientZ" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "jointOrientY" },
                                      { TRANSFORM_TYPE_ROTATION,    { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "jointOrientX" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateZ" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateY" },
                                      { TRANSFORM_TYPE_ROTATION,    { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateX" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateAxisZ" },
                                      { TRANSFORM_TYPE_ROTATION,    { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateAxisY" },
                                      { TRANSFORM_TYPE_ROTATION,    { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, "rotateAxisX" } };

#define INSERT_TABS     for (int i = 0; i < numTabs; i++) printf("\t");

Joint::Joint()
{
    mIdentifier = INVALID_JOINT_INDEX;
    mID = INVALID_JOINT_INDEX;
    
    mJointEntity = NULL;
    
    SetIdentity(&mInverseBindPose);
    
    memset(mJointName, 0, JOINT_NAME_LENGTH);
}

Joint::~Joint()
{
    DeleteHierarchy();
}

void Joint::DeleteHierarchy()
{
    while(mChildren.size() != 0)
    {
        Joint* lastElem = mChildren.back();
        mChildren.pop_back();
        
        delete lastElem;
    }
}


static int numTabs = 0;

void Joint::PrintHierarchy()
{
    INSERT_TABS
    printf("--------------------------------\n");

    INSERT_TABS
    printf("%s, %d\n", mJointName, mID);
    
    for (int curTransformIndex = 0; curTransformIndex < mTransforms.size(); curTransformIndex++)
    {
        TransformRecord* curTransform = mTransforms.at(curTransformIndex);
                
        INSERT_TABS
        printf("Transform %d:\n", curTransformIndex);
        
        INSERT_TABS
        printf("Type is %s\n", sTransformTypeStrings[curTransform->mTransformType]);
        
        switch(curTransform->mTransformType)
        {
            case TRANSFORM_TYPE_ROTATION:
            {
                INSERT_TABS
                printf("Axis %.2f, Angle %.2f, %.2f, %.2f\n",   curTransform->mTransformData.mMatrix[3], 
                                                                curTransform->mTransformData.mMatrix[0],
                                                                curTransform->mTransformData.mMatrix[1],
                                                                curTransform->mTransformData.mMatrix[2]);
                break;
            }
            
            case TRANSFORM_TYPE_TRANSLATION:
            {
                INSERT_TABS
                printf("Translation %.2f, %.2f, %.2f\n",    curTransform->mTransformData.mMatrix[0], 
                                                            curTransform->mTransformData.mMatrix[1],
                                                            curTransform->mTransformData.mMatrix[2]);
                break;
            }
            
            case TRANSFORM_TYPE_MATRIX:
            {
                INSERT_TABS
                printf("Matrix:\t%f, %f, %f, %f\n", curTransform->mTransformData.mMatrix[0],
                                                    curTransform->mTransformData.mMatrix[4],
                                                    curTransform->mTransformData.mMatrix[8],
                                                    curTransform->mTransformData.mMatrix[12]);
                                                    
                INSERT_TABS
                printf("\t\t\t%f, %f, %f, %f\n",    curTransform->mTransformData.mMatrix[1],
                                                    curTransform->mTransformData.mMatrix[5],
                                                    curTransform->mTransformData.mMatrix[9],
                                                    curTransform->mTransformData.mMatrix[13]);
                
                INSERT_TABS
                printf("\t\t\t%f, %f, %f, %f\n",    curTransform->mTransformData.mMatrix[2],
                                                    curTransform->mTransformData.mMatrix[6],
                                                    curTransform->mTransformData.mMatrix[10],
                                                    curTransform->mTransformData.mMatrix[14]);
                                                    
                INSERT_TABS
                printf("\t\t\t%f, %f, %f, %f\n",    curTransform->mTransformData.mMatrix[3],
                                                    curTransform->mTransformData.mMatrix[7],
                                                    curTransform->mTransformData.mMatrix[11],
                                                    curTransform->mTransformData.mMatrix[15]);
                break;
            }
            
            default:
            {
                assert(false);
            }
        }
    }
    
    INSERT_TABS
    printf("--------------------------------\n");
        
    numTabs++;
    
    for (std::vector<Joint*>::iterator curChild = mChildren.begin(); curChild < mChildren.end(); curChild++)
    {
        (*curChild)->PrintHierarchy();
    }
    
    numTabs--;
}

int Joint::CountJoints()
{
    // This joint
    int jointCount = 1;
    
    for (std::vector<Joint*>::iterator curChild = mChildren.begin(); curChild < mChildren.end(); curChild++)
    {
        jointCount+= (*curChild)->CountJoints();
    }
    
    return jointCount;
}

void Joint::GenerateJointList(std::vector<Joint*>* inJointList)
{
    mNextIdentifier = 0;
    
    GenerateJointListHelper(inJointList);
}

void Joint::GenerateJointListHelper(std::vector<Joint*>* inJointList)
{
    mIdentifier = mNextIdentifier;
    
    // Having a separate identifier and ID is redundant.  They should be identical for now.
    assert(mIdentifier == mID);
    
    inJointList->push_back(this);
    
    mNextIdentifier++;
    
    for (std::vector<Joint*>::iterator curChild = mChildren.begin(); curChild < mChildren.end(); curChild++)
    {
        (*curChild)->GenerateJointListHelper(inJointList);
    }
}

int Joint::GetJointIdentifier()
{
    return mIdentifier;
}
        
char* Joint::GetJointName()
{
    return mJointName;
}

Matrix44* Joint::GetInverseBindPoseTransform()
{
    return &mInverseBindPose;
}

std::vector<Joint*>* Joint::GetChildren()
{
    return &mChildren;
}

std::vector<TransformRecord*>* Joint::GetTransforms()
{
    return &mTransforms;
}

void Joint::SetInverseBindPose(Matrix44* inBindPose)
{
    CloneMatrix44(inBindPose, &mInverseBindPose);
}

void Joint::SetJointName(const char* inJointName)
{
    strncpy(mJointName, inJointName, JOINT_NAME_LENGTH);
}

void Joint::AddTransform(TransformRecord* inTransform)
{
    mTransforms.push_back(inTransform);
}

void Joint::SetJointEntity(FCDEntity* inEntity)
{
    mJointEntity = inEntity;
}

FCDEntity* Joint::GetJointEntity()
{
    return mJointEntity;
}

void Joint::SetID(int inID)
{
    mID = inID;
}

int Joint::GetID()
{
    return mID;
}

void Joint::Validate()
{
    // First, let's verify that all transforms appear in the correct order.  If they don't, then inserting the dummy transforms below won't work properly.
    
    int numJointTransforms = mTransforms.size();
    
    int lastTransformIndex = -1;
    
    for (int curJointTransform = 0; curJointTransform < numJointTransforms; curJointTransform++)
    {
        TransformRecord* curTransform = mTransforms.at(curJointTransform);
        
        int transformIndex = Joint::GetExpectedTransformIndex(curTransform->mTransformName);
        
        if (transformIndex <= lastTransformIndex)
        {
            printf("Transforms were not in the expected order.  This requires debugging and inspecting the skeleton in the .dae file");
            assert(false);
        }
        else
        {
            lastTransformIndex = transformIndex;
        }
    }
    
    // If we made it here, time to insert dummy transforms
    
    int numExpectedTransforms = sizeof(sExpectedJoints) / sizeof(TransformRecord);
    
    int insertIndex = 0;
    
    for (int curExpectedTransform = 0; curExpectedTransform < numExpectedTransforms; curExpectedTransform++)
    {
        // No need to export dummy matrix transforms.  Animations can never drive these.
        if (sExpectedJoints[curExpectedTransform].mTransformType == TRANSFORM_TYPE_MATRIX)
        {
            continue;
        }
        
        TransformRecord* transformRecord = GetTransformRecordByName(sExpectedJoints[curExpectedTransform].mTransformName);
        
        if (transformRecord == NULL)
        {
            TransformRecord* newRecord = new TransformRecord;
            
            memcpy(newRecord, &sExpectedJoints[curExpectedTransform], sizeof(TransformRecord));
            
            mTransforms.insert(mTransforms.begin() + insertIndex, newRecord);
        }
        
        insertIndex++;
    }
}

TransformRecord* Joint::GetTransformRecordByName(char* inTransformName)
{
    int numJointTransforms = mTransforms.size();

    for (int curJointTransform = 0; curJointTransform < numJointTransforms; curJointTransform++)
    {
        TransformRecord* curTransform = mTransforms.at(curJointTransform);
        
        if (strncmp(curTransform->mTransformName, inTransformName, NEON21_MODELEXPORTER_TRANSFORM_NAME_LENGTH) == 0)
        {
            return curTransform;
        }
    }
    
    return NULL;
}

int Joint::GetExpectedTransformIndex(char* inTransformName)
{
    int numExpectedTransforms = sizeof(sExpectedJoints) / sizeof(TransformRecord);
    
    for (int curExpectedTransform = 0; curExpectedTransform < numExpectedTransforms; curExpectedTransform++)
    {
        if (strncmp(inTransformName, sExpectedJoints[curExpectedTransform].mTransformName, NEON21_MODELEXPORTER_TRANSFORM_NAME_LENGTH) == 0)
        {
            return curExpectedTransform;
        }
    }
    
    // Unknown transform name.
    assert(false);
    
    return -1;
}

Skeleton::Skeleton()
{
    mRootJoint = NULL;
    mSkeletonName[0] = 0;
    mJointListGenerated = false;
}

Skeleton::~Skeleton()
{
    delete mRootJoint;
}

void Skeleton::SetRootJoint(Joint* inRootJoint)
{
    mRootJoint = inRootJoint;
}

Joint* Skeleton::GetRootJoint()
{
    return mRootJoint;
}

void Skeleton::SetSkeletonName(const char* inName)
{
    strncpy(mSkeletonName, inName, SKELETON_NAME_LENGTH);
}

char* Skeleton::GetSkeletonName()
{
    return mSkeletonName;
}

void Skeleton::BuildJointList()
{
    if (!mJointListGenerated)
    {
        mRootJoint->GenerateJointList(&mJointList);
        mJointListGenerated = true;
    }
}

std::vector<Joint*>* Skeleton::GetJointList()
{
    return &mJointList;
}

Joint* Skeleton::GetJointWithName(const char* inName)
{
    BuildJointList();
    
    int numJoints = mJointList.size();
    
    for (int curJointIndex = 0; curJointIndex < numJoints; curJointIndex++)
    {
        Joint* curJoint = mJointList.at(curJointIndex);
        char* jointName = curJoint->GetJointName();
        
        if (strcmp(inName, jointName) == 0)
        {
            return curJoint;
        }
    }
    
    mJointList.clear();
    
    return NULL;
}

Joint* Skeleton::GetJointWithEntity(FCDEntity* inEntity)
{
    BuildJointList();
    
    int numJoints = mJointList.size();
    
    for (int curJointIndex = 0; curJointIndex < numJoints; curJointIndex++)
    {
        Joint* curJoint = mJointList.at(curJointIndex);
        FCDEntity* jointEntity = curJoint->GetJointEntity();
        
        if (inEntity == jointEntity)
        {
            return curJoint;
        }
    }
    
    mJointList.clear();
    
    return NULL;
}

void Skeleton::ValidateJoints()
{
    int numJoints = mJointList.size();

    for (int curJointIndex = 0; curJointIndex < numJoints; curJointIndex++)
    {
        Joint* curJoint = mJointList.at(curJointIndex);
        
        curJoint->Validate();
    }
}