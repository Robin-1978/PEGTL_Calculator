#include <iostream>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <vector>


#include "vm.h"
#include "compiler.h"
#include "evaluator.h"

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