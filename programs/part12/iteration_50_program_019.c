/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

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
    int id;
};

/* Nested structure */
struct Container {
    struct Point point;
    int count;
    char name[32];
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
    void* ptr;
};

union Variant {
    long long int_val;
    double float_val;
    struct Point point_val;
    string_t string_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void* GenericPtr;
typedef struct Rectangle* RectPtr;
typedef const struct Point* ConstPointPtr;

/* TYPE_ARRAY: Fixed-size arrays */
int global_array[100];
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
typedef struct Point* (*point_creator_t)(int, int, double);
typedef void (*error_handler_t)(const char*, int);

/* Complex callback with structure parameter */
typedef int (*struct_comparator_t)(const struct Point*, const struct Point*);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
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

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Complete the incomplete type definition */
struct incomplete {
    int data;
    struct Point* next;
};

/* Additional complex types for thorough testing */
struct ComplexType {
    /* Mix of all type categories in one struct */
    int scalar_field;              /* TYPE_SCALAR */
    char* string_field;            /* TYPE_STRING */
    struct Point nested_struct;    /* TYPE_STRUCT */
    union Data variant;            /* TYPE_UNION */
    int* pointer_field;            /* TYPE_POINTER */
    int array_field[10];           /* TYPE_ARRAY */
    comparator_t callback_field;   /* TYPE_CALLBACK */
    struct AlignedStruct aligned;  /* TYPE_LANG_STRUCT */
};

/* Function prototypes using all types */
void process_point(struct Point* p);
int compare_points(const struct Point* a, const struct Point* b);
union Data create_data(int type, void* value);
void sort_array(void* base, size_t nmemb, size_t size, comparator_t comp);
void handle_with_callback(int event, callback_t handler, void* data);

#endif /* TEST_TYPES_H */
