/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* String type */
extern char *global_string;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int id;
    double values[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x, y;
} Point2D;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long ll;
    double d;
} NumberUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void **void_double_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*SignalHandler)(int);

/* Language-specific struct (GCC extensions) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
};
#endif

/* Function declarations using these types */
void process_scalars(int i, float f, double d, char c, long long ll, _Bool b);
struct ComplexStruct create_complex_struct(int id, const char *name);
int sum_array(const int *arr, size_t len);
void register_callback(Comparator cmp);
void use_transparent_union(struct PackedStruct ps);

#endif /* TYPE_ZOO_H */
