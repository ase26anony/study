#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;

/* TYPE_STRING: String types */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct Point* point_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef char char_array[20];
typedef struct Point point_array[5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

typedef __attribute__((deprecated)) int deprecated_int;

/* Complex nested type */
struct ComplexType {
    union Value values[4];          /* Array of unions */
    struct Point* points;           /* Pointer to struct */
    callback_func handler;          /* Callback function */
    int (*operations[5])(int, int); /* Array of function pointers */
};

/* Extern declarations for multiple translation units */
extern struct Point global_point;
extern union Value global_value;
extern MyStruct global_struct;

#endif /* TYPES_H */
