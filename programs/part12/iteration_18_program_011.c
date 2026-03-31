/* type_zoo.h - Header file with type declarations */
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

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long timestamp;
    double precision;
    void *ptr;
} TimestampUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern char **string_array_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern Point2D point_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

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

/* Function declarations using these types */
void process_scalars(int a, float b, double c, char d);
struct ComplexStruct create_complex(int id, const char *name);
int sum_array(const int *arr, size_t len);
void register_callback(EventHandler handler);
union DataUnion get_union_data(int type);

#endif /* TYPE_ZOO_H */
