/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long longlong_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;
typedef enum { RED, GREEN, BLUE } color_t;

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
    int id;
};

typedef struct {
    char name[50];
    int age;
    float salary;
} Employee;

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
    struct Point point_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void (*func_ptr_t)(void);
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char char_matrix[3][3];
typedef union Data data_array[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, const char*);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;
typedef Employee EmployeeType;

/* Complex nested types for thorough testing */
struct ComplexType {
    PointPtr points[10];
    comparator_t compare;
    union Data data;
    string_t name;
    int_array_10 counts;
    void (*operation)(struct ComplexType*);
};

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
void handle_callback(callback_t cb, int value, const char* msg);

#endif /* TEST_TYPES_H */
