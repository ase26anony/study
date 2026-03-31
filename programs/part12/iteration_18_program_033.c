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
    int id;
    double values[4];
    struct SimpleStruct* nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char name[32];
} Point2D;

typedef struct Node {
    int data;
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
    void* ptr;
} BigUnion;

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct ComplexStruct*, Point2D*);

/* Language-specific struct (GCC extension) */
struct TransparentUnion {
    int a;
    double b;
} __attribute__((transparent_union));

/* Packed struct (another GCC extension) */
struct PackedData {
    char type;
    int value;
} __attribute__((packed));

/* Function declarations using various types */
void process_scalars(int a, float b, double c, char d, long long e, _Bool f);
struct ComplexStruct* create_complex_struct(int id);
Point2D transform_point(Point2D p, double scale);
void register_callback(SimpleCallback cb);
int process_array(int arr[10], double matrix[4][4]);

#endif /* TYPE_ZOO_H */
