/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int_t;           /* Scalar type */
typedef float my_float_t;       /* Scalar type */
typedef double my_double_t;     /* Scalar type */
typedef char my_char_t;         /* Scalar type */
typedef long long my_llong_t;   /* Scalar type */
typedef _Bool my_bool_t;        /* Scalar type */

/* TYPE_STRING: String types */
typedef char* string_t;         /* String type */
typedef const char* cstring_t;  /* Const string type */

/* TYPE_STRUCT: Plain C structures */
struct Point {                  /* Plain struct */
    int x;
    int y;
};

struct Rectangle {              /* Struct with nested struct */
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

/* TYPE_USER_STRUCT: Typedef'd struct */
typedef struct {
    int id;
    char name[50];
    float salary;
} Employee;                     /* User-defined struct type */

/* TYPE_UNION: Union types */
union Data {                    /* Plain union */
    int i;
    float f;
    char str[20];
};

typedef union {                 /* Typedef'd union */
    long long timestamp;
    double value;
    void* ptr;
} Variant;

/* TYPE_POINTER: Pointer types */
typedef struct Point* PointPtr;         /* Pointer to struct */
typedef union Data* DataPtr;            /* Pointer to union */
typedef int* IntPtr;                    /* Pointer to scalar */
typedef void (*VoidFuncPtr)(void);      /* Pointer to function */

/* TYPE_ARRAY: Array types */
int global_array[10];                   /* Array of scalars */
struct Point point_array[5];            /* Array of structs */
typedef int Matrix[3][3];               /* Multi-dimensional array */

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);  /* Callback type */
typedef void (*EventHandler)(int event_id, void* data); /* Another callback */

/* TYPE_LANG_STRUCT: GCC-specific attributed struct */
struct __attribute__((aligned(16))) AlignedStruct {  /* May trigger lang_struct */
    int data;
    char padding[12];
};

/* Another GCC-specific extension */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
        struct ComplexType* next;
    } value;
    Comparator compare_func;  /* Callback field */
    struct ComplexType* children[5];  /* Array of pointers */
} ComplexType;

/* Function prototypes that use our types */
void process_point(struct Point* p);
int compare_employees(const void* a, const void* b);
ComplexType* create_complex_type(int tag);

#endif /* TEST_TYPES_H */
