#pragma once

#include <unordered_map>

#include "vm.h"

namespace calc
{
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
} // namespace calc
