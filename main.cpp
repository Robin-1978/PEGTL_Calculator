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
    Rule, tao::pegtl::parse_tree::store_content::on<integer, identifier>,
    tao::pegtl::parse_tree::remove_content::on<plus, minus, multiply, divide>,
    rearrange::on<factor, term, expression, grammar>>;

enum OpCode : uint8_t {
  OP_UNKNOWN,
  OP_PUSH,      // op number(4 bytes)
  OP_POP,       // op
  OP_STOR,      // op address
  OP_FETCH,     // op address
  OP_PLUS,      // op
  OP_MINUS,     // op
  OP_MULTIPLY,  // op
  OP_DIVIDE,    // op
};

struct vm {
  std::vector<int> stack;
  std::vector<int> memory;

  bool run(std::string& code, unsigned& ip) {
    while (ip < code.size()) {
      OpCode op = (OpCode)code[ip++];
      switch (op) {
        case OP_PUSH:
          stack.push_back(*(int*)&code[ip]);
          ip += 4;
          break;
        case OP_POP:
          stack.pop_back();
        case OP_STOR: {
          uint8_t address = (uint8_t)code[ip++];
          if (address < memory.size()) {
            memory[address] = stack.back();
            stack.pop_back();
          } else if (address == memory.size()) {
            memory.push_back(stack.back());
            stack.pop_back();
          } else {
            std::cerr << "Error address" << std::endl;
          }
        } break;
        case OP_FETCH: {
          uint8_t address = (uint8_t)code[ip++];
          if (address < memory.size()) {
            stack.push_back(memory[address]);
          } else {
            std::cerr << "Error address" << std::endl;
          }
        } break;
        case OP_PLUS:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] + stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_MINUS:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] - stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_MULTIPLY:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] * stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_DIVIDE:
          if (stack[stack.size() - 1] == 0) {
            std::cerr << "Divide zero" << std::endl;
          } else {
            stack[stack.size() - 2] =
                stack[stack.size() - 2] / stack[stack.size() - 2];
            stack.pop_back();
          }
          break;
        default:
          std::cerr << "Unknown OP" << std::endl;
          break;
      }
    }
    return true;
  }
};

struct CompileState {
  std::unordered_map<std::string, unsigned> variables;
  uint8_t current{};
  uint8_t GetAddress(const std::string& name) {
    auto it = variables.find(name);
    if (it == variables.end()) {
      variables[name] = current++;
      return current - 1;
    } else {
      return it->second;
    }
  }
};

void append_op(OpCode op, std::string& code) { code += (char)op; }

void append_number(int num, std::string& code) {
  code.resize(code.size() + sizeof(int));
  *(int*)&code[code.size() - sizeof(int)] = num;
}

void append_address(uint8_t address, std::string& code) {
  code += (char)address;
}

bool compile(const tao::pegtl::parse_tree::node& node, CompileState& state,
             std::string& code) {
  if (node.is_root())
    return compile(*node.children[0], state, code);  // skip root

  if (node.children.empty()) {
    if (node.is_type<integer>()) {
      append_op(OP_PUSH, code);
      append_number(std::stoi(node.string()), code);
      return true;
    } else {
      std::cerr << "Node error" << std::endl;
      return false;
    }
  } else {
    if (node.children.size() == 2) {
      if (node.is_type<plus>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_PLUS, code);
        return true;
      } else if (node.is_type<minus>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_MINUS, code);
        return true;
      } else if (node.is_type<multiply>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_MULTIPLY, code);
        return true;
      } else if (node.is_type<divide>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_DIVIDE, code);
        return true;
      } else {
        std::cerr << "Node error" << std::endl;
        return false;
      }
    } else {
      std::cerr << "Node error" << std::endl;
      return false;
    }
  }
};

bool compile(const tao::pegtl::parse_tree::node& node, std::string& code) {
  CompileState state;
  return compile(node, state, code);
};

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
      std::cout << value << " (eval)" << std::endl;

      std::string code;
      if (calc::compile(*root, code)) {
        calc::vm vm;
        unsigned ip = 0;
        vm.run(code, ip);
        std::cout << vm.stack[0] << " (vm[" << code.size() << "])" << std::endl;
      } else {
        std::cerr << "Compile error" << std::endl;
      }
    } catch (tao::pegtl::parse_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 0;
}