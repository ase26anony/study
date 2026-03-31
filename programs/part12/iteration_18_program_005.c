/* types.h - Header file to declare types */
#ifndef TYPES_H
#define TYPES_H

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
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int id;
    double values[5];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x;
    int y;
} Point2D;

typedef struct {
    Point2D position;
    double velocity;
    char name[32];
} Particle;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long timestamp;
    double measurement;
    void *ptr;
} TimestampUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern Point2D **double_ptr_ptr;
extern void (*func_ptr)(void);

/* Array types */
extern int int_array[10];
extern struct ComplexStruct struct_array[3];
extern char *string_array[5];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_type, void *data);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (another GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations */
void process_scalars(int seed);
void process_structs_and_unions(void);
void process_arrays_and_pointers(void);
void use_callbacks(void);

#endif /* TYPES_H */
