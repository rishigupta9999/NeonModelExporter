#pragma once

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <vector>
#include "NeonMath.h"
#include "ModelExporterDefines.h"

class FCDEntity;

class Joint
{
    public:
        Joint();
        ~Joint();
        
        static const int INVALID_JOINT_INDEX = -1;
        static const int JOINT_NAME_LENGTH = 64;
        
        void AddChild(Joint* inJoint);
        
        void PrintHierarchy();
        void DeleteHierarchy();
        
        int CountJoints();
        void GenerateJointList(std::vector<Joint*>* inJointList);
        
        int                     GetJointIdentifier();
        char*                   GetJointName();
        Matrix44*               GetInverseBindPoseTransform();
        std::vector<Joint*>*    GetChildren();
        std::vector<TransformRecord*>* GetTransforms();
        
        void        SetJointTransform(Matrix44* inJointTransform);
        void        SetInverseBindPose(Matrix44* inBindPose);
        void        SetJointName(const char* inJointName);
        void        AddTransform(TransformRecord* inTransform);
        
        void        SetJointEntity(FCDEntity* inEntity);
        FCDEntity*  GetJointEntity();
        
        void        SetID(int inID);
        int         GetID();
        
        void        Validate();

        TransformRecord*    GetTransformRecordByName(char* inTransformName);
        static int          GetExpectedTransformIndex(char* inTransformName);
                
    private:
        void        GenerateJointListHelper(std::vector<Joint*>* inJointList);

        int         mID;
        int         mIdentifier;
        Matrix44    mInverseBindPose;
        char        mJointName[JOINT_NAME_LENGTH];
        
        FCDEntity*  mJointEntity;
        
        static int  mNextIdentifier;
        
        std::vector<Joint*>             mChildren;
        std::vector<TransformRecord*>   mTransforms;
};

class Skeleton
{
    public:
        Skeleton();
        ~Skeleton();
        
        void SetRootJoint(Joint* inJoint);
        Joint* GetRootJoint();
        
        Joint* GetJointWithName(const char* inName);
        Joint* GetJointWithEntity(FCDEntity* inEntity);
        
        void SetSkeletonName(const char* inName);
        char* GetSkeletonName();
                
        void BuildJointList();
        std::vector<Joint*>* GetJointList();
        
        void        ValidateJoints();
                
        static const int SKELETON_NAME_LENGTH = 32;
        
    private:
        char                mSkeletonName[SKELETON_NAME_LENGTH];
        Joint*              mRootJoint;
        std::vector<Joint*> mJointList;
        bool                mJointListGenerated;
};