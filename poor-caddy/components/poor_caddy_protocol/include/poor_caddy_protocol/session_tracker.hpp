#pragma once
#include "poor_caddy_protocol/control.hpp"
#include <cstdint>
namespace poor_caddy {
enum class ValidationResult : std::uint8_t {
  Valid,
  WrongSender,
  WrongLength,
  WrongMagic,
  WrongVersion,
  BadCrc,
  InvalidSpeed,
  InvalidSteering,
  DuplicateSequence,
  OldSequence,
  ConflictingSession
};
struct AcceptedCommand {
  ControlState control;
  std::uint64_t received_at_ms;
  std::uint32_t session_id;
  std::uint32_t sequence;
};
class SessionTracker {
public:
  ValidationResult consider(std::uint32_t sid, std::uint32_t seq,
                            ControlState ctl, std::uint64_t now,
                            bool active_timed_out, AcceptedCommand &out);

private:
  bool active_{};
  std::uint32_t sid_{};
  std::uint32_t last_{};
  bool candidate_{};
  std::uint32_t cand_sid_{};
  std::uint32_t cand_last_{};
  std::uint8_t cand_count_{};
};
} // namespace poor_caddy
