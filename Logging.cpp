//
//  Logging.cpp
//  Neon21ModelExporter
//
//  Copyright (c) 2012 Neon Games LLC. All rights reserved.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

#include "Logging.h"
#include "main.h"

void NeonLog(int inLevel, const char* inFormat, ...)
{
    va_list argList;
    
    va_start(argList, inFormat);
    NeonLog(inLevel, inFormat, argList);
    va_end(argList);
}

void NeonLog(int inLevel, const char* inFormat, va_list inArgList)
{
    if (inLevel >= gMinMessageLevel)
    {
        vprintf(inFormat, inArgList);
    }
}

void NeonMessage(const char* inFormat, ...)
{
    va_list argList;
    
    va_start(argList, inFormat);
    NeonLog(0, inFormat, argList);
    va_end(argList);
}

void NeonWarning(const char* inFormat, ...)
{
    va_list argList;
    
    va_start(argList, inFormat);
    NeonLog(1, inFormat, argList);
    va_end(argList);
 
}

void NeonError(const char* inFormat, ...)
{
    va_list argList;
    
    va_start(argList, inFormat);
    NeonLog(2, inFormat, argList);
    va_end(argList);

}