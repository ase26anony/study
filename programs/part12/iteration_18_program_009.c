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
    double data[4];
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char name[32];
} Point;

typedef struct {
    Point start;
    Point end;
    double weight;
} LineSegment;

/* Union types */
union DataUnion {
    int i;
    float f;
    char *str;
};

typedef union {
    long long l;
    double d;
    void *p;
} BigUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern Point *point_ptr;
extern int **double_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern Point point_array[8];
extern int *pointer_array[6];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *, ...);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} TransparentUnion;

/* Function declarations */
void init_globals(void);
int process_structs(void);
int process_unions(void);
int process_arrays(void);
int process_pointers(void);
int process_callbacks(int (*cb)(int), void (*log)(const char*));

#endif /* TYPE_ZOO_H */
