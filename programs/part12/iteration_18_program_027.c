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
    int id;
    double values[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char label[32];
} Point2D;

typedef struct {
    Point2D start;
    Point2D end;
    double thickness;
} LineSegment;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_str[16];
    void *as_ptr;
};

typedef union {
    long long timestamp;
    double precision_time;
    struct {
        unsigned int seconds;
        unsigned int microseconds;
    } split;
} TimeUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern char **string_ptr_ptr;

/* Array types */
extern int int_array[20];
extern struct ComplexStruct struct_array[5];
extern float *pointer_array[10];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

/* Function pointer declarations */
extern void (*global_callback)(int);
extern Comparator global_comparator;

/* Language-specific struct (GCC extensions) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
};

/* Opaque forward declarations for cross-file types */
struct OpaqueStruct;
typedef struct OpaqueStruct OpaqueHandle;

#endif /* TYPE_ZOO_H */
