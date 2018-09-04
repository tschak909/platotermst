/**
 * PLATOTerm ST - Paletteing Queue
 */

#include <stdlib.h>
#include "palette_queue.h"

PaletteElement* palette_queue_create(unsigned short palette_index,
				  unsigned char red,
				  unsigned char green,
				  unsigned char blue,
				 PaletteElement* next)
{
  PaletteElement* new_element = (PaletteElement*)malloc(sizeof(PaletteElement));
  if (new_element==NULL)
    {
      return NULL;
    }
  new_element->palette_index=palette_index;
  new_element->red=red;
  new_element->green=green;
  new_element->blue=blue;
  new_element->next=next;
  
  return new_element;
}

PaletteElement* palette_queue_prepend(PaletteElement* head,
				      unsigned short palette_index,
				      unsigned char red,
				      unsigned char green,
				      unsigned char blue
				      )
{
  PaletteElement* new_element = palette_queue_create(palette_index,red,green,blue,head);
  head=new_element;
  return head;
}

PaletteElement* palette_queue_append(PaletteElement* head,
				     unsigned short palette_index,
				     unsigned char red,
				     unsigned char green,
				     unsigned char blue)
{
  if (head==NULL)
    return palette_queue_prepend(head, palette_index,red,green,blue);

  // Scoot to end.
  PaletteElement* cursor = head;
  while (cursor->next != NULL)
    cursor=cursor->next;

  PaletteElement* new_element = palette_queue_create(palette_index,red,green,blue, NULL);
  cursor->next = new_element;

  return head;
}

void palette_queue_dispose(PaletteElement* head)
{
  PaletteElement *cursor, *tmp;
  if (head != NULL)
    {
      cursor=head->next;
      head->next = NULL;
      while (cursor != NULL)
	{
	  tmp=cursor->next;
	  free(cursor);
	  cursor=tmp;
	}
    }
}

/**
 * Find color index for existing color or return -1
 * if not currently available.
 */
short palette_queue_find_color_index(PaletteElement* head, padRGB* theColor)
{
  PaletteElement* cursor=head;
  while (cursor->next != NULL)
    {
      if (cursor->red==theColor->red &&
	  cursor->green==theColor->green &&
	  cursor->blue==theColor->blue)
	return cursor->palette_index;
      cursor=cursor->next;
    }
  return -1; // Not found.
}

int palette_queue_count(PaletteElement* head)
{
  PaletteElement* cursor=head;
  int count=0;

  while (cursor->next != NULL)
    {
      count++;
      cursor=cursor->next;
    }

  return count;
}
