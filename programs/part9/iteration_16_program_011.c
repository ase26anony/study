#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;

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

/* Complex nested type combinations */
struct ComplexContainer {
    /* Struct containing array of pointers to unions */
    union Value* value_array[8];
    
    /* Struct with callback member */
    EventHandler handler;
    
    /* Nested struct */
    struct {
        int depth;
        char label[16];
    } inner;
    
    /* Pointer to array of structs */
    struct Point* point_grid[4][4];
};

/* GCC attributes for edge cases */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((deprecated));

union __attribute__((packed)) PackedUnion {
    int i;
    char bytes[4];
};

/* Volatile and const qualifiers */
extern volatile const int* volatile* complex_pointer;

#endif /* TYPES_H */
