#ifndef LIBSTATICSET_DRIVER_H
#define LIBSTATICSET_DRIVER_H

#include <functional>
#include <string>

#define STRINGIFY(v) STRINGIFY2(v)
#define STRINGIFY2(v) #v

#define CONCAT(x, y) x##y
#define DUMMY_IDENTIFIER(suffix) CONCAT(dummy, suffix)

#define describe const static int DUMMY_IDENTIFIER(__LINE__) = (DUMMY_IDENTIFIER(__LINE__), _describe)
int _describe(std::string what, std::function<void()> body);

void it(std::string what, std::function<void()> body);

#define expect(cond) _expect((cond), __FILE__, __LINE__, STRINGIFY(cond))
void _expect(bool okay, std::string filename, int line, std::string cond);

#endif
