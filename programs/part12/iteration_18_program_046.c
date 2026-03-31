/* type_zoo.h */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdint.h>

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_llong;
extern _Bool global_bool;

/* String type */
extern char *global_string;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    char tag;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
} Point;

typedef struct {
    Point start;
    Point end;
    double thickness;
} Line;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long l;
    double d;
} NumberUnion;

/* Pointer types */
extern int *global_int_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern void **global_void_dbl_ptr;

/* Array types */
extern int global_int_array[10];
extern struct ComplexStruct global_struct_array[5];
extern Point *global_ptr_array[8];

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct ComplexStruct*, Point);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations */
void use_scalars(void);
void use_strings(void);
void use_structs(void);
void use_unions(void);
void use_pointers(void);
void use_arrays(void);
void use_callbacks(void);
void use_lang_structs(void);

#endif /* TYPE_ZOO_H */
