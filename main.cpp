#include <iostream>
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <vector>

namespace calc {

struct plus : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'+'>>,
                              tao::pegtl::space> {};
struct minus : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'-'>>,
                               tao::pegtl::space> {};
struct multiply : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'*'>>,
                                  tao::pegtl::space> {};
struct divide : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'/'>>,
                                tao::pegtl::space> {};
// Define the rule for matching integers
struct integer : tao::pegtl::plus<tao::pegtl::digit> {};

// Define the rules for parentheses
struct open_parenthesis
    : tao::pegtl::pad<tao::pegtl::one<'('>, tao::pegtl::space> {};
struct close_parenthesis
    : tao::pegtl::pad<tao::pegtl::one<')'>, tao::pegtl::space> {};

struct expression;
// Define the factor rule for handling parentheses and integers
struct parenth
    : tao::pegtl::sor<integer, tao::pegtl::seq<open_parenthesis, expression,
                                               close_parenthesis>> {};

struct factor : tao::pegtl::sor<integer, parenth> {};

// Define the term rule for handling multiplication and division
struct term : tao::pegtl::list<factor, tao::pegtl::sor<multiply, divide>> {};

// Define the expression rule for handling addition and subtraction
struct expression : tao::pegtl::list<term, tao::pegtl::sor<plus, minus>> {};

// Define the top-level grammar rule
struct grammar : tao::pegtl::must<expression, tao::pegtl::eof> {};

enum OP { PLUS, MINUS, MULTIPLY, DIVIDE };

// after a node is stored successfully, you can add an optional transformer like
// this:
struct rearrange
    : tao::pegtl::parse_tree::apply<rearrange>  // allows bulk selection, see
                                                // selector<...>
{
  // recursively rearrange nodes. the basic principle is:
  //
  // from:          PROD/EXPR
  //                /   |   \          (LHS... may be one or more children,
  //                followed by OP,)
  //             LHS... OP   RHS       (which is one operator, and RHS, which is
  //             a single child)
  //
  // to:               OP
  //                  /  \             (OP now has two children, the original
  //                  PROD/EXPR and RHS)
  //         PROD/EXPR    RHS          (Note that PROD/EXPR has two fewer
  //         children now)
  //             |
  //            LHS...
  //
  // if only one child is left for LHS..., replace the PROD/EXPR with the child
  // directly. otherwise, perform the above transformation, then apply it
  // recursively until LHS... becomes a single child, which then replaces the
  // parent node and the recursion ends.
  template <typename Node, typename... States>
  static void transform(std::unique_ptr<Node>& n, States&&... st) {
    if (n->children.size() == 1) {
      n = std::move(n->children.back());
    } else {
      n->remove_content();
      auto& c = n->children;
      auto r = std::move(c.back());
      c.pop_back();
      auto o = std::move(c.back());
      c.pop_back();
      o->children.emplace_back(std::move(n));
      o->children.emplace_back(std::move(r));
      n = std::move(o);
      transform(n->children.front(), st...);
    }
  }
};



template <typename Rule>
using selector = tao::pegtl::parse_tree::selector<
    Rule, tao::pegtl::parse_tree::store_content::on<integer>,
    tao::pegtl::parse_tree::remove_content::on<plus, minus, multiply, divide>,
    rearrange::on<factor, term, expression, grammar>>;


int evaluate(const tao::pegtl::parse_tree::node& node) {
  if (node.is_root()) return evaluate(*node.children[0]);  // skip root

  if (node.children.empty()) {
    if (node.is_type<integer>()) {
      return std::stoi(node.string());
    } else {
      std::cerr << "Node error" << std::endl;
      return 0;
    }
  } else {
    if (node.children.size() == 2) {
      if (node.is_type<plus>()) {
        return evaluate(*node.children[0]) + evaluate(*node.children[1]);
      } else if (node.is_type<minus>()) {
        return evaluate(*node.children[0]) - evaluate(*node.children[1]);
      } else if (node.is_type<multiply>()) {
        return evaluate(*node.children[0]) * evaluate(*node.children[1]);
      } else if (node.is_type<divide>()) {
        auto l = evaluate(*node.children[0]);
        auto r = evaluate(*node.children[1]);
        if (r == 0) {
          std::cerr << "divide zero" << std::endl;
          return 0;
        }
        return l / r;
      } else {
        std::cerr << "Node error" << std::endl;
        return 0;
      }
    } else {
      std::cerr << "Node error" << std::endl;
      return 0;
    }
  }
}

}  // namespace calc

int main(int argc, char* argv[]) {
  if (TAO_PEGTL_NAMESPACE::analyze<calc::grammar>() != 0) {
    return 1;
  }

  std::cout << "Type [q or Q] to quit\n\n";
  std::string str;

  while (!std::getline(std::cin, str).fail()) {
    if (str.empty() || str[0] == 'q' || str[0] == 'Q') {
      break;
    }
    TAO_PEGTL_NAMESPACE::memory_input in(str, "std::cin");
    try {
      auto root = tao::pegtl::parse_tree::parse<
          calc::grammar, tao::pegtl::parse_tree::node, calc::selector>(in);
      tao::pegtl::parse_tree::print_dot(std::cout, *root);
      auto value = calc::evaluate(*root);
      std::cout << "= " << value << std::endl;
    } catch (tao::pegtl::parse_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 0;
}