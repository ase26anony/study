// Example of invalid address of bitfield
struct S {
    unsigned int bf : 4;  // bitfield
};

void test_invalid_address_of_bitfield() {
    struct S s;
    unsigned int *p = &s.bf;  // ERROR: Cannot take address of bitfield
}

// Example of incomplete sizeof
void test_incomplete_sizeof() {
    struct Incomplete;  // Forward declaration, incomplete type
    size_t s = sizeof(struct Incomplete);  // ERROR: sizeof incomplete type
}
