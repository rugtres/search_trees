// kd-tree-c issues addressed: 
// - points are not terrible interesting

#include <iostream>
#include <array>
#include <vector>
#include <deque>
#include <memory>


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



template <typename T>
class KDtree
{
public:
  using value_type = T;

  KDtree() 
  {}

  template <typename IT>
  void build(IT first, IT last)
  {
    nodes_.clear();
    if (first != last) {
      nodes_.emplace_back(*first);
      for (++first; first != last; ++first) {
        do_insert(std::addressof(nodes_.front()), *first, 0);
      }
    }
  }

  template <typename Fun>
  void query(const point_t& point, float radius, Fun fun)
  {
    if (nodes_.empty()) return;
    do_query(std::addressof(nodes_.front()), point, radius, fun, 0);
  }

private:
  struct Node
  {
    Node(value_type& val) : 
      center(val.position), 
      pval(std::addressof(val))
    {}

    point_t center;
    value_type* pval = nullptr;
    Node* lt = nullptr;
    Node* ge = nullptr;
  };


  Node* do_insert(Node* root, value_type& val, size_t depth)
  {
    if (nullptr == root) {
      nodes_.emplace_back(val);
      return std::addressof(nodes_.back());
    }
    auto cc = depth % root->center.size();    // select coordinate for comparison
    // and decide the less-than or greater-or-equal subtree 
    if (val.position[cc] < (root->center[cc])) {
      root->lt = do_insert(root->lt, val, depth + 1);
    }
    else {
      root->ge = do_insert(root->ge, val, depth + 1);
    }
    return root;
  }

  template <typename Fun>
  void do_query(Node* root, const point_t& point, float radius, Fun& fun, size_t depth)
  {
    if (nullptr == root) {
      return;
    }
    if (distance2(point, root->center) <= radius * radius) {
      fun(*root->pval);
    }
    auto cc = depth % point.size();    // select coordinate for comparison
    if ((point[cc] - radius) < root->center[cc]) {
      do_query(root->lt, point, radius, fun, depth + 1);
    }
    if ((point[cc] + radius) >= root->center[cc]) {
      do_query(root->ge, point, radius, fun, depth + 1);
    }
  }

  std::deque<Node> nodes_;
};


template <typename IT, typename Fun>
void brute_force_query(IT first, IT last, const point_t& point, float radius, Fun fun)
{
  for (; first != last; ++first) {
    if (distance2(point, first->position) <= radius * radius) {
      fun(*first);
    }
  }
}


int main()
{
  std::vector<Individual> pop{ {3, 6}, {17, 15}, {13, 14}, {6, 12}, {9, 1}, {2, 7}, {18, 17} };

  auto qcenter = point_t{ 10, 10 };
  auto qradius = 5.0f;
  brute_force_query(pop.begin(), pop.end(), qcenter, qradius, [](const Individual& ind) {
    std::cout << ind.position << '\n';
  });
  std::cout << '\n';
 
  // mind constness
  auto kdtree = KDtree<const Individual>{};
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
  kdtree.query(qcenter, qradius, [](const Individual& ind) {
    std::cout << ind.position << '\n';
  });
  return 0;
}
