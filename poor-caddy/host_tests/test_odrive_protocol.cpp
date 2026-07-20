#include "odrive_protocol.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
void testProtocol();
void testControl();
void testMotorPolicy();
int main() {
  testProtocol();
  testControl();
  testMotorPolicy();
  char b[64];
  assert(cart::odrive_protocol::formatVelocity(b, sizeof(b), 0, 1.25F));
  assert(std::strcmp(b, "v 0 1.250000 0\n") == 0);
  assert(!cart::odrive_protocol::formatVelocity(b, sizeof(b), 2, 1));
  float f{};
  assert(cart::odrive_protocol::parseFloat(" -1.25\r\n", f) && f == -1.25F);
  assert(!cart::odrive_protocol::parseFloat("nan", f));
  std::uint64_t u{};
  assert(cart::odrive_protocol::parseUint64("0x100000000", u) &&
         u == 0x100000000ULL);
  assert(!cart::odrive_protocol::parseUint64("-1", u));
}
