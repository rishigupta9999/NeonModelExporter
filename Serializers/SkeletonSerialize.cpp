#include "ModelExporterDefines.h"

#include "SkeletonSerialize.h"
#include "Skeleton.h"

#include "main.h"

void WriteSkeletons(CFMutableArrayRef inSkeletonList)
{
    int numSkeletons = CFArrayGetCount(inSkeletonList);
    
    if (numSkeletons == 0)
    {
        printf("There are no skeletons in this file that are capable of being exported, skipping.\n");
        return;
    }
    else if (numSkeletons > 1)
    {
        printf("More than one skeleton found.  Only the first one will be exported.\n");
    }
    
    Skeleton* curSkeleton = static_cast<Skeleton*>(const_cast<void*>(CFArrayGetValueAtIndex(inSkeletonList, 0)));

    chdir(gOutputDirectory);
    
    char* skeletonName = curSkeleton->GetSkeletonName();
    
    if (skeletonName[0] == 0)
    {
        curSkeleton->SetSkeletonName("ExportedSkeleton");
    }
    
    char* outputFilename = new char[strlen(skeletonName) + 6];
    
    sprintf(outputFilename, "%s.%s", skeletonName, NEON21_MODELEXPORTER_SKELETON_EXTENSION);
    
    FILE* outputFile = fopen(outputFilename, "w+");
    
    delete outputFilename;
    
    // Write out header
    
    SkeletonHeader  header;
    
    memset(&header, 0, sizeof(header));
    
    header.mMajorVersion = NEON21_MODELEXPORTER_MAJOR_VERSION;
    header.mMinorVersion = NEON21_MODELEXPORTER_MINOR_VERSION;
    
    header.mFileType = NEON21_MODEL_SKELETON;
    
    curSkeleton->BuildJointList();
    curSkeleton->ValidateJoints();
    
    std::vector<Joint*>* jointList = curSkeleton->GetJointList();
    
    header.mNumJoints = jointList->size();
    
    fwrite(&header, sizeof(SkeletonHeader), 1, outputFile);
        
    for (std::vector<Joint*>::iterator curJoint = jointList->begin(); curJoint < jointList->end(); curJoint++)
    {
        JointEntry  curEntry;
        
        memset(&curEntry, 0, sizeof(curEntry));
        
        // Doesn't matter if the name gets truncated, it's just there for debugging purposes
        strncpy(curEntry.mName, (*curJoint)->GetJointName(), NEON21_MODELEXPORTER_JOINT_NAME_LENGTH);
        
        //CloneMatrix44((*curJoint)->GetJointTransform(), &curEntry.mJointTransform);
        CloneMatrix44((*curJoint)->GetInverseBindPoseTransform(), &curEntry.mInverseBindPoseTransform);
        
        curEntry.mNumChildren = (*curJoint)->GetChildren()->size();
        
        int* childrenIndices = new int[curEntry.mNumChildren];
        
        std::vector<Joint*>* children = (*curJoint)->GetChildren();
                    
        for (int curChild = 0; curChild < curEntry.mNumChildren; curChild++)
        {
            childrenIndices[curChild] = ((*children)[curChild])->GetJointIdentifier();
        }
        
        curEntry.mJointID = (*curJoint)->GetID();
        
        std::vector<TransformRecord*>* transforms = (*curJoint)->GetTransforms();
        curEntry.mNumTransforms = transforms->size();
        
        // Write JointEntry structure
        fwrite(&curEntry, sizeof(JointEntry), 1, outputFile);
        
        // Write out all transforms
        for (std::vector<TransformRecord*>::iterator curTransform = transforms->begin(); curTransform < transforms->end(); curTransform++)
        {
            fwrite((*curTransform), sizeof(TransformRecord), 1, outputFile);
        }
        
        fwrite(childrenIndices, sizeof(int), curEntry.mNumChildren, outputFile);
                
        delete [] childrenIndices;
    }

    fclose(outputFile);
}