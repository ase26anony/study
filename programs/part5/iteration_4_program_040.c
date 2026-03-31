struct S {
    unsigned int bf : 4;  // bitfield
};

void test_invalid_address_of_bitfield() {
    struct S s;
    // Invalid: cannot take address of bitfield member
    // unsigned int* p = &s.bf;  // Compilation error
}

void test_incomplete_sizeof() {
    struct incomplete;  // Forward declaration, incomplete type
    // Invalid: sizeof requires complete type
    // size_t size = sizeof(struct incomplete);  // Compilation error
}
