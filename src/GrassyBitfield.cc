#include "GrassyBitfield.hh"

#include <cassert>
#include <stdexcept>
#include <sstream>


GrassyBitfield::GrassyBitfield(unsigned int num_layers, bool initial_value)
  : num_layers(num_layers) {

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

std::uint64_t GrassyBitfield::get_address(std::uint32_t x, std::uint32_t y) {
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

std::uint64_t GrassyBitfield::get_bitfield_key(std::uint64_t address, unsigned int layer) {
  assert(layer < num_layers);

  // The bitfields are stored according to a key, derivable from the
  // address of the tile being indexed.  The key is the address of the
  // bottom-left tile in the bitfield, plus the layer index.


  // Determine the bottom-left tile in the bitfield by zeroing out the
  // smallest 6*(layer+1) bits.  This will always zero out at least 6
  // bits, so those bits can be used to store the layer number.
  unsigned int bits_to_zero = 6*(layer+1);
  address = (address >> bits_to_zero) << bits_to_zero;

  // Layer number on (0, num_layers) is stored in the smallest 6 bits.
  // Layer number is always less than 64, so this won't collide with
  // the bottom-left section.
  return address + layer;
}

unsigned int GrassyBitfield::get_bitfield_loc(std::uint64_t address, unsigned int layer) {
  // Location in layer 0 is in lowest 6 bits, and each successive
  // layer is stacked above it.
  unsigned int starting_pos = 6*layer;
  return (address >> starting_pos) & 0x3f;
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

  // // Find the lowest level that contains the address
  // unsigned int layer = -1;
  // std::uint64_t key = -1;
  // bool found_layer = false;
  // for(layer=0; layer<num_layers; layer++) {
  //   key = get_bitfield_key(address, layer);
  //   if(bitfields.count(key)) {
  //     found_layer = true;
  //     break;
  //   }
  // }

  // // If nothing else, we should reach the topmost bitfield, and should
  // // never reach here.
  // assert(found_layer);

  // // If the value is already set the way we want, no change needed
  // auto loc = get_bitfield_loc(address, layer);
  // bool current_val = bitfields[key].test(loc);
  // if(current_val == val) {
  //   return;
  // }


  // // If we are on an upper level, add a new bitfield at the lowest level
  // if(layer > 0) {
  //   auto new_key = get_bitfield_key(address, 0);
  //   auto bottom_loc = get_bitfield_loc(address, 0);
  //   Bitfield new_field = current_val ? -1L : 0;
  //   new_field.flip(bottom_loc);
  //   bitfields[new_key] = new_field;
  //   return;
  // }

  // Walk up the layers, starting at the lowest level.  Loop concludes
  // if (a) a bitfield doesn't exists or (b) a bitfield exists and
  // cannot be collapsed.
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

      // Top-most layer is allowed to be non-uniform, so that every
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

std::vector<GrassyBitfield::DrawField> GrassyBitfield::get_draw_fields() const {
  std::vector<DrawField> output;

  for(const auto& pair : bitfields) {
    auto address = pair.first;
    unsigned int layer = address & 7;

    std::uint32_t x = 0;
    std::uint32_t y = 0;
    for(unsigned int i=1; i<10; i++) {
      unsigned int coordinate_bits = 3*i;
      unsigned int x_bits = 6*i;
      unsigned int y_bits = 6*i + 3;
      x |= ((address >> x_bits) & 7) << coordinate_bits;
      y |= ((address >> y_bits) & 7) << coordinate_bits;
    }

    DrawField field;
    field.x_min = x;
    field.y_min = y;
    field.width = 8 << (3*layer);

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
