// pretty poor kd tree

#include <iostream>
#include <array>
#include <vector>
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


class KDtree
{
public:
  template <typename IT>
  KDtree(IT first, IT last)
  {
    if (first != last) {
      root_.reset(new Node{ *first });
    }
    for (++first; first != last; ++first) {
      do_insert(root_.get(), *first, 0);
    }
  }

  template <typename Fun>
  void query(const point_t& point, float radius, Fun fun)
  {
    do_query(root_.get(), point, radius, fun, 0);
  }

private:
  struct Node
  {
    Node(const point_t& point) : pt(point) {}
    ~Node()
    {
      if (lt) delete lt;
      if (ge) delete ge;
    }

    point_t pt;
    Node* lt = nullptr;
    Node* ge = nullptr;
  };


  Node* do_insert(Node* root, const point_t& point, size_t depth)
  {
    if (nullptr == root) {
      return new Node{ point };
    }
    auto cc = depth % point.size();    // select coordinate for comparison
    // and decide the less-than or greater-or-equal subtree 
    if (point[cc] < (root->pt[cc])) {
      root->lt = do_insert(root->lt, point, depth + 1);
    }
    else {
      root->ge = do_insert(root->ge, point, depth + 1);
    }
    return root;
  }

  template <typename Fun>
  void do_query(Node* root, const point_t& point, float radius, Fun& fun, size_t depth)
  {
    if (nullptr == root) {
      return;
    }
    if (distance2(point, root->pt) <= radius * radius) {
      fun(root->pt);
    }
    auto cc = depth % point.size();    // select coordinate for comparison
    if ((point[cc] - radius) < root->pt[cc]) {
      do_query(root->lt, point, radius, fun, depth + 1);
    }
    if ((point[cc] + radius) >= root->pt[cc]) {
      do_query(root->ge, point, radius, fun, depth + 1);
    }
  }

  std::unique_ptr<Node> root_;
};


template <typename IT, typename Fun>
void brute_force_query(IT first, IT last, const point_t& point, float radius, Fun fun)
{
  for (; first != last; ++first) {
    if (distance2(point, *first) <= radius * radius) {
      fun(*first);
    }
  }
}


int main()
{
  std::vector<point_t> points{ {3, 6}, {17, 15}, {13, 14}, {6, 12}, {9, 1}, {2, 7}, {18, 17} };

  auto qcenter = point_t{ 10, 10 };
  auto qradius = 5.0f;
  brute_force_query(points.begin(), points.end(), qcenter, qradius, [](const point_t& point) {
    std::cout << point << '\n';
  });
  std::cout << '\n';
 
  auto kdtree = KDtree{points.cbegin(), points.cend()};

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
  kdtree.query(qcenter, qradius, [](const point_t& point) {
    std::cout << point << '\n';
  });
  return 0;
}
