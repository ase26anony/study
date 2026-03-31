#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing macros */
#ifdef __cplusplus
extern "C" {
#endif

/* GCC GC attribute to mark types for garbage collector */
#ifndef GTY
#define GTY(x) __attribute__((user("GC"))) x
#endif

/* Mark types as used to prevent elimination */
#define GTY_USED __attribute__((used, retain))

/* Basic scalar types with GC tracking */
typedef GTY(()) int gty_int;
typedef GTY(()) float gty_float;
typedef GTY(()) double gty_double;
typedef GTY(()) enum { RED, GREEN, BLUE } gty_enum;

/* String type */
typedef GTY(()) char* gty_string;

/* Callback type (function pointer) */
typedef GTY(()) void (*gty_callback)(int, const char*);

/* Forward declarations for complex types */
struct gty_complex_struct;
union gty_complex_union;

/* Language-specific structure simulation */
typedef GTY(()) struct gty_lang_struct {
    int lang_id;
    void* lang_data;
} gty_lang_struct_t;

#ifdef __cplusplus
}
#endif

#endif /* TEST_GENGYPE_H */
