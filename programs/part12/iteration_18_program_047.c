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
    int x;
    float y;
};

struct ComplexStruct {
    int id;
    char name[32];
    struct SimpleStruct nested;
    void *data;
};

/* User struct types (typedef'd) */
typedef struct {
    double real;
    double imag;
} ComplexNumber;

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
} NumberUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern ComplexNumber **double_ptr_ptr;

/* Array types */
extern int int_array[10];
extern struct ComplexStruct struct_array[5];
extern char *string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int, void *);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (another GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    long *long_ptr;
} TransparentUnion;

/* Function declarations using these types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_struct(int id, const char *name);
ComplexNumber add_complex(ComplexNumber a, ComplexNumber b);
void sort_with_callback(void *base, size_t nmemb, Comparator cmp);
void register_handler(EventHandler handler);

#endif /* TYPE_ZOO_H */
