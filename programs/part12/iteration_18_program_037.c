/* type_zoo.h - Header file with forward declarations */
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
};

struct ComplexStruct {
    struct SimpleStruct nested;
    char name[32];
    void *data;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
} Point2D;

typedef struct {
    Point2D start;
    Point2D end;
    double thickness;
} LineSegment;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    struct {
        unsigned int seconds;
        unsigned int microseconds;
    } parts;
} TimeStamp;

/* Pointer types */
extern int *global_int_ptr;
extern int **global_int_ptr_ptr;
extern struct ComplexStruct *global_struct_ptr;
extern void (*global_func_ptr)(void);

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct global_struct_array[5];
extern char *global_string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(1)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentPtr;

/* Function declarations */
void process_scalars(int seed);
void process_structs_and_unions(void);
void process_arrays_and_pointers(void);
void use_callbacks(void);
void process_lang_structs(void);

#endif /* TYPE_ZOO_H */
