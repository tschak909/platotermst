/**
 * PLATOTerm ST - Drawing Queue
 */

#include <stdlib.h>
#include <osbind.h>
#include "screen_queue.h"

DrawElement* screen_queue_create(short mode,
				 short x1,
				 short y1,
				 short x2,
				 short y2,
				 unsigned char* ch,
				 unsigned char chlen,
				 DrawElement* next)
{
  DrawElement* new_element = (DrawElement*)malloc(sizeof(DrawElement));
  if (new_element==NULL)
    {
      return NULL;
    }
  new_element->mode=mode;
  new_element->x1=x1;
  new_element->y1=y1;
  new_element->x2=x2;
  new_element->y2=y2;
  new_element->ch=ch;
  new_element->chlen=chlen;
  new_element->next=next;
  
  return new_element;
}

DrawElement* screen_queue_prepend(DrawElement* head,
				  short mode,
				  short x1,
				  short y1,
				  short x2,
				  short y2,
				  unsigned char* ch,
				  unsigned char chlen)
{
  DrawElement* new_element = screen_queue_create(mode,x1,y1,x2,y2,ch,chlen,head);
  head=new_element;
  return head;
}

DrawElement* screen_queue_append(DrawElement* head,
				 short mode,
				 short x1,
				 short y1,
				 short x2,
				 short y2,
				 unsigned char* ch,
				 unsigned char chlen)
{
  if (head==NULL)
    return screen_queue_prepend(head, mode, x1, y1, x2, y2, ch, chlen);

  // Scoot to end.
  DrawElement* cursor = head;
  while (cursor->next != NULL)
    cursor=cursor->next;

  DrawElement* new_element = screen_queue_create(mode, x1, y1, x2, y2, ch, chlen, NULL);
  cursor->next = new_element;

  return head;
}

void screen_queue_dispose(DrawElement* head)
{
  DrawElement *cursor, *tmp;
  if (head != NULL)
    {
      cursor=head->next;
      head->next = NULL;
      while (cursor != NULL)
	{
	  // Free any character buffers which are being used.
	  if (cursor->ch != NULL)
	    {
	      free(cursor->ch);
	    }
	  tmp=cursor->next;
	  free(cursor);
	  cursor=tmp;
	}
    }
}

int screen_queue_count(DrawElement* head)
{
  DrawElement* cursor=head;
  int count=0;

  while (cursor->next != NULL)
    {
      count++;
      cursor=cursor->next;
      Bconout(2,0x07);
    }

  return count;
}