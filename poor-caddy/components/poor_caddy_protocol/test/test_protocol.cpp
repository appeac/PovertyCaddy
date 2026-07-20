#include "poor_caddy_protocol/protocol.hpp"
#include "poor_caddy_protocol/sequence.hpp"
#include "poor_caddy_protocol/session_tracker.hpp"
#include "poor_caddy_protocol/ramp.hpp"
#include "poor_caddy_protocol/motor_logic.hpp"
#include <cassert>
#include <cstring>
using namespace poor_caddy;
int main(){
 assert(crc32(reinterpret_cast<const std::uint8_t*>("123456789"),9)==0xCBF43926U);
 auto bytes=encodeControlPacket(1,0,{SpeedLevel::Forward2,SteeringState::Left}); ControlPacket p{};
 assert(decodeControlPacket(bytes.data(),bytes.size(),p)==DecodeResult::Valid);
 bytes[13]=9; bytes[20]=0;
 assert(decodeControlPacket(bytes.data(),bytes.size(),p)==DecodeResult::BadCrc);
 assert(classifySequence(true,0xFFFFFFFEU,1U)==SequenceClass::Newer);
 assert(classifySequence(true,100U,100U)==SequenceClass::Duplicate);
 SessionTracker st; AcceptedCommand accepted{};
 assert(st.consider(7,1,{SpeedLevel::Forward1,SteeringState::Straight},0,true,accepted)==ValidationResult::ConflictingSession);
 assert(st.consider(7,2,{SpeedLevel::Forward1,SteeringState::Straight},50,true,accepted)==ValidationResult::ConflictingSession);
 assert(st.consider(7,3,{SpeedLevel::Forward1,SteeringState::Straight},100,true,accepted)==ValidationResult::Valid);
 Ramp r(250); r.reset(0); for(int i=0;i<200;i++) r.update(250,5); assert(r.value()==250);

 MotorConfig config{}; config.stop_deceleration_per_second=100.0F; config.standstill_velocity=1.0F;
 MotorLogic motors(config);
 ODriveStatus idle{{AxisState::Idle,true,0},{AxisState::Idle,true,0}};
 MotorCommand go{}; go.run=true; go.newly_accepted=true; go.left_velocity=50; go.right_velocity=40;
 motors.update(1,go,idle); assert(motors.state()==MotorState::PushableIdle);
 auto request=motors.update(2,go,idle); assert(motors.state()==MotorState::EnablingAxes);
 assert(request.requested_axis_state==AxisState::ClosedLoop);
 ODriveStatus closed{{AxisState::ClosedLoop,true,0},{AxisState::ClosedLoop,true,0}};
 request=motors.update(3,go,closed); assert(motors.state()==MotorState::Running);
 assert(request.left_velocity==50 && request.right_velocity==40);
 MotorCommand stop{}; stop.newly_accepted=true;
 motors.update(103,stop,closed); assert(motors.state()==MotorState::ControlledStopping);
 request=motors.update(203,stop,closed); assert(request.left_velocity==40 && request.right_velocity==30);
 closed.left.velocity=2; closed.right.velocity=0;
 for(std::uint64_t now=303;now<=703;now+=100) request=motors.update(now,stop,closed);
 assert(motors.state()==MotorState::ControlledStopping); // measured left wheel is not still
 closed.left.velocity=0; motors.update(803,stop,closed);
 assert(motors.state()==MotorState::PushableIdle);

 MotorLogic safety(config);
 stop.newly_accepted=false;
 safety.update(1,stop,idle);
 MotorCommand estop{}; estop.emergency_stop=true;
 request=safety.update(2,estop,idle); assert(safety.state()==MotorState::EmergencyStopped);
 assert(request.requested_axis_state==AxisState::Idle);
 safety.update(3,stop,idle); assert(safety.state()==MotorState::EmergencyStopped);
 stop.newly_accepted=true; safety.update(4,stop,idle); assert(safety.state()==MotorState::Recovering);
 safety.update(5,stop,idle); assert(safety.state()==MotorState::PushableIdle);

 MotorLogic asymmetric(config); asymmetric.update(1,stop,idle); asymmetric.update(2,go,idle);
 ODriveStatus one_axis_bad=closed; one_axis_bad.right.healthy=false;
 request=asymmetric.update(3,go,one_axis_bad);
 assert(asymmetric.state()==MotorState::Fault);
 assert(request.requested_axis_state==AxisState::Idle && request.left_velocity==0 && request.right_velocity==0);
 return 0;
}
