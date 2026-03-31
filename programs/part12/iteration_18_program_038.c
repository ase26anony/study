/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types (TYPE_SCALAR) */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_long_long;
extern _Bool global_bool;

/* String type (TYPE_STRING) */
extern char *global_string;

/* Struct types (TYPE_STRUCT) */
struct SimpleStruct {
    int x;
    float y;
};

struct ComplexStruct {
    int id;
    char name[32];
    struct SimpleStruct nested;
    void *data;
};

/* User struct types via typedef (TYPE_USER_STRUCT) */
typedef struct {
    double real;
    double imag;
} ComplexNumber;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types (TYPE_UNION) */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long l;
    double d;
} NumberUnion;

/* Pointer types (TYPE_POINTER) */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void **void_double_ptr;

/* Array types (TYPE_ARRAY) */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(const char *, void *);

/* Language-specific struct (TYPE_LANG_STRUCT) */
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

/* Undefined/forward declared type (TYPE_UNDEFINED) */
struct ForwardDeclared;

/* Function declarations using various types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_complex_struct(int id, const char *name);
ComplexNumber add_complex(ComplexNumber a, ComplexNumber b);
void use_callback(SimpleCallback cb, int value);
void process_array(int arr[], size_t size);

#endif /* TYPE_ZOO_H */
