#ifndef SCREEN_QUEUE_H
#define SCREEN_QUEUE_H

#include "protocol.h"

typedef enum _screen_op_type {
  SCREEN_OP_DOT,
  SCREEN_OP_LINE,
  SCREEN_OP_ALPHA,
  SCREEN_OP_BLOCK_DRAW,
  SCREEN_OP_PAINT,
  SCREEN_OP_CLEAR,
} ScreenOpType;

typedef struct _screen_op {
  ScreenOpType type;
  padPt Coord1;
  padPt Coord2;
  padRGB foreground;
  padRGB background;
  char text[64];
  unsigned char count;
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
ScreenOpNode* _screen_queue_add(ScreenOpNode* node, ScreenOp op);
void screen_queue_add_at(ScreenOpNode* node, ScreenOp data);
void screen_queue_remove_node(ScreenOpNode* head);
ScreenOpNode* screen_queue_free_list(ScreenOpNode *head);

#endif
