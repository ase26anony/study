#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declarations */
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

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Callback)(int, void *);

/* Complex nested type for cross-file testing */
typedef struct ComplexNode {
    void *data;
    struct ComplexNode *next;
    struct ComplexNode *prev;
} ComplexNode;

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_mystruct;

#endif /* TYPES_H */
