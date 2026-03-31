/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
struct forward_declared;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long long_long_t;
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

typedef struct {
    char name[50];
    int id;
    float salary;
} Employee;

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated as user struct) */
typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long long int_val;
    double double_val;
    void* ptr_val;
    struct Point point_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void* generic_ptr_t;
typedef const struct Rectangle* ConstRectPtr;
typedef int (*func_ptr_t)(void);  /* Also a callback */

/* TYPE_ARRAY: Array types */
int global_array[100];
extern float extern_array[50];
static double static_array[25];
struct Point point_array[10];
union Data data_array[5];
int multi_dim[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, char*);
typedef struct Point* (*factory_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    int (*operation)(struct ComplexType*);
    void* context;
    char name[32];
    struct ComplexType* next;
} ComplexType_t;

/* Enumeration types (might be treated as scalar) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color_t;

/* Function prototypes that use all types */
void process_point(struct Point* p);
union Data create_data(int type);
int compare_points(const void* a, const void* b);
ComplexType_t* create_complex_type(const char* name);

#endif /* TEST_TYPES_H */
