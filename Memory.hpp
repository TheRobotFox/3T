#include <cstddef>
#include <vector>
#include "Value.hpp"

namespace TTT {

class Cell {
  friend class Memory;
  Value val;
  bool mark;
};

class RefSanitize {
  friend class Memory;
  std::vector<Cell *> killed;
  long long base_offset;
  RefSanitize(std::vector<Cell *> killed, long long base_offset)
  : killed(std::move(killed)), base_offset(base_offset) {}
  RefSanitize() = default;
};

class Memory {

  float grow_factor = 2;
  size_t grow_threshold = 2 << 10;

  std::vector<Cell> block;
  auto get_dead() -> std::vector<Cell *>;
  auto compact(std::vector<Cell *> kill, std::vector<Cell> &write)
      -> RefSanitize;
  void sanitize_all(const RefSanitize &correction);

public:
  Memory(size_t initial) { block.reserve(initial); }

  auto alloc() -> Reference;
  auto make_space() -> RefSanitize;
  auto get(Reference ref) -> Value * {return &ref.ptr->val;}
  auto available() -> size_t;
  void clear_marks();
  void mark(Reference);
  static void sanitize(Reference &ref, const RefSanitize &correction);
};
} // namespace TTT
