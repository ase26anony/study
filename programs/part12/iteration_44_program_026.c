// Pseudo-code for what this might generate
__m512i result = _mm512_mask_blend_epi8(mask, a, b);  // For V64QImode
__m512d result = _mm512_mask_blend_pd(mask, a, b);    // For V8DFmode
