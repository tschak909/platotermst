#include <stdlib.h>
#include "screen_queue.h"

void screen_queue_init(ScreenOpNode** head)
{
  *head = NULL;
}

ScreenOpNode* screen_queue_add(ScreenOpNode* node, ScreenOp op)
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
