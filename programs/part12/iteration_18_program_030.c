/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

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
    char c;
};

struct ComplexStruct {
    int id;
    double values[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    float z;
} Point3D;

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
    long long l;
    double d;
    void *ptr;
} BigUnion;

/* Pointer types */
extern int *int_ptr;
extern int **int_dbl_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Language-specific struct (GCC extensions) */
struct PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((packed));

struct TransparentUnion {
    int i;
    float f;
} __attribute__((transparent_union));

/* Function declarations using these types */
void process_scalars(int a, float b, double c, char d, long long e, _Bool f);
struct ComplexStruct create_complex_struct(int id, const char *name);
Point3D *create_points(size_t count);
void sort_array(int *arr, size_t n, Comparator cmp);
void register_callback(EventHandler handler);

#endif /* TYPES_H */
