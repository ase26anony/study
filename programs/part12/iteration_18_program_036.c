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
    double data[10];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int counter;
    float value;
    char label[32];
} UserStruct;

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    double measurement;
    void *context;
} UserUnion;

/* Pointer types */
extern int *global_int_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern void **global_void_double_ptr;

/* Array types */
extern int global_int_array[100];
extern struct SimpleStruct global_struct_array[20];
extern char *global_pointer_array[50];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Function pointer declarations */
extern void (*global_callback)(int, char *);
extern Comparator global_comparator;

/* Language-specific struct (GCC extensions) */
#ifdef __GNUC__
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
};
#endif

/* Function prototypes using various types */
struct SimpleStruct process_struct(struct ComplexStruct cs);
UserStruct *create_user_struct(int id, float value);
void register_callback(EventHandler handler);
int compare_values(const void *a, const void *b);

#endif /* TYPES_H */
