//
//  Logging.h
//  Neon21ModelExporter
//
//  Copyright (c) 2012 Neon Games LLC. All rights reserved.
//

#ifndef Neon21ModelExporter_Logging_h
#define Neon21ModelExporter_Logging_h

#include <stdarg.h>

void NeonLog(int inLevel, const char* inFormat, ...);
void NeonLog(int inLevel, const char* inFormat, va_list inArgList);

void NeonMessage(const char* inFormat, ...);
void NeonWarning(const char* inFormat, ...);
void NeonError(const char* inFormat, ...);

#endif