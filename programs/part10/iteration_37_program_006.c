#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_prototyped - old K&R style prototype */
int old_style_function(); /* No parameter list - K&R style */

/* For DW_AT_picture_string - simulate COBOL-like type */
typedef struct {
    char data[100];
    int scale;
    int decimal;
} PICTURE_TYPE;

/* For DW_AT_string_length structures */
struct string_with_length {
    int length;
    char data[]; /* Flexible array member */
};

struct string_with_bit_length {
    unsigned int bit_length : 16;
    unsigned int byte_length : 16;
    char* data;
};

/* For DW_AT_ordering - enum with non-sequential values */
enum reordered_enum {
    FIRST = 10,
    SECOND = 5,
    THIRD = 20,
    FOURTH = 15
};

/* For DW_AT_segment */
#ifdef __GNUC__
#define SECTION_ATTR __attribute__((section(".mysection")))
#else
#define SECTION_ATTR
#endif

/* Thread-local for DW_AT_threads_scaled */
extern thread_local int thread_local_array[100];

#ifdef __cplusplus
} /* extern "C" */

/* For DW_AT_explicit */
class ExplicitConstructor {
private:
    int value;
public:
    explicit ExplicitConstructor(int v) : value(v) {}
    int get_value() const { return value; }
};

/* For DW_AT_mutable */
class ClassWithMutable {
private:
    int regular_member;
    mutable int mutable_member; /* This should trigger DW_AT_mutable */
public:
    ClassWithMutable() : regular_member(0), mutable_member(0) {}
    void modify_mutable() const { mutable_member++; } /* Can modify mutable in const method */
};

/* For DW_AT_small - packed struct with bitfields */
struct __attribute__((packed)) SmallPackedStruct {
    unsigned char tiny : 3;    /* Very small bitfield */
    signed char small : 5;     /* Small signed bitfield */
    unsigned short medium : 10;
    int large;
    
    /* Member function to ensure it's not optimized away */
    int sum() const { return tiny + small + medium + large; }
};

/* Template to force emission in multiple contexts */
template<typename T>
class Container {
private:
    T data[10];
    int size;
public:
    Container() : size(0) {}
    void add(const T& item) {
        if (size < 10) data[size++] = item;
    }
    T get(int index) const {
        return (index >= 0 && index < size) ? data[index] : T();
    }
};

/* Function using std::optional */
std::optional<int> process_optional(std::optional<int> input);

/* Complex variable that might need detailed location info */
register int complex_location_var asm("ebx"); /* Hint for register storage */

#endif /* __cplusplus */

#endif /* DWARF_ATTRIBUTES_H */
