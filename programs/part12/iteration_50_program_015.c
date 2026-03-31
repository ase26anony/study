/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long longlong_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string_t;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

/* Nested structure for more complexity */
struct Employee {
    char name[50];
    int id;
    struct Point location;
    float salary;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    char char_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef char** StringArrayPtr;
typedef void* GenericPtr;

/* Pointer to pointer for depth */
typedef PointPtr* PointPtrPtr;

/* TYPE_ARRAY: Array types */
int global_array[100];
static float static_array[50];
extern double extern_array[25];

struct Point point_array[10];
union Data data_array[5];

/* Multi-dimensional arrays */
int matrix[3][3];
char string_table[5][100];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* Complex callback with structure parameter */
typedef struct Point (*point_generator_t)(int, int);
typedef void (*point_processor_t)(struct Point*);

/* TYPE_LANG_STRUCT: Structures with GCC attributes that might be special */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    union {
        int i;
        float f;
    } u;
};

/* Structure with vector attribute (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
struct VectorStruct {
    v4si vectors[2];
    int scalar;
};

/* Opaque structure pointer for TYPE_UNDEFINED testing */
struct opaque;
typedef struct opaque* OpaquePtr;

/* Enumeration type (should be TYPE_SCALAR) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA
} Color_t;

/* Bitfield structure */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Anonymous union within struct */
struct Container {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
};

/* Function type (not pointer) */
typedef int func_type(int, int);

/* Const qualified types */
typedef const int const_int_t;
typedef const struct Point const_point_t;
typedef const int (*const_int_comparator)(const int*, const int*);

/* Volatile qualified types */
typedef volatile int volatile_int_t;

/* Restrict qualified pointer */
typedef int* restrict IntRestrictPtr;

#endif /* TEST_TYPES_H */
