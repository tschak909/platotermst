#include <stdlib.h>
#include <string.h>
#include "screen_queue.h"

void screen_queue_init(ScreenOpNode** head)
{
  *head = NULL;
}

ScreenOpNode* screen_queue_add(ScreenOpNode* node, ScreenOp op)
{
  ScreenOpNode *tmpPtr = node;
  while (tmpPtr != NULL)
    {
      if ((tmpPtr->op.type==op.type) &&
	  (tmpPtr->op.Coord1.x==op.Coord1.x) &&
	  (tmpPtr->op.Coord2.x==op.Coord2.x) &&
	  (tmpPtr->op.Coord1.y==op.Coord1.y) &&
	  (tmpPtr->op.Coord2.y==op.Coord2.y))
	{
	  // Replace record.
	  // not replacing type, because same.
	  // Not replacing coordinates, because same.
	  tmpPtr->op.foreground=op.foreground;
	  tmpPtr->op.background=op.background;
	  strcpy(tmpPtr->op.text,op.text);
	  tmpPtr->op.count=op.count;
	  tmpPtr->op.textMem=op.textMem;
	  tmpPtr->op.TTY=op.TTY;
	  tmpPtr->op.ModeBold=op.ModeBold;
	  tmpPtr->op.Rotate=op.Rotate;
	  tmpPtr->op.CurMode=op.CurMode;
	  return tmpPtr;
	}
      tmpPtr=tmpPtr->next;
    }
  return _screen_queue_add(node,op);
}

ScreenOpNode* _screen_queue_add(ScreenOpNode* node, ScreenOp op)
{
  ScreenOpNode* temp = (ScreenOpNode*) malloc(sizeof (ScreenOpNode));
  if (temp == NULL)
    {
      /* FIXME: do a purge and re-insert. */
      /* exit(0); // no memory available */
    }
  temp->op = op;
  temp->next = node;
  node = temp;
  return node;
}

void screen_queue_add_at(ScreenOpNode* node, ScreenOp op)
{
  ScreenOpNode* temp = (ScreenOpNode*) malloc(sizeof (ScreenOpNode));
  if (temp == NULL)
    {
      exit(EXIT_FAILURE); // no memory available
    }
  temp->op = op;
  temp->next = node->next;
  node->next = temp;
}

void screen_queue_remove_node(ScreenOpNode* head)
{
  ScreenOpNode* temp = (ScreenOpNode*) malloc(sizeof (ScreenOpNode));
  if (temp == NULL)
    {
      // FIXME: Do we simply notify?
    }
  temp = head->next;
  head->next = head->next->next;
  free(temp);
}

ScreenOpNode* screen_queue_free_list(ScreenOpNode *head)
{
  ScreenOpNode *tmpPtr = head;
  ScreenOpNode *followPtr;
  while (tmpPtr != NULL)
    {
      followPtr = tmpPtr;
      tmpPtr = tmpPtr->next;
      free(followPtr);
    }
  return NULL;
}
