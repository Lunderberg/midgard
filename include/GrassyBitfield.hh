#pragma once

#include <bitset>
#include <cstdint>
#include <map>
#include <vector>


class GrassyBitfield {
public:
  using Bitfield = std::bitset<64>;

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
  GrassyBitfield(unsigned int num_layers, bool initial_value=false,
                 bool edge_wrap=true);

  bool get_val(std::uint32_t x, std::uint32_t y) const;
  void set_val(std::uint32_t x, std::uint32_t y, bool val);

  void growth_iteration();

  std::vector<DrawField> get_draw_fields() const;

  unsigned int get_size() const;
  unsigned int get_num_layers() const { return num_layers; }

private:
  void determine_new_growth(
    std::uint64_t key,
    bool parent_value,
    std::map<std::uint64_t, Bitfield>& output
  ) const;
  bool exists_or_has_subfields(std::uint64_t key) const;
  void set_val(std::uint64_t address, bool val);

  unsigned int num_layers;
  std::map<std::uint64_t, Bitfield> bitfields;
  bool edge_wrap;
};

std::ostream& operator<<(std::ostream& out, const GrassyBitfield::DrawField& f);
std::ostream& operator<<(std::ostream& out, const GrassyBitfield::Bitfield& f);
