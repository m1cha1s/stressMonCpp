#include <cmath>

#include "plot.h"

Plot::Plot()
{
    for (int i = 0;i < BUFFER_SIZE;++i)
    {
        RingBuffer[i] = 0.0;
    }
    
    RingBufferHead = 0;
    RingBufferTail = BUFFER_SIZE-1;
}

void Plot::PushTail(double val)
{
    RingBufferHead++;
    if (RingBufferHead > BUFFER_SIZE-1)
    {
        RingBufferHead = 0;
    }
    RingBufferTail++;
    if (RingBufferTail > BUFFER_SIZE-1)
    {
        RingBufferTail = 0;
    }
    
    RingBuffer[RingBufferHead] = val;
}

void Plot::PushHead(double val)
{
    RingBufferHead--;
    if (RingBufferHead < 0)
    {
        RingBufferHead = BUFFER_SIZE-1;
    }
    RingBufferTail--;
    if (RingBufferTail < 0)
    {
        RingBufferTail = BUFFER_SIZE-1;
    }
    
    RingBuffer[RingBufferTail] = val;
}

void DrawLine(u32 *pixels, int width, int height, double X0, double Y0, double X1, double Y1, u32 color)
{
    double dx = X1-X0;
    double dy = Y1-Y0;
    
    int steps = abs(dx) > abs(dy) ? (int)abs(dx) : (int)abs(dy);
    
    double Xinc = dx / (double)steps;
    double Yinc = dy / (double)steps;
    
    double X = X0;
    double Y = Y0;
    
    for (int i = 0; i < steps; ++i)
    {
        if ((int)round(X)+width*(int)round(Y) < width*height)
        {
            pixels[(int)round(X)+width*(int)round(Y)] = color;
        }
        X += Xinc;
        Y += Yinc;
    }
}

void Plot::Draw(u32 *bitmap, int Width, int Height)
{
    // Draw background
    for (int X = PlotRegion.left; X < PlotRegion.right; X++)
    {
        for (int Y = PlotRegion.top; Y < PlotRegion.bottom; Y++)
        {
            if (X+Width*Y < Width*Height)
            {
                bitmap[X+Width*Y] = 0;
            }
        }
    }
    
    // Draw bounding box
    DrawLine(bitmap, Width, Height,
             PlotRegion.left,
             PlotRegion.top,
             PlotRegion.left,
             PlotRegion.bottom,
             0x00FFFFFF);
    
    DrawLine(bitmap, Width, Height,
             PlotRegion.right,
             PlotRegion.top,
             PlotRegion.right,
             PlotRegion.bottom,
             0x00FFFFFF);
    
    DrawLine(bitmap, Width, Height,
             PlotRegion.left,
             PlotRegion.top,
             PlotRegion.right,
             PlotRegion.top,
             0x00FFFFFF);
    
    DrawLine(bitmap, Width, Height,
             PlotRegion.left,
             PlotRegion.bottom,
             PlotRegion.right,
             PlotRegion.bottom,
             0x00FFFFFF);
    
    double MaxVal = 0;
    double MinVal = 0;
    
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        double V = RingBuffer[i];
        
        if (V > MaxVal) 
        {
            MaxVal = V;
        }
        if (V < MinVal)
        {
            MinVal =V; 
        }
    }
    
    if (MaxVal == 0 && MinVal == 0) 
    {
        MaxVal = 5;
        MinVal = -MaxVal;
    }
    
    double Delta = abs(MaxVal) > abs(MinVal) ? 2*MaxVal : -2*MinVal;
    
    double DX = (double)(PlotRegion.right-PlotRegion.left)/BUFFER_SIZE;
    double X = PlotRegion.left;
    
    for (int i = BUFFER_SIZE-1; i > 0; i--)
    {
        int idx = RingBufferHead + i;
        if (idx > BUFFER_SIZE)
        {
            idx-=BUFFER_SIZE;
        }
        double Y0 = (.5-RingBuffer[idx]/Delta)*(PlotRegion.bottom-PlotRegion.top)+PlotRegion.top;
        double Y1 = (.5-RingBuffer[idx+1]/Delta)*(PlotRegion.bottom-PlotRegion.top)+PlotRegion.top;
        DrawLine(bitmap, Width, Height,
                 X,
                 Y0,
                 X+DX,
                 Y1,
                 StrokeColor);
        X+=DX;
    }
}