#pragma once

#include <bitset>
#include <cstdint>
#include <map>
#include <vector>


class GrassyBitfield {
  using Bitfield = std::bitset<64>;
public:

  struct DrawField {
    bool values[8][8];
    std::uint32_t x_min;
    std::uint32_t y_min;
    std::uint32_t width;
  };

  /// Construct a GrassyBitfield
  /*
    Each recursive bitfield is square, and is of size 8^num_layers.
    There must be at least 1 layer present.
   */
  GrassyBitfield(unsigned int num_layers, bool initial_value=false);

  bool get_val(std::uint32_t x, std::uint32_t y);
  void set_val(std::uint32_t x, std::uint32_t y, bool val);

  std::vector<DrawField> get_draw_fields() const;

private:
  std::uint64_t get_address(std::uint32_t x, std::uint32_t y);
  std::uint64_t get_bitfield_key(std::uint64_t address, unsigned int layer);
  unsigned int get_bitfield_loc(std::uint64_t address, unsigned int layer);

  unsigned int num_layers;
  std::map<std::uint64_t, Bitfield> bitfields;
};

std::ostream& operator<<(std::ostream& out, const GrassyBitfield::DrawField& f);
