/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;

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
    float z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    double area;
};

/* Nested structure */
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
    double d;
    char str[20];
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
typedef int_t* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*VoidFuncPtr)(void);

/* Pointer to incomplete type */
struct incomplete* incomplete_ptr;

/* TYPE_ARRAY: Array types */
int int_array[10];
float float_array[5][3];
struct Point point_array[8];
char string_array[4][50];
int multi_dim_array[2][3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef struct Point* (*FactoryFunc)(int, int);
typedef int (*BinaryOp)(int, int);

/* Complex callback with struct parameter */
typedef void (*StructHandler)(struct Point*, union Data*);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int i;
    long l;
} TransparentUnion_t;

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Additional complex type combinations */
struct ComplexType {
    PointPtr points[10];
    Comparator compare;
    union Data data;
    int (*operations[5])(int, int);
};

/* Function declarations using these types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
void array_operations(int arr[][3], int rows);

#endif /* TEST_TYPES_H */
