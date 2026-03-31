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
typedef char CharMatrix[5][10];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type combinations */
struct ComplexType {
    /* Array of pointers to unions */
    union Value* value_ptrs[5];
    
    /* Callback member */
    EventHandler handler;
    
    /* Pointer to array of structs */
    struct Point (*point_array_ptr)[10];
    
    /* Nested struct with bitfields */
    struct {
        unsigned int flags : 8;
        unsigned int mode : 4;
    } status;
    
    /* Flexible array member */
    int dynamic_data[];
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedData {
    char type;
    int value;
    short count;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated("Use int32_t instead")));

/* Function declarations */
extern void process_data(struct ComplexType* data);
extern union Value create_value(int type, void* data);

#endif /* TYPES_H */
