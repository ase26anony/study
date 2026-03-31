/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

/* Scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_llong_t;
typedef _Bool scalar_bool_t;

/* String type */
typedef char* string_t;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int id;
    double values[4];
    char* name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x, y;
    char label[32];
} Point2D;

typedef struct {
    Point2D start;
    Point2D end;
    double thickness;
} LineSegment;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    char as_char[8];
};

typedef union {
    long long timestamp;
    struct {
        int year;
        int month;
        int day;
    } date;
} TimeUnion;

/* Pointer types */
typedef int* int_ptr_t;
typedef int** int_dbl_ptr_t;
typedef struct ComplexStruct* struct_ptr_t;
typedef union DataUnion* union_ptr_t;

/* Array types */
typedef int int_array_10_t[10];
typedef struct SimpleStruct struct_array_5_t[5];
typedef int* pointer_array_8_t[8];

/* Callback types */
typedef void (*simple_callback_t)(int);
typedef int (*complex_callback_t)(double, char*, void*);

/* Language-specific struct (using GCC extensions) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int* ptr;
    long value;
};
#endif

/* Function declarations */
void process_scalars(scalar_int_t i, scalar_float_t f, scalar_double_t d);
struct ComplexStruct* create_complex_struct(int id, const char* name);
void register_callback(complex_callback_t cb);
void manipulate_arrays(int_array_10_t arr, struct_array_5_t sarr);

#endif /* TYPES_H */
