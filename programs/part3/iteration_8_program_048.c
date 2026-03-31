if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))
  continue;

if (bitmap_equal_p (other->block_bitmap, loop->block_bitmap)) {
  // Handle identical loops (maybe skip or mark as same loop)
  continue;
}

if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap)) {
  // other is completely contained within loop
  loop->loops.safe_push (other);
} else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap)) {
  // loop is completely contained within other
  other->loops.safe_push (loop);
}
// If neither is true, the loops partially overlap but neither contains the other
