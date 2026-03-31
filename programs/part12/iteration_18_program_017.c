/* type_zoo.h - Header file for type declarations */
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
    float z;
} Point3D;

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    double measurement;
    void *ptr;
} DataUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union SimpleUnion *union_ptr;
extern char **string_ptr_ptr;

/* Array types */
extern int int_array[10];
extern struct ComplexStruct struct_array[5];
extern Point3D point_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

/* Language-specific struct (using GCC extensions) */
struct PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((packed));

struct TransparentUnion {
    int a;
    float b;
} __attribute__((transparent_union));

/* Function prototypes */
void init_types(void);
int process_scalars(int seed);
float process_structs(struct ComplexStruct *cs);
void process_pointers(int **ptr_array, size_t count);
DataUnion process_unions(union SimpleUnion u);
void use_callback(Comparator cmp, EventHandler handler);

#endif /* TYPE_ZOO_H */
