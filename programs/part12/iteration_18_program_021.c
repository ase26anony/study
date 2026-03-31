/* types.h - Header file to declare types */
#ifndef TYPES_H
#define TYPES_H

/* Scalar types (TYPE_SCALAR) */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
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

/* User struct types (TYPE_USER_STRUCT) */
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
    long long ll;
    double d;
} NumericUnion;

/* Pointer types (TYPE_POINTER) */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern int **double_ptr;

/* Array types (TYPE_ARRAY) */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event, void *data);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations using these types */
void process_scalars(int a, float b, double c, char d);
struct ComplexStruct create_complex_struct(int id, const char *name);
ComplexNumber add_complex(ComplexNumber a, ComplexNumber b);
void sort_array(int *arr, int size, Comparator cmp);
void handle_event(int event, EventHandler handler);

#endif /* TYPES_H */
