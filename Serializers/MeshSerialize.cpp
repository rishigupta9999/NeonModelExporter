#include "MeshSerialize.h"
#include "Mesh.h"

#include "main.h"

#include <unistd.h>

void WriteMeshes(CFMutableArrayRef inMeshList)
{
    int numMeshes = CFArrayGetCount(inMeshList);
    
    if (numMeshes == 0)
    {
        printf("There were no meshes capable of being exported.  Skipping...\n");
        return;
    }
    
    for (int curMeshIndex = 0; curMeshIndex < numMeshes; curMeshIndex++)
    {
        Mesh* curMesh = static_cast<Mesh*>(const_cast<void*>(CFArrayGetValueAtIndex(inMeshList, curMeshIndex)));
        
        char meshErrorString[Mesh::VALIDATION_STRING_LENGTH];
        
        if (curMesh == NULL)
        {
            continue;
        }
        
        bool isValid = curMesh->Validate(meshErrorString, Mesh::VALIDATION_STRING_LENGTH);
        
        if (!isValid)
        {
            printf("%s. Aborting\n", meshErrorString);
            return;
        }
        
        // Create interleaved stream.  Memory allocated by Mesh class.
        
        uint32_t        numVertices = 0;
        uint32_t        stride = 0;
        unsigned char*  streamData = NULL;
        
        curMesh->CreateInterleavedStream(&streamData, &numVertices, &stride);
        
        // Create header
        
        ModelHeader header;
        
        header.mMajorVersion = NEON21_MODELEXPORTER_MAJOR_VERSION;
        header.mMinorVersion = NEON21_MODELEXPORTER_MINOR_VERSION;
                    
        header.mPositionStride = curMesh->mPositionStride;
        header.mNormalStride = curMesh->mNormalStride;
        header.mTexcoordStride = curMesh->mTexcoordStride;
        
        header.mNumMatricesPerVertex = curMesh->GetMaxMatrixCount();
        
        if (gMaxNumWeights != 0)
        {
            header.mNumMatricesPerVertex = NeonMin(gMaxNumWeights, curMesh->GetMaxMatrixCount());
        }
        
        header.mFileType = NEON21_MODEL_MESH;
        
        header.mNumVertices = numVertices;
        
        Matrix44 bindShapeMatrix;
        curMesh->GetBindShapeMatrix(&bindShapeMatrix);
        memcpy(header.mBindShapeMatrix, bindShapeMatrix.mMatrix, sizeof(float) * 16);
        
        memcpy(header.mTextureFilename, curMesh->mTextureFilename, NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH);
        
        // Now write out everything
        chdir(gOutputDirectory);
        
        if (curMesh->mModelName[0] == 0)
        {
            curMesh->SetModelName("ExportedMesh");
        }
        
        // Write out header
        char* outputFilename = new char[strlen(curMesh->mModelName) + 5];
        
        sprintf(outputFilename, "%s.%s", curMesh->mModelName, NEON21_MODELEXPORTER_MESH_EXTENSION);
        
        FILE* outputFile = fopen(outputFilename, "w+");
        
        delete outputFilename;
        
        fwrite(&header, sizeof(ModelHeader), 1, outputFile);
        
        // Write out stream
        fwrite( streamData, stride, numVertices, outputFile  );
        
        free(streamData);
        fclose(outputFile);
    }
}