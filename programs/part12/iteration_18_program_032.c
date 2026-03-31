/* type_zoo.h - Header file for type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types (TYPE_SCALAR) */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_llong;
extern _Bool global_bool;

/* String type (TYPE_STRING) */
extern char *global_string;
extern const char *global_const_string;

/* Struct types (TYPE_STRUCT) */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

struct ComplexStruct {
    int id;
    double data[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct {
    int width;
    int height;
    float aspect;
} Rectangle;

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
    long long timestamp;
    double precision_time;
} TimeUnion;

/* Pointer types (TYPE_POINTER) */
extern int *global_int_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern union DataUnion *global_union_ptr;
extern void *global_void_ptr;
extern int **global_int_dbl_ptr;

/* Array types (TYPE_ARRAY) */
extern int global_int_array[10];
extern struct SimpleStruct global_struct_array[5];
extern char *global_ptr_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Function pointer declarations */
extern void (*global_callback)(int);
extern Comparator global_comparator;

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long *lptr;
};

/* Undefined/forward declarations (TYPE_UNDEFINED during processing) */
struct ForwardDeclared;
typedef struct ForwardDeclared ForwardType;

/* Function prototypes using various types */
void process_scalars(int a, float b, double c, char d);
struct ComplexStruct create_complex(int id, const char *name);
int compare_values(const void *a, const void *b);
void handle_event(int event, void *data);

#endif /* TYPE_ZOO_H */
