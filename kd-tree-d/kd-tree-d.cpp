// kd-tree-b issues addressed: 
// - 2D only
// - needs to know about value_type
// - query not very flexible

#include <iostream>
#include <array>
#include <vector>
#include <deque>
#include <memory>


template <
  size_t Dim,
  typename T,
  typename TC,
  typename CoorPolicy      // TC operator()(const T&, size_t dim)
>
class KDtree
{
public:
  static constexpr size_t dim = Dim;
  using value_type = T;
  using coor_type = TC;

  KDtree()
  {}

  template <typename IT>
  void build(IT first, IT last)
  {
    nodes_.clear();
    if (first != last) {
      nodes_.emplace_back(*first, CoorPolicy{}(*first, 0));
      for (++first; first != last; ++first) {
        do_insert(std::addressof(nodes_.front()), *first, 0);
      }
    }
  }

  template <
    typename Overlap,   // pair<bool,bool> operator()(coor_type a, size_t dim)
    typename Fun        // called for each visited node
  >
  void query(Overlap overlap, Fun fun)
  {
    if (nodes_.empty()) return;
    do_query(std::addressof(nodes_.front()), overlap, fun, 0);
  }

private:
  struct Node
  {
    Node(value_type& val, coor_type c) :
      coor(c),
      pval(std::addressof(val))
    {}

    coor_type coor;
    value_type* pval = nullptr;
    Node* lt = nullptr;
    Node* ge = nullptr;
  };


  Node* do_insert(Node* root, value_type& val, size_t depth)
  {
    const auto coor = CoorPolicy{}(val, depth % dim);
    if (nullptr == root) {
      nodes_.emplace_back(val, coor);
      return std::addressof(nodes_.back());
    }

    // and decide the less-than or greater-or-equal subtree 
    if (coor < root->coor) {
      root->lt = do_insert(root->lt, val, depth + 1);
    }
    else {
      root->ge = do_insert(root->ge, val, depth + 1);
    }
    return root;
  }

  template <typename Overlap, typename Fun>
  void do_query(Node* root, Overlap& overlap, Fun& fun, size_t depth)
  {
    if (nullptr == root) {
      return;
    }
    auto ov = overlap(root->coor, depth % dim);
    if (ov.first && ov.second) {
      fun(*root->pval);
    }
    if (ov.first) {
      do_query(root->lt, overlap, fun, depth + 1);
    }
    if (ov.second) {
      do_query(root->ge, overlap, fun, depth + 1);
    }
  }

  std::deque<Node> nodes_;
};


using point_t = std::array<float, 2>;


float distance2(const point_t& a, const point_t& b)
{
  float dd = 0.f;
  for (size_t i = 0; i < a.size(); ++i) {
    dd += (a[i] - b[i]) * (a[i] - b[i]);
  }
  return dd;
}


std::ostream& operator<<(std::ostream& os, const point_t& point)
{
  for (size_t i = 0; i < point.size(); ++i) {
    os << point[i] << ' ';
  }
  return os;
}


struct Individual
{
  point_t position;
  std::array<float, 100> state;  // make it expensive to copy
};


// the policy required by KDtree
struct CoorPolicy
{
  float operator()(const Individual& ind, size_t dim) { return ind.position[dim]; };
};


int main()
{
  std::vector<Individual> pop{ {3, 6}, {17, 15}, {13, 14}, {6, 12}, {9, 1}, {2, 7}, {18, 17} };


  // mind constness
  auto kdtree = KDtree<2, const Individual, float, CoorPolicy>{};
  kdtree.build(pop.cbegin(), pop.cend());

  /*

  x                    x ------- lt -------  {3,6}  ++++++ ge +++++++ x
                       |                                              |
  y                  {2,7}                                  y ----- {17,15} +++++ y
                                                            |                     |
  x                                                  x - {13,14}               {18,17}
                                                     |
  y                                            y - {6,12}
                                               |
  x                                          {9,1}

  */

  //point[cc] - radius) < root->center[cc]) {


  point_t pivot{ 10, 10 };
  float radius = 5.0f;
  kdtree.query(
    [&](float coor, size_t dim) { 
      return (std::pair<bool,bool>((pivot[dim] - radius) < coor, (pivot[dim] + radius) >= coor));
    },
    [&](const Individual& ind) {
      if (distance2(ind.position, pivot) <= radius * radius) {
        std::cout << ind.position << '\n';
      }
    }
  );
  return 0;
}
