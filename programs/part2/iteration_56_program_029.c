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
    char c;
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
typedef void (*CallbackFunc)(int, void *);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Data global_union;

#endif /* TEST_TYPES_H */
