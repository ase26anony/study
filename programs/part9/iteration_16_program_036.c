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

/* TYPE_UNION: Union type */
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

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);

/* TYPE_STRING: String types */
typedef const char* const_string;
typedef char* mutable_string;

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
    union Value values[5];
    struct Point* points;
    int (*processor)(struct ComplexType*);
    char name[32];
};

/* External declarations */
extern struct Opaque* global_opaque;
extern MyStruct global_struct;

#endif /* TYPES_H */
