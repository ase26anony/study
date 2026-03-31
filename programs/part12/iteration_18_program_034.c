/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

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

/* User struct types via typedef (TYPE_USER_STRUCT) */
typedef struct {
    int width;
    int height;
    double aspect;
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
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void **void_double_ptr;

/* Array types (TYPE_ARRAY) */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

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

/* Function pointer declarations */
extern void (*global_callback)(int);
extern Comparator global_comparator;

/* Undefined type forward declaration (TYPE_UNDEFINED) */
struct UndefinedStruct;  /* Forward declaration */

#endif /* TYPES_H */
