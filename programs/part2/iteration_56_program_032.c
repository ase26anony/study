#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct IncompleteStruct;

/* TYPE_STRUCT: Plain C structure */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declaration */
union Data {
    int i;
    float f;
    double d;
    char *str;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* Opaque pointer type for cross-file testing */
typedef struct Opaque *OpaqueHandle;

/* Complex nested type for thorough testing */
typedef struct Node {
    void *data;
    struct Node *next;  /* Self-referential pointer */
    struct Node *prev;
} LinkedListNode;

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_mystruct;

#endif /* TEST_TYPES_H */
