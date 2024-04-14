#pragma once

#include "framework.h"

#define BUFFER_SIZE 256

struct Plot
{
    double RingBuffer[BUFFER_SIZE];
    int RingBufferHead;
    int RingBufferTail;
    RECT PlotRegion;
    u32 StrokeColor;
    
    Plot();
    
    void PushHead(double);
    void PushTail(double);
    
    void Draw(u32 *bitmap, int Width, int Height);
};