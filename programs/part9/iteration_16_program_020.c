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
    char name[32];
} MyStruct;

/* TYPE_UNION: Union definition */
union Value {
    int i;
    float f;
    double d;
    void* ptr;
};

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct Point* PointPtr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array typedefs */
typedef int int_array[10];
typedef char string_array[20][50];

/* GCC attributes for edge cases */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[4];
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct_t;

/* Complex nested type */
struct ComplexType {
    union Value values[5];           /* Array of unions */
    struct Point* points;            /* Pointer to struct */
    callback_func handlers[3];       /* Array of callbacks */
    int (*matrix)[10][10];           /* Pointer to 2D array */
    volatile const int* volatile* pp; /* Complex pointer */
};

/* External declarations for multiple translation units */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Value global_union;

#endif /* TYPES_H */
