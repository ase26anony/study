/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

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
    int x;
    float y;
    char z;
};

struct ComplexStruct {
    int id;
    double values[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int counter;
    float data;
    char tag;
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

typedef union {
    long long timestamp;
    double measurement;
    char identifier[8];
} TypedefUnion;

/* Pointer types */
extern int *int_ptr;
extern int **int_dbl_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void (*func_ptr)(void);

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[3];
extern int *ptr_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnionPtr;

/* Function declarations using these types */
void process_scalars(int a, float b, double c, char d, long long e, _Bool f);
struct ComplexStruct create_complex_struct(int id, const char *name);
UserStruct *create_user_struct(int counter, float data, char tag);
void process_union(union DataUnion u, TypedefUnion tu);
int sum_array(const int *arr, size_t len);
void sort_with_callback(void *base, size_t nmemb, size_t size, Comparator comp);
void handle_event(EventHandler handler, int event_id, void *data);

#endif /* TYPES_H */
