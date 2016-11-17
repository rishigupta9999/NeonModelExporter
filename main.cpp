//
//  main.cpp
//  Neon21
//
//  Copyright Neon Games 2009. All rights reserved.
//

#include <stdio.h>

#define NO_LIBXML
#include "FCollada.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDSceneNode.h"

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
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterSurface.h"

#include "FCDocument/FCDImage.h"

#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDController.h"

#include "Objects/Mesh.h"
#include "Objects/Animation.h"
#include "Objects/AnimationClip.h"

#include "ModelExporterDefines.h"

#include "GeometryParse.h"
#include "ControllerParse.h"
#include "AnimationClipParse.h"

#include "MeshSerialize.h"
#include "SkeletonSerialize.h"
#include "AnimationClipSerialize.h"

#include "Logging.h"

#include <sys/stat.h>

#include "main.h"

char* gInputFile = NULL;
char* gOutputDirectory = NULL;

FUObjectRef<FCDocument>     gDocument = NULL;
CFMutableArrayRef           gMeshList = NULL;
CFMutableArrayRef           gSkeletonList = NULL;
CFMutableArrayRef           gAnimationClipList = NULL;
CFMutableArrayRef			gExportRestrictionList = NULL;
int                         gMaxNumWeights = 0;
bool                        gMinMessageLevel = 1;
bool                        gExportIndexed = 0;

static bool                 gAnimationsFound = false;

static const char* ENTITY_NAMES[FCDEntity::TYPE_COUNT] = {  "Entity",
                                                            "Animation",
                                                            "Animation Clip",
                                                            "Camera",
                                                            "Light",
                                                            "Image",
                                                            "Material",
                                                            "Effect",
                                                            "Geometry",
                                                            "Controller",
                                                            "Scene Node",
                                                            "Physics Rigid Constraint",
                                                            "Physics Material",
                                                            "Physics Rigid Body",
                                                            "Physics Shape",
                                                            "Physics Analytical Geometry",
                                                            "Physics Model",
                                                            "Physics Scene Node",
                                                            "Force Field",
                                                            "Emitter"   };

void ExtractInstances(FCDSceneNode* inRoot, fm::pvector<FCDEntityInstance>* inInstances);

bool RequireSetting(const char* inComments, const char* inSetting, const char* inRequiredValue)
{
    char* settingPtr = strstr(inComments, inSetting);
    
    if (!settingPtr)
    {
        printf("Export setting %s not found, proceeding with the assumption that it is set to %s\n", inSetting, inRequiredValue);
        return true;
    }
    else
    {
        settingPtr += strlen(inSetting) + 1;
        
        if (memcmp(settingPtr, inRequiredValue, strlen(inRequiredValue)) != 0)
        {
            printf("Export setting %s should be %s, instead it is %c.  Aborting.\n", inSetting, inRequiredValue, *settingPtr);
            exit(0);
        }
    }
    
    return true;
}

bool ValidateExportSettings()
{
    FCDAsset* asset = gDocument->GetAsset();
    
    int contributorCount = asset->GetContributorCount();
    
    for (int curContributor = 0; curContributor < contributorCount; curContributor++)
    {
        FCDAssetContributor* contributor = asset->GetContributor(curContributor);
        
        const char* comments = contributor->GetComments().c_str();
        
        RequireSetting(comments, "exportTriangles", "1");
    }
    
    return true;
}

bool Init()
{
    struct stat fileInfo;
    
    int success = stat(gInputFile, &fileInfo);

    if (success == -1)
    {
        printf("There was an error reading the file %s.  Most likely it was not found.\n", gInputFile);
        exit(0);
    }
    
    FCollada::Initialize();
    
    gDocument = FCollada::NewTopDocument();
    bool retVal = FCollada::LoadDocumentFromFile(gDocument, gInputFile);
    
    if (!retVal)
    {
        printf("The Collada file could not be parsed, nothing will be done.\n");
        exit(0);
    }
    
    retVal = ValidateExportSettings();
    
    if (!retVal)
    {
        exit(0);
    }
    
    gMeshList = CFArrayCreateMutable(NULL, 0, NULL);
    gSkeletonList = CFArrayCreateMutable(NULL, 0, NULL);
    gAnimationClipList = CFArrayCreateMutable(NULL, 0, NULL);
    
    return retVal;
}

bool ReadSceneLibrary()
{
    bool retVal = true;

	FCDVisualSceneNodeLibrary*  vsl = gDocument->GetVisualSceneLibrary();
	
    int entityCount = vsl->GetEntityCount();
    
    if (entityCount == 0)
    {
        printf("No scenes found in the document.  No models or skeletons will be exported.\n");
    }
    else if (entityCount > 1)
    {
        printf("More than one scene was found in the document.  Only the first one will be exported.\n");
    }
    
    FCDSceneNode* rootScene = vsl->GetEntity(0);
        
    fm::pvector<FCDEntityInstance> instances;
    
    ExtractInstances(rootScene, &instances);
    
    int numInstances = instances.size();
    
    for (int instanceIndex = 0; instanceIndex < numInstances; instanceIndex++)
    {
        FCDEntityInstance* curInstance = instances[instanceIndex];
        FCDEntity* curEntity = curInstance->GetEntity();
        
        if (curEntity == NULL)
        {
            printf("Warning, entity %x is NULL.\n", instanceIndex);
            continue;
        }
        
        int type = curEntity->GetType();
        
		const char* curEntityName = curEntity->GetName().c_str();
        NeonMessage("Processing %s\n", curEntityName);
		
		// Compare against restriction list.  If not on restriction list, then don't export this item.
		// If we have an empty restriction list, then don't worry about this.
		
		int restrictionListSize = 0;
		
		if (gExportRestrictionList != NULL)
		{
			restrictionListSize = CFArrayGetCount(gExportRestrictionList);
		}

		if (restrictionListSize != 0)
		{
			bool shouldExport = false;

			for (int i = 0; i < restrictionListSize; i++)
			{
				CFStringRef entityNameStringRef = (CFStringRef)CFArrayGetValueAtIndex(gExportRestrictionList, i);
				int stringLength = CFStringGetLength(entityNameStringRef);
				
				char* entityNameCStr = (char*)malloc(stringLength + 1);
				
				CFStringGetCString(entityNameStringRef, entityNameCStr, stringLength + 1, kCFStringEncodingASCII);
				
				int result = strcmp(entityNameCStr, curEntityName);
				
				free(entityNameCStr);
				
				if (result == 0)
				{
					shouldExport = TRUE;
					break;
				}
			}
			
			if (!shouldExport)
			{
				NeonMessage("%s is not on the export list.  Skipping...\n", curEntityName);
				continue;
			}
		}
        
        switch(type)
        {
            case FCDEntity::GEOMETRY:
            {
                Mesh* mesh = ReadGeometry((FCDGeometryInstance*)curInstance);
                
                if (mesh != NULL)
                {
                    CFArrayAppendValue(gMeshList, mesh);
                }
                
                break;
            }
            
            case FCDEntity::CONTROLLER:
            {
                FCDControllerInstance* controllerInstance = (FCDControllerInstance*)curInstance;
                
                Skeleton* skeleton = ReadController(controllerInstance);
                
                if (skeleton != NULL)
                {
                    if (CFArrayGetCount(gSkeletonList) == 1)
                    {
                        // When parsing the animation clips, we reference one skeleton and look for nodes in there
                        // that correspond to the animations.  We don't search through all skeletons.
                        printf("More than one skeleton was found in this file.  Animations won't work.");
                    }
                    
                    CFArrayAppendValue(gSkeletonList, skeleton);
                }
                
                break;
            }
            
            default:
            {
                printf("%s encountered.  Skipping.\n", ENTITY_NAMES[type]);
                break;
            }
        }
    }
    
    return retVal;
}

bool ReadAnimationClipLibrary()
{
    FCDAnimationClipLibrary* acl = gDocument->GetAnimationClipLibrary();

    int numEntities = acl->GetEntityCount();
    
    if (numEntities == 0)
    {
        NeonMessage("No animation clips found.  Skipping...\n");
    }
    else
    {
        gAnimationsFound = true;
        
        for (int curEntity = 0; curEntity < numEntities; curEntity++)
        {
            FCDAnimationClip* curClip = acl->GetEntity(curEntity);
            
            AnimationClip* animationClip = ReadAnimationClip(curClip, (Skeleton*)CFArrayGetValueAtIndex(gSkeletonList, 0));
            
            if (animationClip != NULL)
            {
                CFArrayAppendValue(gAnimationClipList, animationClip);
            }
        }
    }
    
    return true;
}

bool ReadAnimationLibrary()
{
    if (gAnimationsFound)
    {
        return true;
    }
    
    FCDAnimationLibrary* animationLibrary = gDocument->GetAnimationLibrary();
    
    int numEntities = animationLibrary->GetEntityCount();
    
    if (numEntities == 0)
    {
        printf("No animation clips found.  Skipping...\n");
    }
    else
    {
        // We don't support files that have both animation clips *and* discrete animations.
        assert(gAnimationsFound == false);
        
        gAnimationsFound = true;
        
        FCDAnimation** animationList = new FCDAnimation*[numEntities];
        
        for (int i = 0; i < numEntities; i++)
        {
            animationList[i] = animationLibrary->GetEntity(i);
        }
                    
        AnimationClip* animationClip = ReadAnimation(animationLibrary, animationList, numEntities, (Skeleton*)CFArrayGetValueAtIndex(gSkeletonList, 0));
        CFArrayAppendValue(gAnimationClipList, animationClip);
        
        free(animationList);
    }
    
    return true;
}

bool ReadScene()
{
    ReadSceneLibrary();
    ReadAnimationClipLibrary();
    ReadAnimationLibrary();
    
    return true;
}

void WriteScene()
{
    WriteMeshes(gMeshList);
    WriteSkeletons(gSkeletonList);
    WriteAnimationClips(gAnimationClipList);
}

void ExtractInstances(FCDSceneNode* inRoot, fm::pvector<FCDEntityInstance>* inInstances)
{
    int numChildren = inRoot->GetChildrenCount();
    int numInstances = inRoot->GetInstanceCount();
    
    for (int curInstance = 0; curInstance < numInstances; curInstance++)
    {
        inInstances->push_back(inRoot->GetInstance(curInstance));
    }
    
    for (int curChild = 0; curChild < numChildren; curChild++)
    {
        ExtractInstances(inRoot->GetChild(curChild), inInstances);
    }
}

void DisplayUsage()
{
    printf("Usage is Neon21ModelExporter <input filename> <output directory>\n");
	printf("Optional arguments are:\n");
	printf("-maxNumWeights <val>:\tThis will restrict the maximum number of vertex influences\n");
	printf("\t\t\t\t\t\t\t(and rescale the largest influences to add to 1).\n");
	printf("-restrictObjects \"<objects separated by spaces>\":\tThis will only export objects with the\n");
	printf("\t\t\t\t\t\t\t\t\t\tindicated names. Object names must be separated by spaces,\n");
	printf("\t\t\t\t\t\t\t\t\t\tand the entire list remust be enclosed in quotes.\n");
}

bool ParseArgs(int inArgc, char* inArgv[])
{
    gInputFile = inArgv[inArgc - 2];
    gOutputDirectory = inArgv[inArgc - 1];
    
    int numExtraArgs = inArgc - 3;
		    
    for (int i = 0; i < numExtraArgs; i += 2)
    {
        int argIndex = i + 1;
        
        if (strstr(inArgv[argIndex], "-maxNumWeights"))
        {
            sscanf(inArgv[argIndex + 1], "%d", &gMaxNumWeights);
        }
		else if (strstr(inArgv[argIndex], "-restrictObjects"))
		{
			int objectListStringLength = strlen(inArgv[argIndex + 1]);
			char* objectList = (char*)malloc(objectListStringLength + 1);
			
			gExportRestrictionList = CFArrayCreateMutable(NULL, 0, NULL);
			
			strcpy(objectList, inArgv[argIndex + 1]);

			char* curFile = strtok(objectList, " ");
			
			while(curFile != NULL)
			{
				CFStringRef newString = CFStringCreateWithCString(NULL, curFile, kCFStringEncodingASCII);
				CFArrayAppendValue(gExportRestrictionList, newString);
				
				curFile = strtok(NULL, " ");
			}
			
			free(objectList);
		}
        else if (strstr(inArgv[argIndex], "-verbose"))
        {
            gMinMessageLevel = 0;
        }
        else if (strstr(inArgv[argIndex], "-indexed"))
        {
            gExportIndexed = true;
        }
    }
    
    return true;
}

int main (int argc, char* argv[])
{
    bool status = ParseArgs(argc, argv);
    
    if (status)
    {
        // Initialize FCollada and have it parse the Collada file
        Init();
        
        // Verify the high level scene is valid
        ReadScene();
        
        // Write out the information in a more sane binary format
        WriteScene();
    }

    return 0;
}
