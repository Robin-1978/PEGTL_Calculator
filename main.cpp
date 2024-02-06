#include <iostream>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <vector>

#include "compiler.h"
#include "evaluator.h"
#include "vm.h"

int main(int argc, char* argv[]) {
  if (TAO_PEGTL_NAMESPACE::analyze<calc::grammar>() != 0) {
    return 1;
  }

  std::cout << R"(Type [? expression] to show result.
Type [identitifier = expression] to store viriable.
Type [q or Q] to quit)
  
)";

  std::string str;

  std::unordered_map<std::string, int> memory;
  calc::CompileState state;
  calc::vm vm;
  while (!std::getline(std::cin, str).fail()) {
    if (str.empty() || str[0] == 'q' || str[0] == 'Q') {
      break;
    }
    TAO_PEGTL_NAMESPACE::memory_input in(str, "std::cin");
    try {
      auto root = tao::pegtl::parse_tree::parse<
          calc::grammar, tao::pegtl::parse_tree::node, calc::selector>(in);
      // auto root = tao::pegtl::parse_tree::parse<calc::grammar>(in);
      // tao::pegtl::parse_tree::print_dot(std::cout, *root);
      
      
      // Evaluate
      calc::evaluate(*root, memory);

      // Compile and run
      std::string code;
      if (calc::compile(*root, state, code)) {
        unsigned ip = 0;
        vm.run(code, ip);
        std::cout << " (vm[" << code.size() << "," << vm.stack.size() << "])"
                  << std::endl;
        for (auto& name : state.variables) {
          std::cout << name.first << "[" << vm.memory[name.second] << "]\t";
        }
        std::cout << std::endl;
      } else {
        std::cerr << "Compile error" << std::endl;
      }
    } catch (tao::pegtl::parse_error& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 0;
}