#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile const int volatile_const_int;

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
    char tag;
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
typedef char CharArray[20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type */
struct ComplexType {
    IntArray numbers;           /* TYPE_ARRAY */
    union Value values[5];      /* TYPE_UNION array */
    Comparator compare_func;    /* TYPE_CALLBACK */
    struct Point* points;       /* TYPE_POINTER to TYPE_STRUCT */
    const char* description;    /* TYPE_STRING */
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    char flag;
    int value;
};

/* Deprecated typedef */
typedef int OldIntType __attribute__((deprecated));

/* Function declarations */
extern void process_data(struct ComplexType* data);
extern union Value create_value(int type);

#endif /* TYPES_H */
