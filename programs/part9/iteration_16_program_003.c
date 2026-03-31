#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile int volatile_int;
extern const double const_double;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char mutable_string[50];

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
    char* s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef char CharMatrix[5][20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type for deep traversal */
struct ComplexContainer {
    union Value values[5];           /* Array of unions */
    struct Point* points[10];        /* Array of pointers to structs */
    Comparator sort_func;            /* Callback function pointer */
    volatile const int* volatile* volatile_ptr_ptr; /* Complex pointer */
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    char flag;
    int value;
    short count;
};

/* Deprecated typedef */
typedef struct Point OldPoint __attribute__((deprecated));

#endif /* TYPES_H */
