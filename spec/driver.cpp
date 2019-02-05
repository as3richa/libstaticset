#include "driver.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <vector>

#define RED "\u001b[31m"
#define GREEN "\u001b[32m"
#define UNCOLOR "\u001b[39m"

struct Test {
  const std::string what;
  const std::function<void()> body;
  Test(std::string what, std::function<void()> body) : what(what), body(body) { ; }
};

struct Suite {
  const std::string what;
  std::vector<Test> tests;
  std::vector<Suite> children;
  Suite(std::string what) : what(what) { ; }
};

class ExpectationFailure : public std::exception {
public:
  const std::string filename;
  const int line;
  const std::string cond;

  ExpectationFailure(std::string filename, int line, std::string cond) : filename(filename), line(line), cond(cond) {
    ;
  }
};

struct State {
  Suite root;
  std::vector<Suite *> stack;
  State() : root(""), stack({&root}) { ; }
};

static State *state;
size_t n_tests;
size_t n_failed;
std::vector<std::string> messages;

Suite &getRoot() {
  if (state == nullptr) {
    state = new State();
  }
  return state->root;
}

std::vector<Suite *> &getStack() {
  if (state == nullptr) {
    state = new State();
  }
  return state->stack;
}

int _describe(std::string what, std::function<void()> body) {
  auto &stack = getStack();

  Suite &parent = *stack.back();
  parent.children.emplace_back(what);
  Suite &suite = parent.children.back();

  stack.push_back(&suite);
  body();
  stack.pop_back();

  return 0;
}

void it(std::string what, std::function<void()> body) {
  const auto &stack = getStack();
  Suite &suite = *stack.back();
  suite.tests.emplace_back(what, body);
  n_tests++;
}

void _expect(bool okay, std::string filename, int line, std::string cond) {
  if (okay) {
    return;
  }
  throw ExpectationFailure(filename, line, cond);
}

bool runSuite(Suite &suite, size_t depth) {
  const std::string indent(2 * depth + 1, ' ');

  std::cout << indent << "= " << suite.what << std::endl;

  bool okay = true;

  for (const auto &test : suite.tests) {
    try {
      test.body();
      std::cout << GREEN << indent << "  + " << test.what << UNCOLOR << std::endl;
    } catch (ExpectationFailure ex) {
      n_failed++;
      std::cout << RED << indent << "  ! " << test.what << UNCOLOR << std::endl;
      messages.push_back(ex.filename + ":" + std::to_string(ex.line) + ": !(" + ex.cond + ")");
    }
  }

  // std::sort(suite.children.begin(), suite.children.end(), orderByWhat);

  for (auto &child : suite.children) {
    if (!runSuite(child, depth + 1)) {
      okay = false;
    }
  }

  return okay;
}

int main(void) {
  Suite &root = getRoot();

  for (auto &suite : root.children) {
    runSuite(suite, 0);
  }

  if (!messages.empty()) {
    std::cout << std::endl << "Failed expectations:" << std::endl;
    for (auto message : messages) {
      std::cout << RED << "- " << message << UNCOLOR << std::endl;
    }
  }

  std::cout << std::endl << "Ran " << n_tests << " test(s); " << n_failed << " failure(s)." << std::endl;

  return ((n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
