#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    float value;
} MyStruct;

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator)(const void*, const void*);

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef char CharArray[20];

/* GCC attributes for edge cases */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct;

/* Complex nested type */
struct ComplexType {
    union Value values[5];           /* Array of unions */
    struct Point* points;            /* Pointer */
    comparator cmp_func;             /* Callback */
    char* description;               /* String */
    int (*operation)(int, float);    /* Another callback */
};

/* For TYPE_LANG_STRUCT - C++ specific will be in separate file */
#ifdef __cplusplus
extern "C" {
#endif

extern struct Opaque* get_opaque(void);
extern void process_complex(struct ComplexType* ct);

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
