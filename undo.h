#ifndef LED_UNDO
#define LED_UNDO

enum {
    UNDO_ACTION_DELETE_CHAR = 0,
    UNDO_ACTION_APPEND_CHAR,
};

typedef struct UndoAction {
    int type;
    int line;
    int cursor;
    char ch;
} UndoAction;

typedef struct _UndoBuffer {
    UndoAction action;
    struct _UndoBuffer *next;
} UndoBuffer;

struct LedState;

UndoBuffer *undo_init(void);
UndoBuffer *undo_append(UndoBuffer *head, UndoAction action);
UndoBuffer *undo_delete(UndoBuffer *head);
void undo_free(UndoBuffer *head);

void undo(struct LedState *state);

#endif // LED_UNDO
