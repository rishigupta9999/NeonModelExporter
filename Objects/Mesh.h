//
//  Mesh.h
//  Neon21
//
//  Copyright Neon Games 2009. All rights reserved.
//

#pragma once

#include "ModelExporterDefines.h"
#include <stdint.h>

#include <vector>

class Mesh
{
    public:
    
        enum MeshState
        {
            MESH_STATE_PENDING,
            MESH_STATE_STARTED,
        };
    
        class MeshInfo
        {
            public:
                MeshInfo();
                ~MeshInfo();
                
                uint32_t    mNumPositionIndices;
                uint32_t*   mPositionIndices;
                
                uint32_t    mNumNormalIndices;
                uint32_t*   mNormalIndices;
                
                uint32_t    mNumTexcoordIndices;
                uint32_t*   mTexcoordIndices;

                uint32_t    mNumFaces;          // Never touched if we just get a triangle list.  Only non-zero if we have to split non-triangular primitives.
                uint32_t*   mFaceVertexCounts;  // A number for each face, for each face this specifies the number of vertices.  So 3 would be a triangle,
                                                // 4 a quad, 5 a pentagon, and so forth.
        };
        
        Mesh();
        ~Mesh();
        
        void BeginMesh();
        void EndMesh();
        
        void AddPositions(uint32_t inNumPositionElems, uint32_t inPositionStride, float* inPositionElems);
        void AddNormals(uint32_t inNumNormalElems, uint32_t inNormalStride, float* inNormalElems);
        void AddTexcoords(uint32_t inNumTexcoordElems, uint32_t inTexcoordStride, float* inTexcoordElems);
        
        void AddPositionIndices(uint32_t inNumIndices, uint32_t* inIndices);
        void AddNormalIndices(uint32_t inNumIndices, uint32_t* inIndices);
        void AddTexcoordIndices(uint32_t inNumIndices, uint32_t* inIndices);
        
        void AddMatrixCounts(uint32_t inNumCounts, uint32_t* inCounts);
        void AddMatrixIndices(uint32_t inNumIndices, uint32_t* inIndices);
        void AddMatrixWeights(uint32_t inNumWeights, float* inWeights);
        
        void AddFaceVertexInfo(uint32_t inNumFaces, const uint32_t* inFaceVertexCounts);
        
        void SetBindShapeMatrix(Matrix44* inBindShapeMatrix);
        void GetBindShapeMatrix(Matrix44* outBindShapeMatrix);
        
        uint32_t GetMaxMatrixCount();
                                        
        static const int VALIDATION_STRING_LENGTH = 512;
        bool Validate(char* outErrorString, int inErrorStringSize);
        
        void CreateInterleavedStream(unsigned char** outStream, uint32_t* outNumVertices, uint32_t* outStride);
        
        void CreateSplittedIndexBuffers(MeshInfo* inMeshInfo);
        void SplitBuffer(   uint32_t* inInputIndexBuffer, uint32_t inInputNumElems,
                            uint32_t** outOutputIndexBuffer, uint32_t* outOutputNumElems, MeshInfo* inMeshInfo);
        
        void SetModelName(const char* inName);
        
        void AddTextureName(char* inTextureName);
        
        float*      mPositionElems;
        uint32_t    mNumPositionElems;
        uint32_t    mPositionStride;
                
        float*      mNormalElems;
        uint32_t    mNumNormalElems;
        uint32_t    mNormalStride;
                
        float*      mTexcoordElems;
        uint32_t    mNumTexcoordElems;
        uint32_t    mTexcoordStride;
                
        uint32_t*   mMatrixCounts;      // Matrix / weight counts per vertex
        uint32_t    mNumMatrixCounts;   // Should be the same as the number of vertices
        
        uint32_t*   mMatrixIndices;     // Joint indices per vertex
        uint32_t    mNumMatrixIndices;  // Number of matrix indices in total (summed over all vertices)
        
        float*      mMatrixWeights;     // Bind weights per joint per vertex
        uint32_t    mNumMatrixWeights;  // Number of matrix weights in total (summed over all vertices)
        
        Matrix44    mBindShapeMatrix;
        
        std::vector<MeshInfo*> mMeshInfo;
                        
        char        mTextureFilename[NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH];
        
        MeshInfo*   mCurMeshInfo;
        MeshState   mMeshState;
        
        static const int MODEL_NAME_LENGTH = 32;
        char        mModelName[MODEL_NAME_LENGTH];
        
    private:
        struct IndexWeightPair
        {
            int arrayIndex;
            int jointIndex;
            float weight;
        };

        static int IndexWeightComparator(const void* inLeft, const void* inRight);
};