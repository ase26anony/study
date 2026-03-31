/* type_zoo.h */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdint.h>

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_llong;
extern _Bool global_bool;

/* String type */
extern char* global_string;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int* ptr;
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
    double length;
} LineSegment;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    struct {
        int date;
        int time;
    } parts;
} TimeUnion;

/* Pointer types */
extern int* global_int_ptr;
extern int** global_int_ptr_ptr;
extern struct SimpleStruct* global_struct_ptr;
extern union DataUnion* global_union_ptr;

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct global_struct_array[5];
extern char* global_string_array[8];

/* Callback types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* data);

/* Function pointer declarations */
extern void (*global_callback)(int);
extern Comparator global_comparator;

/* Language-specific struct (GCC extensions) */
#ifdef __GNUC__
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int* ptr;
    void* vptr;
};
#endif

/* Opaque declarations to force type inclusion */
void use_all_types(void);
int compute_checksum(void);

#endif /* TYPE_ZOO_H */
