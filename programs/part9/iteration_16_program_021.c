#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;  /* TYPE_STRING: char* in struct context */
} MyStruct;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
    volatile const int* volatile* complex_ptr;  /* Complex pointer with qualifiers */
};

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
    char* str;  /* Another TYPE_STRING */
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef char string_array[20][50];

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct Point* point_ptr;
typedef void (*func_ptr)(void);

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[4];
};

union __attribute__((packed)) PackedUnion {
    int i;
    char bytes[4];
};

/* Deprecated typedef */
typedef int old_int_type __attribute__((deprecated));

/* External declarations for multiple translation units */
extern struct Point global_point;
extern union Value global_value;
extern MyStruct global_mystruct;

#endif /* TYPES_H */
