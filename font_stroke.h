#ifndef FONT_STROKE_H
#define FONT_STROKE_H

#include <GL/gl.h>

/* The stroke font structures */

typedef struct {
    GLfloat         X, Y;
} SFG_StrokeVertex;

typedef struct {
    int             Number;
    const SFG_StrokeVertex* Vertices;
} SFG_StrokeStrip;

typedef struct {
    GLfloat         Right;
    int             Number;
    const SFG_StrokeStrip* Strips;
} SFG_StrokeChar;

typedef struct {
    char*           Name;              /* The source font name      */
    int             Quantity;          /* Number of chars in font   */
    GLfloat         Height;            /* Height of the characters  */
    const SFG_StrokeChar** Characters; /* The characters mapping    */
} SFG_StrokeFont;

extern const SFG_StrokeFont fgStrokeMonoRoman;

#endif
