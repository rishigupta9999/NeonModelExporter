#include <stdlib.h>

class Mesh;
class FCDGeometryInstance;
class FCDGeometry;

void ExtractMaterial(FCDGeometryInstance* inInstance, Mesh* inMesh);
Mesh* ReadGeometry(FCDGeometryInstance* inInstance, FCDGeometry* inGeometry = NULL);