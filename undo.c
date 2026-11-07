#include "undo.h"

#include "line_ops.h"
#include "state.h"

#include <stdlib.h>

UndoBuffer *undo_init(void)
{
    return NULL;
}

UndoBuffer *undo_append(UndoBuffer *head, UndoAction action)
{
    UndoBuffer *node = malloc(sizeof(UndoBuffer));
    node->action = action;
    node->next = head;
    return node;
}

UndoBuffer *undo_delete(UndoBuffer *head)
{
    if (!head)
      return NULL;

    UndoBuffer *node = head;
    head = head->next;
    free(node);
    return head;
}

void undo_free(UndoBuffer *head)
{
    UndoBuffer *node = head;
    while (node) {
        UndoBuffer *tmp = node;
        node = node->next;
        free(tmp);
    }
}

void undo(LedState *state)
{
    if (!state->undo)
        return;

    UndoAction action = state->undo->action;
    state->line = action.line;
    state->cursor = action.cursor - 1;

    switch (action.type) {
        case UNDO_ACTION_DELETE_CHAR:
            append_char_cursor(state, action.ch, false);
            break;
        case UNDO_ACTION_APPEND_CHAR:
            delete_char_cursor(state, false);
            break;
    }

    state->undo = undo_delete(state->undo);
}
