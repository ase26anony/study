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
    double z;
};

struct Rectangle {              /* Another plain struct */
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

/* TYPE_USER_STRUCT: Typedef'd struct */
typedef struct Point Point_t;   /* User struct via typedef */

/* TYPE_UNION: Union types */
union Data {                    /* Plain union */
    int i;
    float f;
    char str[20];
    double d;
};

typedef union Data Data_t;      /* Typedef'd union */

/* TYPE_POINTER: Pointer types */
typedef struct Point* PointPtr;         /* Pointer to struct */
typedef union Data* DataPtr;            /* Pointer to union */
typedef int* IntPtr;                    /* Pointer to scalar */
typedef void* VoidPtr;                  /* Generic pointer */
typedef const struct Point* ConstPointPtr; /* Const pointer */

/* TYPE_ARRAY: Array types */
int global_array[10];                   /* Array of scalars */
struct Point point_array[5];            /* Array of structs */
typedef int matrix_t[3][3];             /* Multi-dimensional array */
extern char extern_array[];             /* Incomplete array type */

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);  /* Function pointer */
typedef void (*callback_t)(int, void*);                 /* Another function pointer */
typedef int (*math_op_t)(int, int);                     /* Simple function pointer */

/* TYPE_LANG_STRUCT: GCC-specific attributed struct */
struct __attribute__((aligned(16))) AlignedStruct {  /* May trigger lang_struct */
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {        /* Another attributed struct */
    char a;
    int b;
    short c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int tag;
    union {
        int int_val;
        double double_val;
        char* string_val;
        struct ComplexType* next;
    } data;
    void (*processor)(struct ComplexType*);
    int array[8];
} ComplexType_t;

/* Enumeration type (should be treated as scalar) */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color_t;

/* Function declarations using various types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
void register_callback(callback_t cb);
ComplexType_t* create_complex_type(void);

#endif /* TEST_TYPES_H */
