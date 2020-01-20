#include <gtest/gtest.h>

#include "GrassyBitfield.hh"

TEST(BitfieldTests, SetSingleVal) {
  GrassyBitfield field(3, false);
  EXPECT_EQ(field.get_val(5,5), false);
  EXPECT_EQ(field.get_draw_fields().size(), 1U);

  field.set_val(5, 5, true);
  EXPECT_EQ(field.get_val(5,5), true);
  EXPECT_EQ(field.get_draw_fields().size(), 2U);

  field.set_val(5, 5, false);
  EXPECT_EQ(field.get_val(5,5), false);
  EXPECT_EQ(field.get_draw_fields().size(), 1U);
}

TEST(BitfieldTests, UnSetSingleVal) {
  GrassyBitfield field(3, true);
  EXPECT_EQ(field.get_val(5,5), true);
  EXPECT_EQ(field.get_draw_fields().size(), 1U);

  field.set_val(5, 5, false);
  EXPECT_EQ(field.get_val(5,5), false);
  EXPECT_EQ(field.get_draw_fields().size(), 2U);

  field.set_val(5, 5, true);
  EXPECT_EQ(field.get_val(5,5), true);
  EXPECT_EQ(field.get_draw_fields().size(), 1U);
}

TEST(BitfieldTests, CollapseLowestLevel) {
  GrassyBitfield field(3);

  for(int x = 8; x<16; x++) {
    for(int y = 8; y<16; y++) {
      field.set_val(x,y,true);
    }
  }
  EXPECT_EQ(field.get_val(10,10), true);

  for(auto draw_field : field.get_draw_fields()) {
    EXPECT_GT(draw_field.width, 8U);
    std::cout << draw_field << std::endl;
  }
}
