__attribute__((noinline, used))
float compare_safe(int cond) {
    if (cond < 0 || cond > 31) { // SSE predicates are 5-bit values
        return NAN; // Or handle error appropriately
    }
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    __m128 c = _mm_cmp_ss(a, b, cond);
    return _mm_cvtss_f32(c);
}
