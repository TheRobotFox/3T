namespace TTT {

  class Memory;
  class Cell;
class Reference {
  friend class Memory;
  Cell *ptr;
  Reference(Cell *v) : ptr(v) {}
public:
  auto operator==(const Reference &other) const -> bool = default;
};

} // namespace TTT
