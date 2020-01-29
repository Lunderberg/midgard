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
  for(int x = 8; x<16; x++) {
    for(int y = 8; y<16; y++) {
      EXPECT_EQ(field.get_val(x,y), true);
    }
  }

  field.set_val(10,10,true);
  for(int x = 8; x<16; x++) {
    for(int y = 8; y<16; y++) {
      EXPECT_EQ(field.get_val(x,y), true);
    }
  }

  for(auto draw_field : field.get_draw_fields()) {
    EXPECT_GT(draw_field.width, 8U);
  }
}

TEST(BitfieldTests, GrassGrowth) {
  GrassyBitfield field(2);
  field.set_val(4,7,true);
  field.set_val(52,8,true);
  field.set_val(7, 36,true);
  field.set_val(48, 36, true);

  for(int x=24; x<32; x++) {
    for(int y=16; y<24; y++) {
      field.set_val(x,y,true);
    }
  }

  for(int i=0; i<10; i++) {
    std::cout << "Iteration: " << i << std::endl;
    std::cout << std::string(73, '-') << std::endl;
    for(int y=63; y>=0; y--) {

      for(int x=0; x<64; x++) {
        if(x%8 == 0) {
          std::cout << "|";
        }
        std::cout << (field.get_val(x,y) ? "#" : " ");
      }
      std::cout << "|\n";


      if(y%8 == 0) {
        std::cout << std::string(73, '-') << std::endl;
      }
    }
    field.growth_iteration();

    std::cin.get();
  }
}
