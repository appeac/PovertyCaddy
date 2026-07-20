#include "poor_caddy_protocol/protocol.hpp"
#include "poor_caddy_protocol/control.hpp"
#include "poor_caddy_protocol/sequence.hpp"
#include "poor_caddy_protocol/session_tracker.hpp"
#include "poor_caddy_protocol/ramp.hpp"
#include "poor_caddy_protocol/motor_logic.hpp"
#include <cassert>
#include <cstring>
using namespace poor_caddy;
int main(){ assert(crc32(reinterpret_cast<const std::uint8_t*>("123456789"),9)==0xCBF43926U); auto bytes=encodeControlPacket(1,0,{SpeedLevel::Forward2,SteeringState::Left}); ControlPacket p{}; assert(decodeControlPacket(bytes.data(),bytes.size(),p)==DecodeResult::Valid); bytes[13]=9; bytes[20]=0; assert(decodeControlPacket(bytes.data(),bytes.size(),p)==DecodeResult::BadCrc); assert(classifySequence(true,0xFFFFFFFEU,1U)==SequenceClass::Newer); assert(classifySequence(true,100U,100U)==SequenceClass::Duplicate); SessionTracker st; AcceptedCommand out{}; assert(st.consider(7,1,{SpeedLevel::Forward1,SteeringState::Straight},0,true,out)==ValidationResult::ConflictingSession); assert(st.consider(7,2,{SpeedLevel::Forward1,SteeringState::Straight},50,true,out)==ValidationResult::ConflictingSession); assert(st.consider(7,3,{SpeedLevel::Forward1,SteeringState::Straight},100,true,out)==ValidationResult::Valid); Ramp r(250); r.reset(0); for(int i=0;i<200;i++) r.update(250,5); assert(r.value()==250); DriveVelocityConfig config{{{300,600,1200}},{{500,750,1000}},-1,1,1000}; auto targets=driveTargets({SpeedLevel::Forward2,SteeringState::Left},config); assert(targets.left==-450&&targets.right==600); targets=driveTargets({SpeedLevel::Forward3,SteeringState::Straight},config); assert(targets.left==-1000&&targets.right==1000); return 0; }
