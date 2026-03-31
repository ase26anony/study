// Pseudo-code for what these functions generate
__m512i result = _mm512_mask_blend_epi8(mask, a, b);
// Where mask is a 64-bit mask (k-register), and a, b are 512-bit vectors
