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

/* TYPE_USER_STRUCT: Typedef for struct */
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
struct ComplexType {
    union Value values[5];           /* Array of unions */
    struct Point* points;            /* Pointer to struct */
    Comparator compare_func;         /* Callback */
    void (*operations[3])(void);     /* Array of function pointers */
    volatile const int* volatile* complex_ptr; /* Complex pointer */
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char type;
    int value;
    double amount;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int flags;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated("Use int32_t instead")));

/* Forward declarations for cross-file references */
extern struct CrossFileStruct* get_cross_struct(void);
extern void process_union_array(union Value arr[], int size);

#endif /* TYPES_H */
