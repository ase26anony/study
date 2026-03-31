// test_dwarf_attributes.h
#ifndef TEST_DWARF_ATTRIBUTES_H
#define TEST_DWARF_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

// For DW_AT_prototyped
void prototyped_function(void);
int variadic_function(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void noreturn_function(void) __attribute__((noreturn));

// For DW_AT_segment
#ifdef __GNUC__
#define SEGMENT_ATTR __attribute__((section(".custom_segment")))
#else
#define SEGMENT_ATTR
#endif

// For thread-local storage (DW_AT_threads_scaled)
extern _Thread_local int thread_local_var;

#ifdef __cplusplus
}
#endif

#endif // TEST_DWARF_ATTRIBUTES_H
