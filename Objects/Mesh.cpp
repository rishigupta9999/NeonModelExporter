//
//  Mesh.cpp
//  Neon21
//
//  Copyright Neon Games 2009. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>

#include "Mesh.h"
#include "main.h"

Mesh::MeshInfo::MeshInfo()
{
    mNumPositionIndices = 0;
    mPositionIndices = NULL;
    
    mNumNormalIndices = 0;
    mNormalIndices = NULL;
    
    mNumTexcoordIndices = 0;
    mTexcoordIndices = NULL;
    
    mNumFaces = 0;
    mFaceVertexCounts = NULL;
}

Mesh::MeshInfo::~MeshInfo()
{
    delete mPositionIndices;
    delete mNormalIndices;
    delete mTexcoordIndices;
    delete mFaceVertexCounts;
}

Mesh::Mesh()
{
    mNumPositionElems = 0;
    mPositionElems = NULL;
    mPositionStride = 0;
        
    mNumNormalElems = 0;
    mNormalElems = NULL;
    mNormalStride = 0;
        
    mNumTexcoordElems = 0;
    mTexcoordElems = NULL;
    mTexcoordStride = 0;
        
    mNumMatrixCounts = 0;
    mMatrixCounts = NULL;
    
    mNumMatrixIndices = 0;
    mMatrixIndices = NULL;
    
    mNumMatrixWeights = 0;
    mMatrixWeights = 0;
        
    memset(mTextureFilename, 0, NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH);
    
    mModelName[0] = 0;
    
    mMeshState = MESH_STATE_PENDING;
    mCurMeshInfo = NULL;
    
    SetIdentity(&mBindShapeMatrix);
}

Mesh::~Mesh()
{
    delete mPositionElems;
    delete mNormalElems;
    delete mTexcoordElems;
        
    delete mMatrixCounts;
    delete mMatrixIndices;
    delete mMatrixWeights;
    
    assert(mCurMeshInfo == NULL);
}

void Mesh::BeginMesh()
{
    mMeshState = MESH_STATE_STARTED;
    
    mCurMeshInfo = new MeshInfo;
}

void Mesh::EndMesh()
{
    mMeshInfo.push_back(mCurMeshInfo);
    
    mMeshState = MESH_STATE_PENDING;
    
    mCurMeshInfo = NULL;
}

void Mesh::AddPositions(uint32_t inNumPositionElems, uint32_t inPositionStride, float* inPositionElems)
{
    if (mPositionElems != NULL)
    {
        delete mPositionElems;
    }
    
    mNumPositionElems = inNumPositionElems;
    mPositionStride = inPositionStride;
    
    mPositionElems = new float[inNumPositionElems];
    
    memcpy(mPositionElems, inPositionElems, sizeof(float) * inNumPositionElems);
}

void Mesh::AddNormals(uint32_t inNumNormalElems, uint32_t inNormalStride, float* inNormalElems)
{
    if (mNormalElems != NULL)
    {
        delete mNormalElems;
    }
    
    mNumNormalElems = inNumNormalElems;
    mNormalStride = inNormalStride;
    
    mNormalElems = new float[inNumNormalElems];
    
    memcpy(mNormalElems, inNormalElems, sizeof(float) * inNumNormalElems);
}

void Mesh::AddTexcoords(uint32_t inNumTexcoordElems, uint32_t inTexcoordStride, float* inTexcoordElems)
{
    if (mTexcoordElems != NULL)
    {
        delete mTexcoordElems;
    }
    
    mNumTexcoordElems = inNumTexcoordElems;
    mTexcoordStride = inTexcoordStride;
    
    mTexcoordElems = new float[inNumTexcoordElems];
    
    memcpy(mTexcoordElems, inTexcoordElems, sizeof(float) * inNumTexcoordElems);
    
    for (int i = 1; i < inNumTexcoordElems; i+=2)
    {
        mTexcoordElems[i] = 1.0 - mTexcoordElems[i];
    }
}

void Mesh::AddPositionIndices(uint32_t inNumIndices, uint32_t* inIndices)
{
    assert(mCurMeshInfo != NULL);
    
    if (mCurMeshInfo->mPositionIndices != NULL)
    {
        delete mCurMeshInfo->mPositionIndices;
    }
    
    mCurMeshInfo->mNumPositionIndices = inNumIndices;
    mCurMeshInfo->mPositionIndices = new uint32_t[inNumIndices];
    
    memcpy(mCurMeshInfo->mPositionIndices, inIndices, sizeof(uint32_t) * inNumIndices);
}

void Mesh::AddNormalIndices(uint32_t inNumIndices, uint32_t* inIndices)
{
    assert(mCurMeshInfo != NULL);
    
    if (mCurMeshInfo->mNormalIndices != NULL)
    {
        delete mCurMeshInfo->mNormalIndices;
    }
    
    mCurMeshInfo->mNumNormalIndices = inNumIndices;
    mCurMeshInfo->mNormalIndices = new uint32_t[inNumIndices];
    
    memcpy(mCurMeshInfo->mNormalIndices, inIndices, sizeof(uint32_t) * inNumIndices);
}

void Mesh::AddTexcoordIndices(uint32_t inNumIndices, uint32_t* inIndices)
{
    assert(mCurMeshInfo != NULL);
    
    if (mCurMeshInfo->mTexcoordIndices != NULL)
    {
        delete mCurMeshInfo->mTexcoordIndices;
    }
    
    mCurMeshInfo->mNumTexcoordIndices = inNumIndices;
    mCurMeshInfo->mTexcoordIndices = new uint32_t[inNumIndices];
    
    memcpy(mCurMeshInfo->mTexcoordIndices, inIndices, sizeof(uint32_t) * inNumIndices);
}

void Mesh::AddMatrixCounts(uint32_t inNumCounts, uint32_t* inCounts)
{
    if (mMatrixCounts != NULL)
    {
        delete mMatrixCounts;
    }
    
    mNumMatrixCounts = inNumCounts;
    mMatrixCounts = new uint32_t[inNumCounts];
    
    memcpy(mMatrixCounts, inCounts, sizeof(uint32_t) * inNumCounts);
}

void Mesh::AddMatrixIndices(uint32_t inNumIndices, uint32_t* inIndices)
{
    if (mMatrixIndices != NULL)
    {
        delete mMatrixIndices;
    }
    
    mNumMatrixIndices = inNumIndices;
    mMatrixIndices = new uint32_t[inNumIndices];
    
    memcpy(mMatrixIndices, inIndices, sizeof(uint32_t) * inNumIndices);
}

void Mesh::AddMatrixWeights(uint32_t inNumWeights, float* inWeights)
{
    if (mMatrixWeights != NULL)
    {
        delete mMatrixWeights;
    }
    
    mNumMatrixWeights = inNumWeights;
    mMatrixWeights = new float[inNumWeights];
    
    memcpy(mMatrixWeights, inWeights, sizeof(float) * inNumWeights);
}

uint32_t Mesh::GetMaxMatrixCount()
{
    int maxCount = 0;
    
    for (int i = 0; i < mNumMatrixCounts; i++)
    {
        if (mMatrixCounts[i] > maxCount)
        {
            maxCount = mMatrixCounts[i];
        }
    }
    
    return maxCount;
}

bool Mesh::Validate(char* outErrorString, int inErrorStringSize)
{
    // Validate strides
    
    if (mNumPositionElems != 0)
    {
        if (mPositionStride == 0)
        {
            snprintf(outErrorString, inErrorStringSize, "Non-zero number of position elements, but the position stride is zero.  This is not allowed.\n");
            return false;
        }
    }
    
    if (mNumNormalElems != 0)
    {
        if (mNormalStride == 0)
        {
            snprintf(outErrorString, inErrorStringSize, "Non-zero number of normal elements, but the normal stride is zero.  This is not allowed.\n");
            return false;
        }
    }

    if (mNumTexcoordElems != 0)
    {
        if (mTexcoordStride == 0)
        {
            snprintf(outErrorString, inErrorStringSize, "Non-zero number of texcoord elements, but the texcoord stride is zero.  This is not allowed.\n");
            return false;
        }
    }
    
    if (mNumMatrixCounts != 0)
    {
        if (mNumMatrixCounts != (mNumPositionElems / mPositionStride))
        {
            snprintf(outErrorString, inErrorStringSize, "Number of matrix counts don't match the number of vertices.\n");
            return false;
        }
        
        int calculatedNumJoints = 0;
        
        for (int i = 0; i < mNumMatrixCounts; i++)
        {
            calculatedNumJoints += mMatrixCounts[i];
        }
        
        if (calculatedNumJoints != mNumMatrixIndices)
        {
            snprintf(outErrorString, inErrorStringSize, "Number of matrices implied by the matrix counts doesn't match the number of matrix incides.\n");
            return false;
        }
        
        if (calculatedNumJoints != mNumMatrixWeights)
        {
            snprintf(outErrorString, inErrorStringSize, "Number of matrices implied by the matrix counts doesn't match the number of weights.\n");
            return false;
        }
    }

    // Ensure there are no partial vectors in the streams
    
    int positionElemRemainder = (mPositionStride == 0) ? 0 : mNumPositionElems % mPositionStride;
    int normalElemRemainder = (mNormalStride == 0) ? 0 : mNumNormalElems % mNormalStride;
    int texcoordElemRemainder = (mTexcoordStride == 0) ? 0 : mNumTexcoordElems % mTexcoordStride;
    
    if (positionElemRemainder != 0)
    {
        snprintf(outErrorString, inErrorStringSize, "Non-integral number of position vectors in the position stream.\n");
        return false;
    }
    
    if (normalElemRemainder != 0)
    {
        snprintf(outErrorString, inErrorStringSize, "Non-integral number of normal vectors in the normal stream.\n");
        return false;
    }


    if (texcoordElemRemainder != 0)
    {
        snprintf(outErrorString, inErrorStringSize, "Non-integral number of texcoord vectors in the texcoord stream.\n");
        return false;
    }

    // Ensure the indices never go out of range
    
    int numPositionVectors = (mPositionStride == 0) ? 0 : mNumPositionElems / mPositionStride;
    int numNormalVectors = (mNormalStride == 0) ? 0 : mNumNormalElems / mNormalStride;
    int numTexcoordVectors = (mTexcoordStride == 0) ? 0 : mNumTexcoordElems / mTexcoordStride;
    
    int numMeshInfo = mMeshInfo.size();
    
    for (int curMeshInfoIndex = 0; curMeshInfoIndex < numMeshInfo; curMeshInfoIndex++)
    {
        MeshInfo* curMeshInfo = mMeshInfo.at(curMeshInfoIndex);
        
        for (int i = 0; i < curMeshInfo->mNumPositionIndices; i++)
        {
            if (curMeshInfo->mPositionIndices[i] >= numPositionVectors)
            {
                snprintf(outErrorString, inErrorStringSize, "Position index out of range.\n");
                return false;
            }
        }
        
        for (int i = 0; i < curMeshInfo->mNumNormalIndices; i++)
        {
            if (curMeshInfo->mNormalIndices[i] >= numNormalVectors)
            {
                snprintf(outErrorString, inErrorStringSize, "Normal index out of range.\n");
                return false;
            }
        }
        
        for (int i = 0; i < curMeshInfo->mNumTexcoordIndices; i++)
        {
            if (curMeshInfo->mTexcoordIndices[i] >= numTexcoordVectors)
            {
                snprintf(outErrorString, inErrorStringSize, "Texcoord index out of range.\n");
                return false;
            }
        }

        // Ensure that the number of indices for each attribute type are the same
        
        int compare[3];
        
        compare[0] = curMeshInfo->mNumPositionIndices;
        compare[1] = curMeshInfo->mNumNormalIndices;
        compare[2] = curMeshInfo->mNumTexcoordIndices;
        
        bool compareSuccess = true;
        
        for (int left = 0; left < 3; left++)
        {
            for (int right = 0; right < 3; right++)
            {
                int leftVal = compare[left];
                int rightVal = compare[right];
                
                if ((leftVal != 0) && (rightVal != 0))
                {
                    if (leftVal != rightVal)
                    {
                        compareSuccess = false;
                        goto loopExit;
                    }
                }
            }
        }
        
loopExit:
        if (!compareSuccess)
        {
            snprintf(outErrorString, inErrorStringSize, "Unequal numbers of position, normal, and texcoord indices.\n");
            return false;
        }
        
        // For .dae files that were exported as polygon lists, ensure that we consume all vertex indices.
        // For example, if we have a faceVertex list of "4" (that is, one polygon with 4 vertices) and then
        // we were provided with 3, or 5 (or some number other than 4) vertices, that is invalid.
        
        if (curMeshInfo->mFaceVertexCounts != NULL)
        {
            int totalFaceVertexCount = 0;
            
            for (int i = 0; i < curMeshInfo->mNumFaces; i++)
            {
                totalFaceVertexCount += curMeshInfo->mFaceVertexCounts[i];
            }
            
            if (    (totalFaceVertexCount != curMeshInfo->mNumPositionIndices) || (totalFaceVertexCount != curMeshInfo->mNumNormalIndices) ||
                    (totalFaceVertexCount != curMeshInfo->mNumTexcoordIndices)   )
            {
                snprintf(outErrorString, inErrorStringSize, 
                            "Total face vertex count is not equal to one of the attribute counts.  That should not happen.\n");
                return false;
            }
        }
    
        // If exportIndexed is on, then verify that position, normal, and texcoord indices are the same
        if (gExportIndexed)
        {
            bool needsDeindexing = false;
            
            if ((curMeshInfo->mNumPositionIndices != curMeshInfo->mNumNormalIndices) || (curMeshInfo->mNumPositionIndices != curMeshInfo->mNumTexcoordIndices))
            {
                needsDeindexing = true;
            }
            else
            {
                int numIndices = curMeshInfo->mNumPositionIndices;
                
                for (int curIndex = 0; curIndex < numIndices; curIndex++)
                {
                    int positionIndex = curMeshInfo->mPositionIndices[curIndex];
                    int normalIndex = curMeshInfo->mNormalIndices[curIndex];
                    int texcoordIndex = curMeshInfo->mTexcoordIndices[curIndex];
                    
                    if ((positionIndex != normalIndex) || (positionIndex != texcoordIndex))
                    {
                        needsDeindexing = true;
                    }
                }
            }
            
            if (needsDeindexing)
            {
                snprintf(outErrorString, inErrorStringSize, "Exported indexed was set, but the index stream needs to be deindexed (same indexes for position, normal and texcoord)");
                return false;
            }
        }
    }
    
    
    return true;
}

void Mesh::AddFaceVertexInfo(uint32_t inNumFaces, const uint32_t* inFaceVertexCounts)
{
    assert(mMeshState != MESH_STATE_PENDING);
    
    mCurMeshInfo->mNumFaces = inNumFaces;
    mCurMeshInfo->mFaceVertexCounts = new uint32_t[mCurMeshInfo->mNumFaces];
    
    memcpy(mCurMeshInfo->mFaceVertexCounts, inFaceVertexCounts, sizeof(uint32_t) * mCurMeshInfo->mNumFaces);
}

void Mesh::SetBindShapeMatrix(Matrix44* inBindShapeMatrix)
{
    CloneMatrix44(inBindShapeMatrix, &mBindShapeMatrix);
}

void Mesh::GetBindShapeMatrix(Matrix44* outBindShapeMatrix)
{
    CloneMatrix44(&mBindShapeMatrix, outBindShapeMatrix);
}

void Mesh::CreateInterleavedStream(unsigned char** outStream, uint32_t* outNumVertices, uint32_t* outStride)
{
    assert(mMeshState == MESH_STATE_PENDING);
    
    *outNumVertices = 0;
    
    for (std::vector<MeshInfo*>::iterator curMeshInfoIterator = mMeshInfo.begin(); curMeshInfoIterator != mMeshInfo.end(); curMeshInfoIterator++)
    {
        if ((*curMeshInfoIterator)->mNumFaces != 0)
        {
            CreateSplittedIndexBuffers(*curMeshInfoIterator);
        }
        
        *outNumVertices += (*curMeshInfoIterator)->mNumPositionIndices;
    }
            
    int matrixCount = GetMaxMatrixCount();
    
    if ((gMaxNumWeights != 0) && (matrixCount > gMaxNumWeights))
    {
        matrixCount = gMaxNumWeights;
    }
    
    // Position elems, normal elems, texcoord elems, and number of weights are all floats
    // Then we have matrix indices that are unsigned bytes
    int totalStride = (mPositionStride + mNormalStride + mTexcoordStride + matrixCount) * sizeof(float) + matrixCount;
    
    // 4 byte align the stride
    totalStride = (totalStride + 3) & 0xFFFFFFFC;
    
    *outStride = totalStride;
    
    *outStream = new unsigned char[totalStride * (*outNumVertices)];
    memset(*outStream, 0, totalStride * (*outNumVertices));
    
    uint32_t* matrixOffsetList = NULL;
    
    if (mNumMatrixCounts != 0)
    {
        matrixOffsetList = new uint32_t[mNumMatrixCounts];
        int curOffset = 0;

        for (int i = 0; i < mNumMatrixCounts; i++)
        {
            matrixOffsetList[i] = curOffset;
            
            curOffset += mMatrixCounts[i];
        }
    }
    
    int numMeshes = mMeshInfo.size();
    int vertexWriteIndex = 0;
    
    for (int curMesh = 0; curMesh < numMeshes; curMesh++)
    {
        MeshInfo* curMeshInfo = mMeshInfo.at(curMesh);
                
        for (int curVertex = 0; curVertex < curMeshInfo->mNumPositionIndices; curVertex++)
        {
            unsigned char* vertexWritePtr = (*outStream) + (vertexWriteIndex * totalStride);
                    
            if (mPositionStride != 0)
            {
                memcpy(vertexWritePtr, &mPositionElems[curMeshInfo->mPositionIndices[curVertex] * mPositionStride], sizeof(float) * mPositionStride);
                vertexWritePtr += (mPositionStride * sizeof(float));
            }
            
            if (mNormalStride != 0)
            {
                memcpy(vertexWritePtr, &mNormalElems[curMeshInfo->mNormalIndices[curVertex] * mNormalStride], sizeof(float) * mNormalStride);
                vertexWritePtr += (mNormalStride * sizeof(float));
            }
            
            if (mTexcoordStride != 0)
            {
                memcpy(vertexWritePtr, &mTexcoordElems[curMeshInfo->mTexcoordIndices[curVertex] * mTexcoordStride], sizeof(float) * mTexcoordStride);
                vertexWritePtr += (mTexcoordStride * sizeof(float));
            }
            
            if (mNumMatrixCounts != 0)
            {
                int index = curMeshInfo->mPositionIndices[curVertex];
                IndexWeightPair* indexWeightPairs = NULL;
                
                // Copy matrix weights
				
				// Sanitize output, make sure we aren't writing vertices with no influence whatsoever.  This can lead
				// to simulator crashes, or GPU driver crashes.
				float weightSum = 0.0f;
				
				for (int i = 0; i < mMatrixCounts[index]; i++)
				{
					weightSum += mMatrixWeights[matrixOffsetList[index] + i];
				}
								                
                if ((gMaxNumWeights == 0) || (mMatrixCounts[index] <= matrixCount))
                {
					if (weightSum == 0.0f)
					{
						// Skin to root.  1 influence with a weight of 1.0f
												
						((float*)vertexWritePtr)[0] = 1.0f;
						vertexWritePtr += sizeof(float);
						
						mMatrixCounts[index] = 1;
					}
					else
					{
						memcpy(vertexWritePtr, &mMatrixWeights[matrixOffsetList[index]], mMatrixCounts[index] * sizeof(float));
						vertexWritePtr += (mMatrixCounts[index] * sizeof(float));
					}
                }
                else
                {
					if (weightSum == 0.0f)
					{
						((float*)vertexWritePtr)[0] = 1.0f;
						vertexWritePtr += sizeof(float);
						
						mMatrixCounts[index] = 1;
					}
					else
					{
						// Find the matrixCount smallest influences
											
						indexWeightPairs = new IndexWeightPair[mMatrixCounts[index]];
						float* weightsBase = &mMatrixWeights[matrixOffsetList[index]];
						unsigned int* jointIndicesBase = &mMatrixIndices[matrixOffsetList[index]];

						// First copy all indices and weights
						
						for (int curIndex = 0; curIndex < mMatrixCounts[index]; curIndex++)
						{
							indexWeightPairs[curIndex].arrayIndex = curIndex;
							indexWeightPairs[curIndex].jointIndex = jointIndicesBase[curIndex];
							indexWeightPairs[curIndex].weight = weightsBase[curIndex];
						}
						
						qsort(indexWeightPairs, mMatrixCounts[index], sizeof(IndexWeightPair), IndexWeightComparator);
						
						// indexWeightPairs is now sorted in descending order
						
						float* weightWriteBase = (float*)vertexWritePtr;
						
						// Calculate scale factor
						
						float sum = 0;
						
						for (int curIndex = 0; curIndex < matrixCount; curIndex++)
						{
							sum += indexWeightPairs[curIndex].weight;
						}
						
						float scaleFactor = 1.0f / sum;
						
						for (int curIndex = 0; curIndex < matrixCount; curIndex++)
						{
							weightWriteBase[curIndex] = indexWeightPairs[curIndex].weight * scaleFactor;
							vertexWritePtr += sizeof(float);
						}
					}
                }
                                
                // Padding (if necessary)
                if (mMatrixCounts[index] < matrixCount)
                {
                    memset(vertexWritePtr, 0, (matrixCount - mMatrixCounts[index]) * sizeof(float));
                    vertexWritePtr += ((matrixCount - mMatrixCounts[index]) * sizeof(float));
                }
                
                // Copy matrix indices
                for (int curIndex = 0; curIndex < matrixCount; curIndex++)
                {
                    if (curIndex < mMatrixCounts[index])
                    {
                        if (indexWeightPairs == NULL)
                        {
                            *vertexWritePtr = mMatrixIndices[matrixOffsetList[index] + curIndex];
                        }
                        else
                        {
                            *vertexWritePtr = indexWeightPairs[curIndex].jointIndex;
                        }
                    }
                    else
                    {
                        *vertexWritePtr = 0;
                    }
                    
                    *vertexWritePtr++;
                }
                
                delete [] indexWeightPairs;
            }
            
            vertexWriteIndex++;
        }
    }
    
    delete [] matrixOffsetList;
}

int Mesh::IndexWeightComparator(const void* inLeft, const void* inRight)
{
    IndexWeightPair* left = (IndexWeightPair*)inLeft;
    IndexWeightPair* right = (IndexWeightPair*)inRight;
    
    if (left->weight < right->weight)
    {
        return 1;
    }
    else if (right->weight < left->weight)
    {
        return -1;
    }
    
    return 0;
}

void Mesh::CreateSplittedIndexBuffers(MeshInfo* inMeshInfo)
{
    uint32_t* splittedPositionBuffer;
    uint32_t  numSplittedPositionElements;
    
    SplitBuffer(inMeshInfo->mPositionIndices, inMeshInfo->mNumPositionIndices, &splittedPositionBuffer, &numSplittedPositionElements, inMeshInfo);
    
    inMeshInfo->mNumPositionIndices = numSplittedPositionElements;
    
    delete inMeshInfo->mPositionIndices;
    inMeshInfo->mPositionIndices = splittedPositionBuffer;
    
    
    uint32_t* splittedNormalBuffer;
    uint32_t  numSplittedNormalElements;
    
    SplitBuffer(inMeshInfo->mNormalIndices, inMeshInfo->mNumNormalIndices, &splittedNormalBuffer, &numSplittedNormalElements, inMeshInfo);
    
    inMeshInfo->mNumNormalIndices = numSplittedNormalElements;
    
    delete inMeshInfo->mNormalIndices;
    inMeshInfo->mNormalIndices = splittedNormalBuffer;
    
    
    uint32_t* splittedTexcoordBuffer;
    uint32_t  numSplittedTexcoordElements;
    
    SplitBuffer(inMeshInfo->mTexcoordIndices, inMeshInfo->mNumTexcoordIndices, &splittedTexcoordBuffer, &numSplittedTexcoordElements, inMeshInfo);
    
    inMeshInfo->mNumTexcoordIndices = numSplittedTexcoordElements;
    
    delete inMeshInfo->mTexcoordIndices;
    inMeshInfo->mTexcoordIndices = splittedTexcoordBuffer;
}

void Mesh::SplitBuffer( uint32_t* inInputIndexBuffer, uint32_t inInputNumElems,
                        uint32_t** outOutputIndexBuffer, uint32_t* outOutputNumElems, MeshInfo* inMeshInfo    )
{
    uint32_t numVertices = 0;
    
    for (int curFace = 0; curFace < inMeshInfo->mNumFaces; curFace++)
    {
        uint32_t faceVertexCount = inMeshInfo->mFaceVertexCounts[curFace];
        
        // 1 triangle for the first 3 vertices, and then one more triangle for every vertex thereafter.
        // assume all faces are convex (if the Collada exporter wants to be insane and give us concave
        // faces, there's not much I can do - exportTriangles should be turned on for all practical
        // cases anyway).  This formula / algorithm will not work for concave polygons.
        
        int numTriangles = (faceVertexCount - 3) + 1;
        
        numVertices += (numTriangles * 3);
    }
    
    *outOutputNumElems = numVertices;
    *outOutputIndexBuffer = new uint32_t[numVertices];
    
    memset(*outOutputIndexBuffer, 0, sizeof(uint32_t) * numVertices);
    
    uint32_t* outputIndexBuffer = *outOutputIndexBuffer;
    
    uint32_t numWrittenVertices = 0;
    uint32_t numReadVertices = 0;
    
    for (int curFace = 0; curFace < inMeshInfo->mNumFaces; curFace++)
    {
        uint32_t faceVertexCount = inMeshInfo->mFaceVertexCounts[curFace];

        int numTriangles = (faceVertexCount - 3) + 1;

        for (int curTriangle = 0; curTriangle < numTriangles; curTriangle++)
        {
            uint32_t* writeBase = &outputIndexBuffer[numWrittenVertices + (curTriangle * 3)];
            uint32_t* readBase = &inInputIndexBuffer[numReadVertices];
            
            // Ensure we're not about to write out of bounds
            assert( (numWrittenVertices + (curTriangle * 3)) <= numVertices );
            
            writeBase[0] = readBase[0];
            writeBase[1] = readBase[curTriangle + 1];
            writeBase[2] = readBase[curTriangle + 2];
        }
        
        numReadVertices += faceVertexCount;
        numWrittenVertices += (numTriangles * 3);
    }
}

void Mesh::SetModelName(const char* inName)
{
    strncpy(mModelName, inName, MODEL_NAME_LENGTH);
}

void Mesh::AddTextureName(char* inTextureName)
{
    strncpy(mTextureFilename, inTextureName, NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH);
}
