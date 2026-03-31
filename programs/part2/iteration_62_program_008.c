(define_insn "avx512f_blendmv16si"
  [(set (match_operand:V16SI 0 "register_operand" "=v")
        (vec_merge:V16SI
          (match_operand:V16SI 1 "register_operand" "v")
          (match_operand:V16SI 2 "register_operand" "v")
          (match_operand:SI 3 "register_operand" "k")))]
  "TARGET_AVX512F"
  "vpblendmd\t{%3, %2, %1, %0|%0, %1, %2, %3}"
  [(set_attr "type" "sselog")
   (set_attr "prefix" "evex")
   (set_attr "mode" "XI")])
