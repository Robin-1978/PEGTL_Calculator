#pragma once

#include <iostream>

#include "grammer.h"

namespace calc {

int evaluate(const tao::pegtl::parse_tree::node& node,
             std::unordered_map<std::string, int>& memory) {
  if (node.is_root()) return evaluate(*node.children[0], memory);  // skip root

  if (node.children.empty()) {
    if (node.is_type<integer>()) {
      return std::stoi(node.string());
    } else if (node.is_type<identifier>()) {
      auto it = memory.find(node.string());
      if (it != memory.end()) {
        return it->second;
      } else {
        std::cerr << "vairavle not found:" << node.string() << std::endl;
        return 0;
      }
    } else {
      std::cerr << "Node error" << std::endl;
      return 0;
    }
  } else {
    if (node.children.size() == 2) {
      if (node.is_type<plus>()) {
        return evaluate(*node.children[0], memory) +
               evaluate(*node.children[1], memory);
      } else if (node.is_type<minus>()) {
        return evaluate(*node.children[0], memory) -
               evaluate(*node.children[1], memory);
      } else if (node.is_type<multiply>()) {
        return evaluate(*node.children[0], memory) *
               evaluate(*node.children[1], memory);
      } else if (node.is_type<divide>()) {
        auto l = evaluate(*node.children[0], memory);
        auto r = evaluate(*node.children[1], memory);
        if (r == 0) {
          std::cerr << "divide zero" << std::endl;
          return 0;
        }
        return l / r;
      } else if (node.is_type<modulo>()) {
        auto l = evaluate(*node.children[0], memory);
        auto r = evaluate(*node.children[1], memory);
        return l % r;
      } else if (node.is_type<assign>()) {
        memory[node.children[0]->string()] =
            evaluate(*node.children[1], memory);
        return 0;
      } else {
        std::cerr << "Node error" << std::endl;
        return 0;
      }
    } else if (node.children.size() == 1) {
      if (node.is_type<print>()) {
        std::cout << "=" << evaluate(*node.children[0], memory) << std::endl;
        return 0;
      }
    } else {
      if (node.is_type<caller>()) {
        // TODO : call function
      } else {
        std::cerr << "Node error" << std::endl;
      }
      return 0;
    }
  }
  return 0;
}

}  // namespace calc