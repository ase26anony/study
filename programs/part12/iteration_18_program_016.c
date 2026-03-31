/* type_zoo.h - Header file with declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

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
    int *ptr;
    double arr[5];
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char name[20];
} Point;

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
    long l;
    double d;
    void *p;
} GenericUnion;

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int, void *);

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

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[3];

/* Pointer types */
extern int **global_ppint;
extern struct ComplexStruct *global_struct_ptr;
extern union DataUnion *global_union_ptr;

/* Function declarations using various types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_complex(int seed);
Point *create_point(int x, int y, const char *name);
void use_callback(Comparator cmp, void *data);
void handle_event(EventHandler handler, int event_id);

#endif /* TYPE_ZOO_H */
