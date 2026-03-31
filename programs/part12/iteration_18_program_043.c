/* type_zoo.h */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

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
    struct SimpleStruct nested;
    double extra;
    char name[32];
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    float z;
} Point3D;

typedef struct Node {
    int value;
    struct Node* next;
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
} NumberUnion;

/* Pointer types */
extern int* int_ptr;
extern struct SimpleStruct* struct_ptr;
extern union DataUnion* union_ptr;
extern char** string_ptr_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern Point3D point_array[8];

/* Callback types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* data);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* ip;
    void* vp;
} TransparentUnionPtr;

/* Function declarations using various types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_complex(int seed);
void array_operations(int* arr, size_t len);
int compare_points(const void* a, const void* b);
void handle_event(int id, void* data);

#endif /* TYPE_ZOO_H */
