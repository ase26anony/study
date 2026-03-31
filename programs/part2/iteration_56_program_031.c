#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_SCALAR: Basic scalar types */
typedef int MyInt;
typedef float MyFloat;
typedef double MyDouble;
typedef char MyChar;

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
    char *s;
};

/* TYPE_ARRAY: Array type in header */
extern int global_array[100];

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef void (*VoidFunc)(void);

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* Complex nested type for recursive analysis */
typedef struct Node {
    struct Node *next;          /* TYPE_POINTER */
    union Data value;           /* TYPE_UNION */
    int (*process)(struct Node*); /* TYPE_CALLBACK */
} Node;

/* External declarations for multi-file testing */
extern void process_types(void);
extern int type_checksum(void);

#endif /* TEST_TYPES_H */
