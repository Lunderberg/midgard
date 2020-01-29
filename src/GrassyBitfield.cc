#include "GrassyBitfield.hh"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace {
std::uint64_t get_address(std::uint32_t x, std::uint32_t y) {
  // Interleave every 3 bits of x and y.  Every 3 bits gives the
  // coordinate in a given layer.  This every 6 bit chunk as a
  // location in a particular field.

  std::uint64_t output = 0;
  for(unsigned int i=0; i<10; i++) {
    unsigned int coordinate_bits = 3*i;
    unsigned int x_bits = 6*i;
    unsigned int y_bits = 6*i + 3;
    output |= ((x >> coordinate_bits) & 7) << x_bits;
    output |= ((y >> coordinate_bits) & 7) << y_bits;
  }
  return output;
}


std::uint64_t get_bitfield_key(std::uint64_t address, unsigned int layer) {
  // The bitfields are stored according to a key, derivable from the
  // address of the tile being indexed.  The key is the address of the
  // bottom-left tile in the bitfield, plus (15 - layer).  By
  // generating it this way, looping over all fields by increasing
  // order of key does a pre-order depth-first traversal of all
  // bitfields.

  // Determine the bottom-left tile in the bitfield by zeroing out the
  // smallest 6*(layer+1) bits.  This will always zero out at least 6
  // bits, so those bits can be used to store the layer number.
  unsigned int bits_to_zero = 6*(layer+1);
  address = (address >> bits_to_zero) << bits_to_zero;

  // Layer number is in range [0, num_layers), where num_layers is at
  // most 10.  This can be packed into the lowest 4 bits.  The layer
  // number is stored as (15 - layer), so that larger layers (higher
  // layer num) will have lower values.
  return address + (15 - layer);
}

std::uint64_t get_bitfield_key(std::uint32_t x, std::uint32_t y, unsigned int layer) {
  return get_bitfield_key(get_address(x,y), layer);
}


unsigned int get_bitfield_loc(std::uint64_t address, unsigned int layer) {
  // Location in layer 0 is in lowest 6 bits, and each successive
  // layer is stacked above it.
  unsigned int starting_pos = 6*layer;
  return (address >> starting_pos) & 0x3f;
}

struct BitfieldKeyInfo{
  std::uint64_t bottom_left_address;
  std::uint32_t x_min;
  std::uint32_t y_min;
  unsigned int layer;
  std::uint32_t field_width;
  std::uint32_t tile_width;
};
BitfieldKeyInfo unpack_bitfield_key(std::uint64_t key) {
  BitfieldKeyInfo output;

  output.layer = 15 - (key & 15);
  output.tile_width = 1 << (3*output.layer);
  output.field_width = 8*output.tile_width;

  output.bottom_left_address = key & 0xffffffffffffffc0;

  output.x_min = 0;
  output.y_min = 0;
  for(unsigned int i=1; i<10; i++) {
    unsigned int coordinate_bits = 3*i;
    unsigned int x_bits = 6*i;
    unsigned int y_bits = 6*i + 3;
    output.x_min |= ((key >> x_bits) & 7) << coordinate_bits;
    output.y_min |= ((key >> y_bits) & 7) << coordinate_bits;
  }

  return output;
}

std::uint64_t get_subfield_key(std::uint64_t key, unsigned int loc) {
  // Returns the subfield of a field at a given location.
  // Assumes that the key given is not at layer 0.
  // Assumes that loc is in range [0,64).
  auto layer = unpack_bitfield_key(key).layer;
  // Add the segment of the address
  key |= std::uint64_t(loc) << (6*layer);
  // Increment the layer field, which decreases the layer by one,
  // since they are stored in reverse order.
  key += 1;
  return key;
}
}


GrassyBitfield::GrassyBitfield(unsigned int num_layers, bool initial_value,
                               bool edge_wrap)
  : num_layers(num_layers), edge_wrap(edge_wrap) {

  if(num_layers < 1) {
    throw std::invalid_argument("num_layers must be at least 1");
  }

  // Limitation based on 32-bit integer coordinates, and 64-bit
  // integer address.  3 bits per coordinate per layer gives a
  // maximum number of 10 layers.
  if(num_layers > 10) {
    throw std::invalid_argument("num_layers can be at most 10");
  }

  auto top_addr = get_bitfield_key(get_address(0,0), num_layers-1);
  bitfields[top_addr] = initial_value ? -1L : 0;
}


bool GrassyBitfield::get_val(std::uint32_t x, std::uint32_t y) {
  auto address = get_address(x,y);

  // Start at the lowest possible layer, then walk up to the topmost layer.
  for(unsigned int layer=0; layer<num_layers; layer++) {
    auto key = get_bitfield_key(address, layer);
    if(bitfields.count(key)) {
      auto loc = get_bitfield_loc(address, layer);
      return bitfields[key].test(loc);
    }
  }

  // If nothing else, we should reach the topmost bitfield, and should
  // never reach here.
  assert(false);
}

void GrassyBitfield::set_val(std::uint32_t x, std::uint32_t y, bool val) {
  auto address = get_address(x,y);
  set_val(address, val);
}

void GrassyBitfield::set_val(std::uint64_t address, bool val) {
  // Walk up the layers, starting at the lowest level.  Loop concludes
  // if (a) a bitfield doesn't exists or (b) a bitfield exists and
  // cannot be collapsed.

  // This function is used both to set an arbitrary location, or to check
  for(unsigned int layer=0; layer<num_layers; layer++) {
    auto key = get_bitfield_key(address, layer);
    if(bitfields.count(key)) {
      // Set the bit that has been passed up from the previous level
      auto& bitfield = bitfields[key];
      bitfield.set(get_bitfield_loc(address, layer), val);

      // If all the values are the same, pass the value to be set into
      // the next iteration of the loop.
      if(bitfield.all()) {
        val = true;
      } else if(bitfield.none()) {
        val = false;
      } else {
        break;
      }

      // Top-most layer is allowed to be uniform, so that every
      // location always exists within some bitfield.
      if(layer != num_layers-1) {
        bitfields.erase(key);
      }
    } else {
      // The bitfield doesn't already exist.  If whatever indirect
      // parent is above this layer already has the current value,
      // then we don't need to do anything.  Otherwise, we need to
      // make a new bitfield at the current layer.  The new bitfield
      // has the parent value for everything but the current loc.
      bool current_val = false;
      for(unsigned parent_layer = layer+1; parent_layer<num_layers; parent_layer++) {
        auto parent_key = get_bitfield_key(address, parent_layer);
        if(bitfields.count(parent_key)) {
          auto parent_loc = get_bitfield_loc(address, parent_layer);
          current_val = bitfields[parent_key].test(parent_loc);
          break;
        }
      }

      if(val != current_val) {
        auto loc = get_bitfield_loc(address, layer);
        Bitfield new_field = current_val ? -1L : 0;
        new_field.flip(loc);
        bitfields[key] = new_field;
      }
      break;
    }
  }
}


void GrassyBitfield::growth_iteration() {

  std::map<std::uint64_t, Bitfield> new_growth;
  determine_new_growth(bitfields.begin()->first, false, new_growth);

  for(const auto& pair : new_growth) {
    auto info = unpack_bitfield_key(pair.first);
    if(pair.second.any()) {
      auto& bitfield = bitfields[pair.first];
      bitfield |= pair.second;
      set_val(info.bottom_left_address, bitfield.test(0));
    }
  }
}

void GrassyBitfield::determine_new_growth(
  std::uint64_t key,
  bool parent_value,
  std::map<std::uint64_t, Bitfield>& output
) const {
  // Determine map
  Bitfield bitfield;
  if(bitfields.count(key)) {
    bitfield = bitfields.at(key);
  } else {
    bitfield = parent_value ? -1L : 0;
  }

  auto info = unpack_bitfield_key(key);
  std::uint32_t full_grid_size = 1UL << (3*num_layers);

  // Find the grid on the same level or higher that contains the
  // specified point.
  auto find_field = [&](std::uint32_t x, std::uint32_t y) {
    // If it is off the edge and edge wrapping is disabled, treat as
    // empty.
    if((x >= full_grid_size || y >= full_grid_size) && !this->edge_wrap) {
      return Bitfield(0);
    }
    // Adjust coordinate in case it is off the edge.
    x = (x + full_grid_size) % full_grid_size;
    y = (y + full_grid_size) % full_grid_size;

    auto address = get_address(x,y);
    for(unsigned int layer=info.layer; layer<this->num_layers; layer++) {
      auto key = get_bitfield_key(address, layer);
      if(bitfields.count(key)) {
        if(layer == info.layer) {
          // Adjacent field is on same layer, return bitfield as is.
          return bitfields.at(key);
        } else {
          // Adjacent field is on higher layer, return full/empty
          // bitfield.
          auto loc = get_bitfield_loc(address, layer);
          bool value = bitfields.at(key).test(loc);
          return Bitfield(value ? -1L : 0);
        }
      }
    }

    assert(false);
  };

  auto needs_recursion = [&](std::uint64_t key) {
    // Recursion is needed if the bitfield exists, or if any of its
    // subfields exists, or if any of its neighbors' subfields exist.
    auto info = unpack_bitfield_key(key);
    auto left_field = get_bitfield_key(get_address(info.x_min-1,
                                                   info.y_min),
                                       info.layer);
    auto right_field = get_bitfield_key(get_address(info.x_min+info.field_width,
                                                    info.y_min),
                                        info.layer);
    auto down_field = get_bitfield_key(get_address(info.x_min,
                                                   info.y_min+info.field_width),
                                       info.layer);
    auto up_field = get_bitfield_key(get_address(info.x_min,info.y_min-1),
                                     info.layer);
    return (exists_or_has_subfields(key) ||
            exists_or_has_subfields(left_field) ||
            exists_or_has_subfields(right_field) ||
            exists_or_has_subfields(up_field) ||
            exists_or_has_subfields(down_field));
  };


  // Find each adjacent field
  auto left_field = find_field(info.x_min-1, info.y_min);
  auto right_field = find_field(info.x_min+info.field_width, info.y_min);
  auto up_field = find_field(info.x_min, info.y_min+info.field_width);
  auto down_field = find_field(info.x_min, info.y_min-1);

  // Determine which values are being spread onto the current layer
  auto from_right = (left_field & Bitfield(0x8080808080808080)) >> 7;
  auto from_left = (right_field & Bitfield(0x0101010101010101)) << 7;
  auto from_down = (down_field & Bitfield(0xff00000000000000)) >> 56;
  auto from_up = (up_field & Bitfield(0x00000000000000ff)) << 56;

  // Generate a bitfield of the new spread that occurs.
  Bitfield no_left_side = 0x7f7f7f7f7f7f7f7f;
  Bitfield no_right_side = 0xfefefefefefefefe;
  auto new_spread_left = (((bitfield & no_left_side) << 1) | from_right) & ~bitfield;
  auto new_spread_right = (((bitfield & no_right_side) >> 1) | from_left) & ~bitfield;
  auto new_spread_down = ((bitfield >> 8) | from_up) & ~bitfield;
  auto new_spread_up = ((bitfield << 8) | from_down) & ~bitfield;

  if(info.layer == 0) {
    // If we are in the lowest layer, output gets updated with
    // from_left | from_right | from_up | from_down.

    output[key] =
      new_spread_left | new_spread_right | new_spread_down | new_spread_up;
  } else {
    // If we are not in the lowest layer, for each tile, if it has a
    // subgrid, recursively call to that subgrid.  If not, then check if
    // any of the adjacent fields cause lowest-level fields to need to
    // be added to the adjacency.
    for(unsigned int loc=0; loc<64; loc++) {
      auto subfield_key = get_subfield_key(key, loc);
      if(needs_recursion(subfield_key)) {
        // Further refinement exists, so let that level handle it.
        determine_new_growth(subfield_key, bitfield.test(loc), output);
      } else {
        // No further refinement exists.  Therefore, any new spread
        // must now be generated as new fields.
        auto subinfo = unpack_bitfield_key(subfield_key);

        // Line the left edge with left-edge filled
        if(new_spread_left.test(loc)) {
          auto x = subinfo.x_min;
          for(std::uint32_t y = subinfo.y_min; y<subinfo.y_min+subinfo.field_width; y += 8) {
            auto new_field = get_bitfield_key(x, y, subinfo.layer);
            output[new_field] |= Bitfield(0x0101010101010101);
          }
        }

        // Line the right edge with right-edge filled
        if(new_spread_right.test(loc)) {
          auto x = subinfo.x_min + subinfo.field_width - 1;
          for(std::uint32_t y = subinfo.y_min; y<subinfo.y_min+subinfo.field_width; y += 8) {
            auto new_field = get_bitfield_key(x, y, subinfo.layer);
            output[new_field] |= Bitfield(0x8080808080808080);
          }
        }

        // Line the bottom edge with bottom-edge filled
        if(new_spread_down.test(loc)) {
          auto y = subinfo.y_min;
          for(std::uint32_t x = subinfo.x_min; x<subinfo.x_min+subinfo.field_width; x += 8) {
            auto new_field = get_bitfield_key(x, y, subinfo.layer);
            //output[new_field] |= Bitfield(0x00000000000000ff);
            output[new_field] |= Bitfield(0xff00000000000000);
          }
        }

        // Line the top edge with top-edge filled spaces
        if(new_spread_up.test(loc)) {
          auto y = subinfo.y_min + subinfo.field_width - 1;
          for(std::uint32_t x = subinfo.x_min; x<subinfo.x_min+subinfo.field_width; x += 8) {
            auto new_field = get_bitfield_key(x, y, subinfo.layer);
            //output[new_field] |= Bitfield(0xff00000000000000);
            output[new_field] |= Bitfield(0x00000000000000ff);
          }
        }
      }
    }
  }
}

bool GrassyBitfield::exists_or_has_subfields(std::uint64_t key) const {
  auto info = unpack_bitfield_key(key);
  auto top_right_corner = get_address(info.x_min + info.field_width-1,
                                      info.y_min + info.field_width-1);
  auto last_possible_subfield = get_bitfield_key(top_right_corner, 0);
  return bitfields.lower_bound(key) != bitfields.upper_bound(last_possible_subfield);
}


std::vector<GrassyBitfield::DrawField> GrassyBitfield::get_draw_fields() const {
  std::vector<DrawField> output;

  for(const auto& pair : bitfields) {
    auto info = unpack_bitfield_key(pair.first);


    DrawField field;
    field.x_min = info.x_min;
    field.y_min = info.y_min;
    field.width = info.field_width;

    auto bitfield = pair.second;
    for(unsigned int y=0; y<8; y++) {
      for(unsigned int x=0; x<8; x++) {
        field.values[y][x] = bitfield.test(8*y + x);
      }
    }

    output.push_back(field);
  }

  return output;
}

std::ostream& operator<<(std::ostream& out, const GrassyBitfield::DrawField& f) {
  out << "(" << f.x_min << ", " << f.y_min << ")"
      << "\t"
      << "Width: " << f.width
      << "\n";

  out << "+--------+\n";
  for(int y=7; y>=0; y--) {
    out << "|";
    for(int x=0; x<8; x++) {
      out << (f.values[y][x] ? "#" : " ");
    }
    out << "|\n";
  }
  out << "+--------+";

  return out;
}

std::ostream& operator<<(std::ostream& out, const GrassyBitfield::Bitfield& f) {
  out << "+--------+\n";
  for(int y=7; y>=0; y--) {
    out << "|";
    for(int x=0; x<8; x++) {
      out << (f.test(y*8 + x) ? "#" : " ");
    }
    out << "|\n";
  }
  out << "+--------+";

  return out;
}
