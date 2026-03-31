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
typedef enum { RED, GREEN, BLUE } color_t;  /* Enum is scalar */

/* TYPE_STRING: String types */
typedef char* string_t;         /* String pointer */
typedef const char* cstring_t;  /* Constant string pointer */

/* TYPE_STRUCT: Plain C structures */
struct Point {                  /* Basic struct */
    int x;
    int y;
    double z;
};

struct ComplexStruct {          /* More complex struct */
    struct Point position;
    color_t color;
    string_t name;
    int id;
};

/* TYPE_USER_STRUCT: Struct with typedef */
typedef struct {                /* Anonymous struct with typedef */
    int data;
    float value;
} user_struct_t;

/* TYPE_UNION: Union types */
union Data {                    /* Basic union */
    int i;
    float f;
    char str[20];
    double d;
};

typedef union {                 /* Typedef union */
    long long l;
    void* ptr;
    struct Point point;
} variant_t;

/* TYPE_POINTER: Pointer types */
typedef struct Point* PointPtr;          /* Pointer to struct */
typedef union Data* DataPtr;             /* Pointer to union */
typedef int* int_ptr_t;                  /* Pointer to scalar */
typedef void (*void_func_ptr)(void);     /* Pointer to function */
typedef const struct Point* ConstPointPtr; /* Const pointer */

/* TYPE_ARRAY: Array types */
int global_array[10];                     /* Global array of scalars */
struct Point point_array[5];              /* Array of structs */
typedef int matrix_t[3][3];               /* 2D array typedef */
extern char string_array[][50];           /* Array of arrays */

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);  /* Callback type */
typedef void (*callback_t)(int, void*);                 /* Another callback */
typedef char* (*string_func_t)(void);                   /* Function returning string */

/* TYPE_LANG_STRUCT: GCC-specific attributed struct */
struct __attribute__((aligned(16))) AlignedStruct {  /* May trigger lang_struct */
    int data;
    double values[4];
};

struct __attribute__((packed)) PackedStruct {        /* Packed struct */
    char a;
    int b;
    short c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct Node {
    int value;
    struct Node* next;           /* Self-referential pointer */
    void* data;
    comparator_t compare_func;   /* Function pointer member */
} Node;

/* Forward declaration that will be defined later */
struct LaterDefined;

/* Function prototypes that use all types */
void process_point(struct Point* p);
int compare_nodes(const void* a, const void* b);
variant_t create_variant(int type, void* value);
void use_all_types(void);

#endif /* TEST_TYPES_H */
