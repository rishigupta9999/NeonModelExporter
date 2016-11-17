#include "GeometryParse.h"
#include "Mesh.h"

#include <stdio.h>

#define NO_LIBXML
#include "FCollada.h"

#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsInput.h"

#include "FCDocument/FCDMaterialInstance.h"
#include "FCDocument/FCDMaterial.h"

#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectParameterSurface.h"

#include "FCDocument/FCDImage.h"

#include "main.h"

void ExtractMaterial(FCDGeometryInstance* inInstance, Mesh* inMesh)
{
    int materialInstanceCount = inInstance->GetMaterialInstanceCount();
    
    if (materialInstanceCount > 1)
    {
        printf("%d material instances were found, only the first one will be exported.\n", materialInstanceCount);
    }
    
    if (materialInstanceCount != 0)
    {
        FCDMaterialInstance* materialInstance = inInstance->GetMaterialInstance(0);
        
        if (materialInstance == NULL)
        {
            printf("No material instance was returned.  No texture references will be exported.\n");
            return;
        }
        
        FCDMaterial* material = (FCDMaterial*)materialInstance->GetEntity();
        
        if (material == NULL)
        {
            printf("Failed to get material entity.  No texture references will be exported.\n");
            return;
        }
        
        FCDEffect* effect = material->GetEffect();
        
        if (effect == NULL)
        {
            printf("No effect found for the material.  No texture references will be exported.\n");
            return;
        }
        
        if (effect->GetEffectParameterCount() != 0)
        {
            printf("Non-zero global effect parameter count.  Non profile specific effect parameters are not supported and these will be ignored.\n");
        }
        
        FCDEffectProfile* effectProfile = effect->FindProfile(FUDaeProfileType::COMMON);
        
        if (effectProfile == NULL)
        {
            printf("No common effect profile was found.  This is the only type supported.  No texture references will be exported.\n");
        }
        
        int effectParameterCount = effectProfile->GetEffectParameterCount();
        
        FCDEffectParameterSurface* surfaceParam = NULL;
        
        for (int i = 0; i < effectParameterCount; i++)
        {
            FCDEffectParameter* effectParameter = effectProfile->GetEffectParameter(i);
            
            if (effectParameter->GetType() == FCDEffectParameter::SURFACE)
            {
                surfaceParam = (FCDEffectParameterSurface*)effectParameter;
                break;
            }
        }
        
        if (surfaceParam == NULL)
        {
            printf("No surface parameter was found.  No texture references will be exported.\n");
            return;
        }
        
        int imageCount = surfaceParam->GetImageCount();
        
        if (imageCount == 0)
        {
            printf("No images found in the surface.  No texture references will be exported.\n");
            return;
        }
        else if (imageCount > 1)
        {
            printf("More than one image was found in the surface.  Only the first will be exported.\n");
        }
        
        FCDImage* image = surfaceParam->GetImage(0);
        char* filePath = (char*)image->GetFilename().c_str();
        
        int fileNameLength = strlen(filePath);
        char* fileName = (char*)filePath;
        
        for (int i = (fileNameLength - 1); i >= 0; i--)
        {
            if (filePath[i] == '/')
            {
                fileName = &filePath[i + 1];
                break;
            }
        }
        
        if (strlen(fileName) >= NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH)
        {
            printf("%s is too long of a filename, %d is the limit.  Please reduce to at most this level and try again.\n", fileName, NEON21_MODELEXPORTER_TEXTURE_NAME_LENGTH - 1);
            return;
        }
        
        inMesh->AddTextureName(fileName);
    }
}

// If inInstance is specified, then inGeometry will be ignored.
// Otherwise inInstance should be NULL and inGeometry will be used.
Mesh* ReadGeometry(FCDGeometryInstance* inInstance, FCDGeometry* inGeometry)
{
    FCDGeometry* geometry = NULL;
    Mesh* curMesh;
    
    if (inInstance != NULL)
    {
        geometry = (FCDGeometry*)inInstance->GetEntity();
    }
    else
    {
        geometry = inGeometry;
    }
        
    if (!geometry->IsMesh())
    {
        printf("%s geometry is not a mesh, skipping.\n", geometry->GetName().c_str());
        return NULL;
    }
    
    FCDGeometryMesh* mesh = geometry->GetMesh();
    
    FCDGeometrySource* positionSource = mesh->FindSourceByType(FUDaeGeometryInput::POSITION);
    FCDGeometrySource* normalSource = mesh->FindSourceByType(FUDaeGeometryInput::NORMAL);
    FCDGeometrySource* texcoordSource = mesh->FindSourceByType(FUDaeGeometryInput::TEXCOORD);
    
    if (positionSource == NULL)
    {
        printf("No position data in mesh, skipping.\n");
        return NULL;
    }
    else
    {
        curMesh = new Mesh;
        
        if (positionSource != NULL)
        {
            curMesh->AddPositions(positionSource->GetDataCount(), positionSource->GetStride(), positionSource->GetData());
        }
        
        if (normalSource != NULL)
        {
            curMesh->AddNormals(normalSource->GetDataCount(), normalSource->GetStride(), normalSource->GetData());
        }
        
        if (texcoordSource != NULL)
        {
            curMesh->AddTexcoords(texcoordSource->GetDataCount(), texcoordSource->GetStride(), texcoordSource->GetData());
        }
        
        curMesh->SetModelName(geometry->GetName().c_str());
    }

    int numPolySources = mesh->GetPolygonsCount();
    
    FCDGeometryPolygonsInput* positionInput = NULL;
    FCDGeometryPolygonsInput* normalInput = NULL;
    FCDGeometryPolygonsInput* texcoordInput = NULL;
    FCDGeometryPolygons* polygons = NULL;
    
    bool needsSplitting = false;
    
    if (numPolySources < 1)
    {
        printf("There are no polygon sources in this mesh, skipping.\n");
        goto cleanup;
    }

    for (int curMeshSource = 0; curMeshSource < numPolySources; curMeshSource++)
    {
        polygons = mesh->GetPolygons(curMeshSource);
        
        if (polygons->GetPrimitiveType() != FCDGeometryPolygons::POLYGONS)
        {
            printf("Primitive type is something other than Polygons.  Skipping\n");
            goto cleanup;
        }
            
        if (!mesh->IsTriangles())
        {
            // If we didn't get triangles, we'll need to convert the streams into triangles
            needsSplitting = true;
        }
        
        positionInput = polygons->FindInput(FUDaeGeometryInput::POSITION);
        normalInput = polygons->FindInput(FUDaeGeometryInput::NORMAL);
        texcoordInput = polygons->FindInput(FUDaeGeometryInput::TEXCOORD);
        
        if (positionInput == NULL)
        {
            printf("No position indices found.  Skipping this mesh.\n");
            goto cleanup;
        }
        else
        {
            curMesh->BeginMesh();

            if (needsSplitting)
            {
                FCDGeometryPolygons::PrimitiveType primitiveType = polygons->GetPrimitiveType();
                
                // We only know how to split polygon lists.  Bail if we got something else (eg: triangle strips, fans, etc)
                if (primitiveType != FCDGeometryPolygons::POLYGONS)
                {
                    printf("Splitting polygon lists is the only type of splitting that's supported.  Aborting this mesh.\n");
                    goto cleanup;
                }
                else
                {
                    // Split index list so that primitive types are triangles instead of polygons
                    
                    uint32_t numFaces = polygons->GetFaceVertexCountCount();
                    const uint32_t* faceVertexCounts = polygons->GetFaceVertexCounts();
                    
                    curMesh->AddFaceVertexInfo(numFaces, faceVertexCounts);
                }
            }
            
            if (positionInput != NULL)
            {
                curMesh->AddPositionIndices(positionInput->GetIndexCount(), positionInput->GetIndices());
            }
            
            if (normalInput != NULL)
            {
                curMesh->AddNormalIndices(normalInput->GetIndexCount(), normalInput->GetIndices());
            }
            
            if (texcoordInput != NULL)
            {
                curMesh->AddTexcoordIndices(texcoordInput->GetIndexCount(), texcoordInput->GetIndices());
            }
            
            curMesh->EndMesh();
        }
    }
    
    if (inInstance != NULL)
    {
        ExtractMaterial(inInstance, curMesh);
    }

    return curMesh;
    
cleanup:
    delete curMesh;
    return NULL;
}
