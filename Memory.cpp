#include "Memory.hpp"
#include <cstddef>
#include <vector>

namespace TTT {

auto Memory::get_dead() -> std::vector<Cell*> {
  const size_t n = block.size();

  std::vector<Cell*> dead;
  dead.reserve(n / 4); // TODO better estimation tahn 25%

  for (size_t i = 0; i < n; ++i)
    if (block[i].mark)
      dead.push_back(block.data() + i);

  return dead;
}
/*
 * Cleanup Memory Block
 */
auto Memory::compact(std::vector<Cell *> killed, std::vector<Cell> &write)
    -> RefSanitize {
  Cell *read_ptr = block.data(), *write_ptr = write.data();
  for (Cell *c : killed) {
    while (c != read_ptr)
      *write_ptr++ = *read_ptr++;

    ++read_ptr;
  }

  write.resize(write_ptr - write.data());

  return {std::move(killed), write.data() - block.data()};
}

void Memory::sanitize_all(const RefSanitize &correction) {

  const auto offset = [&killed = correction.killed](Reference &ref) {
    const size_t n = killed.size();
    for (size_t i = 0; i < n; ++i) {
      if (killed[i] > ref.ptr)
        return ref.ptr -= i - 1;
    }
    return ref.ptr -= n;
  };

  for (Cell &cell : block)
    cell.val.apply_refs(offset);

  if (correction.base_offset != 0) {
    const auto rebase = [offset = correction.base_offset](Reference &ref) {
      return ref.ptr += offset;
    };

    for (Cell &cell : block)
      cell.val.apply_refs(rebase);
  }
}

void Memory::sanitize(Reference &ref, const RefSanitize &correction) {
  const size_t n = correction.killed.size();
  for (size_t i = 0; i < n; ++i) {
    if (correction.killed[i] > ref.ptr) {
      ref.ptr -= i - 1;
      goto base;
    }
  }
  ref.ptr -= n;

base:
  ref.ptr += correction.base_offset;
}

auto Memory::make_space() -> RefSanitize {
  const auto dead = get_dead();
  RefSanitize correction;

  if (dead.size() < grow_threshold) {
    std::vector<Cell> transfer;
    transfer.reserve(block.capacity() * grow_factor);
    correction = compact(dead, transfer);
    block = transfer;
  } else
    correction = compact(dead, block);

  for (Cell &cell : block)
    cell.val.apply_refs(
        [&correction](Reference &ref) { Memory::sanitize(ref, correction); });
  return correction;
}

  auto Memory::available() -> size_t { return block.capacity() - block.size(); }
  auto Memory::alloc() -> Reference { return {&block.emplace_back()}; }

  void Memory::clear_marks() {
    for (Cell &c : block)
      c.mark = false;
}

void Memory::mark(Reference ref) {
  if (ref.ptr->mark)
    return;
  
  ref.ptr->mark = true;
  ref.ptr->val.apply_refs([this](const Reference &ref) { mark(ref); });
}

} // namespace TTT
