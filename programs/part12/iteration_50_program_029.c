/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int_t;
typedef float my_float_t;
typedef double my_double_t;
typedef char my_char_t;
typedef long long my_longlong_t;
typedef unsigned int my_uint_t;
typedef _Bool my_bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* const const_string_ptr_t;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated as user struct) */
typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
    void* ptr;
};

union Variant {
    long long int_val;
    double float_val;
    struct Point point_val;
    union Data data_val;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void (*void_func_ptr)(void);
typedef const struct Rectangle* ConstRectPtr;

/* TYPE_ARRAY: Fixed-size arrays */
int global_int_array[100];
struct Point point_array[50];
union Data data_array[25];
char string_array[10][256];

/* Multi-dimensional arrays */
int matrix[10][10];
struct Point point_matrix[5][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef int (*binary_op_t)(int, int);
typedef char* (*string_transform_t)(const char*);

/* Complex callback with struct parameter */
typedef struct Point (*point_generator_t)(int, int);
typedef void (*point_processor_t)(struct Point*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
    char tag;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Vector types with GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} trans_union_t;

/* Enum types (should be TYPE_SCALAR) */
enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
};

typedef enum Color Color_t;

/* Bitfield struct */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

/* Anonymous struct/union */
struct Container {
    struct {
        int x;
        int y;
    } position;
    union {
        int int_data;
        float float_data;
    } value;
};

/* Forward declared struct that gets defined later */
struct ForwardDeclared;

/* Function declarations using our types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);

#endif /* TEST_TYPES_H */
