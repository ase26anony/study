#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern _Bool global_bool;
extern long global_long;
extern long long global_long_long;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char* mutable_string;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    void* p;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef char CharArray[256];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);

/* Compiler attributes */
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
} DeprecatedStruct_t;

/* Complex nested type for deep traversal */
struct ComplexType {
    union Value values[5];           /* Array of unions */
    struct Point* points;            /* Pointer to struct */
    Comparator compare;              /* Callback */
    void (*handlers[3])(int);        /* Array of function pointers */
    volatile const int* volatile* volatile_ptr_ptr; /* Complex pointer */
};

/* For TYPE_LANG_STRUCT - using visibility attribute */
struct __attribute__((visibility("hidden"))) HiddenStruct {
    int secret_data;
};

/* Pack pragma example */
#pragma pack(push, 1)
struct PackedWithPragma {
    char flag;
    int value;
    short count;
};
#pragma pack(pop)

#endif /* TYPES_H */
