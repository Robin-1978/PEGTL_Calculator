#pragma once

#include <iostream>
#include "grammer.h"

namespace calc{

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

}