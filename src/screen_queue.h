/**
 * PLATOTerm ST - Drawing Queue
 */

#include "protocol.h"

#ifndef SCREEN_QUEUE_H
#define SCREEN_QUEUE_H

#define SCREEN_QUEUE_DOT         0
#define SCREEN_QUEUE_LINE        1
#define SCREEN_QUEUE_CHAR        2
#define SCREEN_QUEUE_BLOCK_ERASE 3
#define SCREEN_QUEUE_PAINT       4

// Singly linked list of drawing elements.
typedef struct draw_element {
  short mode;                // mode, 0=dot, 1=line, 2=char, 3=blockerase, 4=paint
  short x1;                  // Coordinate X1
  short y1;                  // Coordinate Y1
  short x2;                  // Coordinate X2
  short y2;                  // Coordinate Y2
  unsigned char* ch;         // Character pointer to buffer for mode 2
  unsigned char chlen;       // length of character buffer for mode 2
  DispMode CurMode;          // Current drawing mode
  CharMem CurMem;            // Current text memory.
  short background_color_index; // Background color index
  short foreground_color_index; // Foreground color index
  struct draw_element* next; // Next element.
} DrawElement;

DrawElement* screen_queue_create(short mode, short x1, short y1, short x2, short y2, unsigned char* ch, unsigned char chlen, short background_color_index, short foreground_color_index, DrawElement* next);
DrawElement* screen_queue_prepend(DrawElement* head, short mode, short x1, short y1, short x2, short y2, unsigned char* ch, unsigned char chlen, short background_color_index, short foreground_color_index);
DrawElement* screen_queue_append(DrawElement* head, short mode, short x1, short y1, short x2, short y2, unsigned char* ch, unsigned char chlen, short background_color_index, short foreground_color_index);
void screen_queue_dispose(DrawElement* head);
int screen_queue_count(DrawElement* head);

#endif /* SCREEN_QUEUE_H */
