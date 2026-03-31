uint64_t add_with_carry(uint64_t a, uint64_t b, uint64_t* carry) {
    uint64_t result;
    asm volatile ("add %[a], %[b]\n\t"
                  "adc $0, %[carry]"
                  : [result] "=r"(result), [carry] "+r"(*carry)
                  : [a] "r"(a), [b] "r"(b));
    return result;
}
