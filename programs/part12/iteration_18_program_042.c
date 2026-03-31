/* test_types.h - Header file with type declarations */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

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
    int id;
    double values[4];
    char* name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x, y;
} Point2D;

typedef struct Node {
    int value;
    struct Node* next;
} ListNode;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

typedef union {
    long long timestamp;
    double precision;
} TimeUnion;

/* Pointer types */
extern int* int_ptr;
extern int** int_dbl_ptr;
extern struct SimpleStruct* struct_ptr;
extern union DataUnion* union_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char* string_array[3];

/* Callback types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Function pointer declarations */
extern void (*global_callback)(int);
extern Comparator global_comparator;

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

/* Function prototypes using various types */
void process_scalars(int i, float f, double d);
struct ComplexStruct* create_complex_struct(int id, const char* name);
int compare_points(const Point2D* a, const Point2D* b);
void handle_event(EventHandler handler, int event_id);
void manipulate_arrays(int* arr, size_t size);
union DataUnion create_data_union(int type);

#endif /* TEST_TYPES_H */
