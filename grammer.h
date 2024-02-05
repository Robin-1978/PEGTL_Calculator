#pragma once

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace calc {

struct plus : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'+'>>,
                              tao::pegtl::space> {};
struct minus : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'-'>>,
                               tao::pegtl::space> {};
struct multiply : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'*'>>,
                                  tao::pegtl::space> {};
struct divide : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'/'>>,
                                tao::pegtl::space> {};
struct assign : tao::pegtl::pad<tao::pegtl::plus<tao::pegtl::one<'='>>,
                                tao::pegtl::space> {};

// Define the rule for matching integers
struct integer : tao::pegtl::plus<tao::pegtl::digit> {};

struct identifier_first : tao::pegtl::ranges<'a', 'z', 'A', 'Z'> {};
struct identifier_next : tao::pegtl::ranges<'a', 'z', 'A', 'Z', '0', '9', '_'> {
};
struct identifier
    : tao::pegtl::seq<identifier_first, tao::pegtl::star<identifier_next>> {};

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

struct factor : tao::pegtl::sor<integer, parenth, identifier> {};

// Define the term rule for handling multiplication and division
struct term : tao::pegtl::list<factor, tao::pegtl::sor<multiply, divide>> {};

// Define the expression rule for handling addition and subtraction
struct expression : tao::pegtl::list<term, tao::pegtl::sor<plus, minus>> {};

struct st_exp : expression{};

struct st_assign : tao::pegtl::seq<identifier, assign, expression>{};

struct statement : tao::pegtl::sor<st_assign, st_exp>{};

// Define the top-level grammar rule
struct grammar : tao::pegtl::must<statement, tao::pegtl::eof> {};

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
    Rule, tao::pegtl::parse_tree::store_content::on<integer, identifier>,
    tao::pegtl::parse_tree::remove_content::on<plus, minus, multiply, divide, assign>,
    rearrange::on<factor, term, expression, st_exp, st_assign, statement, grammar>>;

}
