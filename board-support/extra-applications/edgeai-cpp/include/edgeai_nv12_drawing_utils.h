/*
 *  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TI_POST_PROCESS_UTILS_
#define _TI_POST_PROCESS_UTILS_

/* Standard headers. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Module Headers. */
#include <edgeai_nv12_font_utils.h>

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define RGB2Y(R, G, B) CLIP(( ( 66*(R) + 129*(G) + 25*(B) + 128) >>8 ) + 16)
#define RGB2U(R, G, B) CLIP(( ( -38*(R) - 74*(G) + 112*(B) + 128) >>8) + 128)
#define RGB2V(R, G, B) CLIP((( 112*(R) - 94*(G) - 18*(B) + 128) >> 8) + 128)

#define SIGN(x) ((x<0) ? -1 : ((x>0) ? 1 : 0))
#define ABSOLUTE(x)  (x < 0 ? -x : x)

/**
 * \defgroup group_post_process_utils Utils for Post Process
 *
 * \brief Helper functions for drawing and text rendering
 *        in NV12
 *
 * \ingroup group_post_process
 */

/** Structure to contain info
 *  about NV12 image and its addr 
 */
typedef struct {
    uint8_t* yRowAddr;
    uint8_t* uvRowAddr;
    int32_t  width;
    int32_t  height;
} Image;

/** Structure for YUV Color. */
typedef struct {
    uint8_t Y;
    uint8_t U;
    uint8_t V;
} YUVColor;

/**
 * BarGraph Structure
 *
 */
typedef struct {
    Image*          m_img;
    int32_t         m_topX;
    int32_t         m_topY;
    int32_t         m_graphTopX;
    int32_t         m_graphTopY;
    FontProperty*   m_valueFontProp;
    YUVColor*       m_textColor;
    YUVColor*       m_fillColor;
    int32_t         m_width;
    int32_t         m_height;
    int32_t         m_maxValue;
    float           m_heightPerUnit;
    const char*     m_valueUnit;
} BarGraph;

/**
* Utility Function for getting YUV color from RGB counterpart
*
* @param color Empty YUV color structure
* @param R     Red value
* @param G     Green value
* @param B     Blue value
*/
void getColor(YUVColor* color, uint8_t R, uint8_t G, uint8_t B);

/**
* Function to draw a pixel at specific coordinate in NV12 image
*
* @param img    Image Structure
* @param drawX  X coordinate
* @param drawY  Y coordinate
* @param color  color
*/
void drawPixel(Image* img,int32_t drawX,int32_t drawY,YUVColor* color);

/**
* Function to fill region specified by top-left coordinate
*
* @param img     Image Structure
* @param startX  X coordinate
* @param startY  Y coordinate
* @param width   width
* @param height  height
* @param color   color
*/
void fillRegion(Image*      img,
                int32_t     startX,
                int32_t     startY,
                int32_t     width,
                int32_t     height,
                YUVColor*   color);

/**
* Function to draw a line using Breshnam's algorithm
*
* @param img        Image Structure
* @param drawX1     X coordinate
* @param drawY1     Y coordinate
* @param drawX2     X coordinate
* @param drawY2     Y coordinate
* @param color      color
* @param thickness  thickness of the line
*/
void drawLine(Image*    img,
                int32_t   drawX1,
                int32_t   drawY1,
                int32_t   drawX2,
                int32_t   drawY2,
                YUVColor* color,
                int32_t   thickness);

/**
* Function to draw a horizontal line
*
* @param img        Image Structure
* @param startX     X coordinate
* @param startY     Y coordinate
* @param width      width of the line
* @param color      color
* @param thickness  thickness of the line
*/
void drawHorizontalLine(Image*      img,
                        int32_t     startX,
                        int32_t     startY,
                        int32_t     width,
                        YUVColor*   color,
                        int32_t     thickness);

/**
* Function to draw a vertical line
*
* @param img        Image Structure
* @param startX     X coordinate
* @param startY     Y coordinate
* @param height     height of the line
* @param color      color
* @param thickness  thickness of the line
*/
void drawVerticalLine(Image*    img,
                        int32_t   startX,
                        int32_t   startY,
                        int32_t   height,
                        YUVColor* color,
                        int32_t   thickness);

/**
* Function to draw a rectangle
*
* @param img        Image Structure
* @param startX     X coordinate
* @param startY     Y coordinate
* @param width      width of the rect
* @param height     height of the rect
* @param color      color
* @param thickness  thickness of the rect
*/
void drawRect(Image*    img,
                int32_t   startX,
                int32_t   startY,
                int32_t   width,
                int32_t   height,
                YUVColor* color,
                int32_t   thickness);

/**
* Function to draw a rectangle
*
* @param img        Image Structure
* @param xc         Center X coordinate
* @param yc         Center Y coordinate
* @param radius     Radius of curcle
* @param color      color
* @param thickness  thickness of the circle
*/
void drawCircle(Image*      img,
                int32_t     xc,
                int32_t     yc,
                int32_t     radius,
                YUVColor*   color,
                int32_t     thickness);

/**
* Function to render text
*
* @param img        Image Structure
* @param text       Text to render
* @param topX       X coordinate
* @param topY       Y coordinate
* @param fontProp   Font Property
* @param color      Color
*/
void drawText(Image*        img,
              const char*   text,
              int32_t       topX,
              int32_t       topY,
              FontProperty* fontProp,
              YUVColor*     color);

/**
* Function to blend two images
*
* @param imgSrc     Image Structure
* @param imgDest    Image Structure
* @param alpha      alpha value
* @param beta       beta value
* @param gamma      gamma value
*/
void blendImage(Image     *imgSrc,
                Image     *imgDest,
                float     alpha,
                float     beta,
                float     gamma
                );

/**
* Function to initialize bar graph
*
* @param graph          BarGraph Structure
* @param img            Image Structure
* @param topX           topX coordinate of graph
* @param topY           topY coordinate of graph
* @param width          width of graph
* @param height         height of graph
* @param maxValue       maximum value of the graph
* @param title          title of the graph
* @param valueUnit      Unit to append to value while displaying
* @param titleFontProp  Font of the title
* @param valueFontProp  Font of the value
* @param textColor      Color of the text
* @param fillColor      Color to fill graph with
* @param bgColor        Background color of graph
*/

void initGraph(BarGraph*     graph,
               Image*        img,
               int32_t       topX,
               int32_t       topY,
               int32_t       width,
               int32_t       height,
               int32_t       maxValue,
               const char*   title,
               const char*   valueUnit,
               FontProperty* titleFontProp,
               FontProperty* valueFontProp,
               YUVColor*     textColor,
               YUVColor*     fillColor,
               YUVColor*     bgColor
               );

/**
* Function to update bar graph
*
* @param graph      BarGraph Structure
* @param value      Update Value
*/
void updateGraph(BarGraph* graph, int32_t value);

#endif // _TI_POST_PROCESS_UTILS_
