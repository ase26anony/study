#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;

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
    char c;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct Point* point_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef char char_array[50];

/* TYPE_STRING: String types (char* in structured context) */
typedef const char* const_string;
typedef char* mutable_string;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int tag;
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct_t;

/* Complex nested type */
struct ComplexType {
    union Value values[5];           /* Array of unions */
    struct Point* points;            /* Pointer to struct */
    comparator cmp_func;             /* Callback */
    const_string name;               /* String */
    int (*operation)(int, int);      /* Another callback */
    volatile const int* volatile* complex_ptr; /* Complex pointer */
};

/* Forward declarations for cross-file references */
extern struct CrossFileStruct* get_cross_struct(void);
extern void register_callback(callback_func func);

#endif /* TYPES_H */
