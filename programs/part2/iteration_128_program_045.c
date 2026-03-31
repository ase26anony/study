// Pseudo-code for what these generators might produce
__m512i result = _mm512_mask_blend_epi8(mask, src1, src2);  // For V64QImode
__m512d result = _mm512_mask_blend_pd(mask, src1, src2);    // For V8DFmode
