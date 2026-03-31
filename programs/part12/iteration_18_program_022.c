/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Scalar types - TYPE_SCALAR */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_long_long;
extern _Bool global_bool;

/* String type - TYPE_STRING */
extern char *global_string;

/* Struct types - TYPE_STRUCT */
struct SimpleStruct {
    int x;
    float y;
};

struct ComplexStruct {
    int id;
    double values[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types - TYPE_USER_STRUCT */
typedef struct {
    int width;
    int height;
    float aspect;
} Rectangle;

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* Union types - TYPE_UNION */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long timestamp;
    double precision_time;
} TimeUnion;

/* Pointer types - TYPE_POINTER */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern void **void_double_ptr;

/* Array types - TYPE_ARRAY */
extern int int_array[10];
extern struct ComplexStruct struct_array[5];
extern char *string_array[8];

/* Callback types - TYPE_CALLBACK */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Function pointer declarations */
extern void (*global_callback)(int);
extern int (*math_operation)(int, int);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long *lptr;
};

/* Undefined/forward declarations - TYPE_UNDEFINED */
struct ForwardDeclared;  /* Will be defined later */

/* Function prototypes that use various types */
void process_scalars(int a, float b, double c, char d);
struct ComplexStruct create_complex_struct(int id, const char *name);
Rectangle *create_rectangle(int w, int h);
void use_union(union DataUnion *u);
int array_operations(int arr[], size_t len);
void register_callback(EventHandler handler);
void use_lang_structs(void);

#endif /* TYPE_ZOO_H */
