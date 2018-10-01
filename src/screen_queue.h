#ifndef SCREEN_QUEUE_H
#define SCREEN_QUEUE_H

#include "protocol.h"

typedef enum _screen_op_type {
  DOT,
  LINE,
  ALPHA,
  BLOCK_DRAW,
} ScreenOpType;

typedef struct _screen_op {
  ScreenOpType type;
  padPt Coord1;
  padPt Coord2;
  padRGB foreground;
  padRGB background;
  char text[64];
  CharMem textMem;
  padBool TTY;
  padBool ModeBold;
  padBool Rotate;
  padBool CurMode;
} ScreenOp;

typedef struct _screen_op_node {
  ScreenOp op;
  struct _screen_op_node* next;
} ScreenOpNode;

void screen_queue_init(ScreenOpNode** head);
ScreenOpNode* screen_queue_add(ScreenOpNode* node, ScreenOp data);
void screen_queue_add_at(ScreenOpNode* node, ScreenOp data);
void screen_queue_remove_node(ScreenOpNode* head);
ScreenOpNode* screen_queue_free_list(ScreenOpNode *head);

#endif
