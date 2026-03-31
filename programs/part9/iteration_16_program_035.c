/* types1.c - Contains scalar, string, struct, and user struct types */

#include "types_common.h"

/* TYPE_SCALAR examples */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
volatile int volatile_int = 100;
const float const_float = 1.618f;

/* TYPE_STRING examples */
const char* global_string = "Hello, World!";
char* mutable_string = "Mutable";

/* TYPE_STRUCT - Plain C struct */
struct Point {
    int x;
    int y;
    const char* name;  /* TYPE_STRING inside struct */
};

/* TYPE_USER_STRUCT - Typedef struct */
typedef struct {
    int id;
    char name[50];     /* TYPE_ARRAY inside struct */
    struct Point* location;  /* TYPE_POINTER inside struct */
} User;

/* Forward declaration for TYPE_UNDEFINED */
struct Opaque;

/* Complex nested type */
struct Container {
    int scalar_field;                     /* TYPE_SCALAR */
    char* string_field;                   /* TYPE_STRING */
    struct Point point_field;             /* TYPE_STRUCT */
    User user_field;                      /* TYPE_USER_STRUCT */
    struct Opaque* opaque_ptr;            /* TYPE_POINTER to TYPE_UNDEFINED */
    int (*compare)(int, int);             /* TYPE_CALLBACK */
    int numbers[10];                      /* TYPE_ARRAY */
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char flag;
    int value;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int tag;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* Function using various types */
void process_data(struct Container* cont) {
    /* Function body - not important for gengtype */
    (void)cont;
}

int main(void) {
    /* Minimal main to satisfy compiler */
    return 0;
}
