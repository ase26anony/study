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

/* Nested structure */
struct Employee {
    char name[50];
    int id;
    struct Point location;
    double salary;
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

/* Pointer to pointer */
typedef PointPtr* PointPtrPtr;

/* TYPE_ARRAY: Array types */
int int_array[10];
float float_array[5][3];
struct Point point_array[8];
char string_array[4][50];
int multi_dim_array[2][3][4];

/* Fixed-size array typedef */
typedef int fixed_array_t[100];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, char*);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* Complex callback with struct parameter */
typedef struct Point (*point_generator_t)(int);
typedef void (*point_processor_t)(struct Point*, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    union {
        int i;
        float f;
    } u;
};

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* TYPE_USER_STRUCT: May be triggered by typedef struct */
typedef struct Point PointType;
typedef struct Employee EmployeeType;

/* Additional complex types for thorough testing */
struct ComplexType {
    /* Mix of all type categories */
    int scalar_field;              /* TYPE_SCALAR */
    char* string_field;            /* TYPE_STRING */
    struct Point struct_field;     /* TYPE_STRUCT */
    union Data union_field;        /* TYPE_UNION */
    int* pointer_field;            /* TYPE_POINTER */
    int array_field[5];            /* TYPE_ARRAY */
    comparator_t callback_field;   /* TYPE_CALLBACK */
    struct AlignedStruct lang_struct_field; /* TYPE_LANG_STRUCT */
};

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
void handle_callback(callback_t cb, int value, char* message);

#endif /* TEST_TYPES_H */
