/**
 * PLATOTerm ST - Palette Queue
 */

#include "protocol.h"

#ifndef PALETTE_QUEUE_H
#define PALETTE_QUEUE_H

// Singly linked list of paletteing elements.
typedef struct palette_element {
  unsigned short palette_index;          // The palette index #
  unsigned char red;                     // The Red value
  unsigned char green;                   // The Green value
  unsigned char blue;                    // The Blue value
  struct palette_element* next; // Next element.
} PaletteElement;

PaletteElement* palette_queue_create(unsigned short palette_index, unsigned char red, unsigned char green, unsigned char blue, PaletteElement* next);
PaletteElement* palette_queue_prepend(PaletteElement* head, unsigned short palette_index, unsigned char red, unsigned char green, unsigned char blue);
PaletteElement* palette_queue_append(PaletteElement* head, unsigned short palette_index, unsigned char red, unsigned char green, unsigned char blue);
void palette_queue_dispose(PaletteElement* head);
int palette_queue_count(PaletteElement* head);

#endif /* PALETTE_QUEUE_H */
