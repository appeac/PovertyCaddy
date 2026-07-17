#include "poor_caddy_protocol/protocol.hpp"
#include <cstring>
namespace poor_caddy {
static void put32(PacketBytes& b, std::size_t o, std::uint32_t v){ b[o]=v&0xffU; b[o+1]=(v>>8)&0xffU; b[o+2]=(v>>16)&0xffU; b[o+3]=(v>>24)&0xffU; }
static std::uint32_t get32(const std::uint8_t* b, std::size_t o){ return static_cast<std::uint32_t>(b[o]) | (static_cast<std::uint32_t>(b[o+1])<<8) | (static_cast<std::uint32_t>(b[o+2])<<16) | (static_cast<std::uint32_t>(b[o+3])<<24); }
PacketBytes encodeControlPacket(std::uint32_t sid, std::uint32_t seq, ControlState s, std::uint8_t flags){ PacketBytes b{}; put32(b,0,kProtocolMagic); put32(b,4,sid); put32(b,8,seq); b[12]=kProtocolVersion; b[13]=static_cast<std::uint8_t>(s.speed); b[14]=static_cast<std::uint8_t>(s.steering); b[15]=flags; put32(b,16,0); put32(b,20,crc32(b.data(),20)); return b; }
DecodeResult decodeControlPacket(const std::uint8_t* d, std::size_t len, ControlPacket& out){ if(d==nullptr || len!=kControlPacketSize) return DecodeResult::WrongLength; out.magic=get32(d,0); if(out.magic!=kProtocolMagic) return DecodeResult::WrongMagic; out.session_id=get32(d,4); out.sequence=get32(d,8); out.protocol_version=d[12]; if(out.protocol_version!=kProtocolVersion) return DecodeResult::WrongVersion; out.desired_speed=d[13]; out.desired_steering=d[14]; out.flags=d[15]; out.crc32=get32(d,20); if(crc32(d,20)!=out.crc32) return DecodeResult::BadCrc; if(!isValidSpeed(out.desired_speed)) return DecodeResult::InvalidSpeed; if(!isValidSteering(out.desired_steering)) return DecodeResult::InvalidSteering; return DecodeResult::Valid; }
}
