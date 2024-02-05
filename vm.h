#pragma once
#include <iostream>
#include <vector>

#include "grammer.h"

namespace calc {

enum OpCode : unsigned char {
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
          break;
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

}  // namespace calc
