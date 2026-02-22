#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS

#include "network_analyzer.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features
{
	void NetworkAnalyzer::DrawUI() {
	}

	void NetworkAnalyzer::DrawBackgroundUI() {}

	struct PacketInfo {
		uint16_t msgId;
		std::vector<uint8_t> head;
		std::vector<uint8_t> body;
	};

	void DecryptPacket(void* data, uint32_t len) {
		if (!data || len == 0) {
			return;
		}
		// MoleMole.Packet = "private static Boolean ;"
		// 	public static void [A-Z]{11}\(byte[]& buf, int length) { } 
		Unity::Array<uint8_t>* arr =
			(Unity::Array<uint8_t>*)new char[len + 0x20];

		arr->max_length = len;
		memcpy(arr->items, data, len);

		Packet_XorEncrypt(&arr, len);

		memcpy(data, arr->items, len);
		delete[] arr;
	}

	uint16_t ReadBE16(const uint8_t* p) {
		return (p[0] << 8) | p[1];
	}

	uint32_t ReadBE32(const uint8_t* p) {
		return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	}

	bool ParsePacket(uint8_t* enc, uint32_t len, PacketInfo& out) {
		if (!enc || len < 12) {
			return false;
		}

		std::vector<uint8_t> data(enc, enc + len);

		DecryptPacket(data.data(), len);

		uint16_t magicHead = *(uint16_t*)&data[0];
		uint16_t magicEnd = *(uint16_t*)&data[len - 2];

		if (magicHead != 0x6745) {
			return false;
		}

		if (magicEnd != 0xAB89) {
			return false;
		}

		out.msgId = ReadBE16(&data[2]);
		uint16_t headSize = ReadBE16(&data[4]);
		uint32_t bodySize = ReadBE32(&data[6]);

		if (len < headSize + bodySize + 12) {
			return false;
		}

		out.head.assign(data.begin() + 10, data.begin() + 10 + headSize);
		out.body.assign(
			data.begin() + 10 + headSize,
			data.begin() + 10 + headSize + bodySize
		);

		return true;
	}

	struct KcpPacket {
		void* data;
		uint32_t dataLen;
	};

	enum class KcpEventType : int32_t {
		EventNotSet = -1,
		EventConnect = 0,
		EventConnectFailed = 1,
		EventDisconnect = 2,
		EventRecvMsg = 3,
		EventIpChanging = 4,
		EventIpChanged = 5,
		EventPing = 6,
		EventCount = 7
	};

	struct KcpEvent {
		KcpEventType type; // 0x0
		uint32_t token; // 0x4
		uint32_t data; // 0x8
		KcpPacket* packet; // 0x10
	};

	uint32_t ReadVarint32(const uint8_t*& p, const uint8_t* end) {
		uint32_t result = 0;
		int shift = 0;
		while (p < end) {
			uint8_t b = *p++;
			result |= (b & 0x7F) << shift;
			if ((b & 0x80) == 0) break;
			shift += 7;
		}
		return result;
	}

	void RebuildAndEncryptPacket(void* buffer, uint32_t& len, PacketInfo& info) {
		std::vector<uint8_t> newPacket;

		// Magic head (0x4567 big-endian)
		newPacket.push_back(0x45);
		newPacket.push_back(0x67);

		// Message ID (big-endian)
		newPacket.push_back((info.msgId >> 8) & 0xFF);
		newPacket.push_back(info.msgId & 0xFF);

		// Head size (big-endian)
		uint16_t headSize = info.head.size();
		newPacket.push_back((headSize >> 8) & 0xFF);
		newPacket.push_back(headSize & 0xFF);

		// Body size (big-endian)
		uint32_t bodySize = info.body.size();
		newPacket.push_back((bodySize >> 24) & 0xFF);
		newPacket.push_back((bodySize >> 16) & 0xFF);
		newPacket.push_back((bodySize >> 8) & 0xFF);
		newPacket.push_back(bodySize & 0xFF);

		// Head data
		newPacket.insert(newPacket.end(), info.head.begin(), info.head.end());

		// Body data (MODIFIED)
		newPacket.insert(newPacket.end(), info.body.begin(), info.body.end());

		// Magic end (0x89AB big-endian)
		newPacket.push_back(0x89);
		newPacket.push_back(0xAB);

		Log("Packet size: %u -> %zu\n", len, newPacket.size());

		// Шифруем обратно (XOR симметричен)
		DecryptPacket(newPacket.data(), newPacket.size());

		// Копируем в buffer
		memcpy(buffer, newPacket.data(), newPacket.size());
		len = newPacket.size();
	}

	void DumpProtobufStructure(const std::vector<uint8_t>& data, int indent = 0) {
		if (indent > 10) return; // max recursion depth guard

		const uint8_t* p = data.data();
		const uint8_t* end = p + data.size();

		while (p < end) {
			if (p >= end) break;

			uint32_t tag = ReadVarint32(p, end);
			if (p > end) break; // ReadVarint32 can overshoot on malformed data

			uint32_t fieldNumber = tag >> 3;
			uint32_t wireType = tag & 0x7;

			if (fieldNumber == 0) break; // invalid field number, stop

			for (int i = 0; i < indent; i++) Log("  ");
			Log("Field %u (wire=%u): ", fieldNumber, wireType);

			if (wireType == 0) {
				uint32_t value = ReadVarint32(p, end);
				Log("varint=%u\n", value);
			}
			else if (wireType == 2) {
				uint32_t length = ReadVarint32(p, end);
				Log("length=%u\n", length);

				// Bounds check before recursing
				if (length > (uint32_t)(end - p)) {
					Log("  [INVALID: length exceeds remaining data]\n");
					break;
				}

				std::vector<uint8_t> sub(p, p + length);
				DumpProtobufStructure(sub, indent + 1);
				p += length;
			}
			else if (wireType == 1) { // 64-bit fixed
				if (end - p < 8) break;
				Log("64bit\n");
				p += 8;
			}
			else if (wireType == 5) { // 32-bit fixed
				if (end - p < 4) break;
				Log("32bit\n");
				p += 4;
			}
			else {
				Log("unknown wire type, stopping\n");
				break; // Unknown wire type — stop to avoid garbage parsing
			}
		}
	}

	void WriteVarint(std::vector<uint8_t>& buf, uint32_t value) {
		while (value >= 0x80) {
			buf.push_back((value & 0x7F) | 0x80);
			value >>= 7;
		}
		buf.push_back(value & 0x7F);
	}

	int32_t(*KcpNative_kcp_client_send_packet)(void* kcp, KcpPacket* packet);
	int32_t hKcpNative_kcp_client_send_packet(void* kcp, KcpPacket* packet) {
		if (!packet || !packet->data || packet->dataLen == 0) {
			return KcpNative_kcp_client_send_packet(kcp, packet);
		}

		PacketInfo info;
		if (ParsePacket((uint8_t*)packet->data, packet->dataLen, info)) {
			std::vector<uint16_t> skip{
				//27610, 20832, 25068, 2323
			};
			if (std::find(skip.begin(), skip.end(), info.msgId) == skip.end()) {
				Log(
					"[KCP][SEND][OK] msgId=%u head=%zu body=%zu\n",
					info.msgId,
					info.head.size(),
					info.body.size()
				);
				if (info.msgId == 20832) {
					DumpProtobufStructure(info.body);
				}
			}
		}
		else {
			Log("[KCP][SEND][SKIP] Failed to parse packet\n");
		}
		int32_t result = KcpNative_kcp_client_send_packet(kcp, packet);
		//Log("[KCP][SEND] result=%d\n", result);
		return result;
	}

	// Улучшенный парсер protobuf с выводом в удобочитаемом формате
	struct ProtoField {
		uint32_t number;
		uint32_t wireType;
		size_t offset;
		size_t dataOffset; // Где начинаются данные (после tag и length)

		// Для разных типов
		uint64_t varint = 0;
		uint32_t length = 0;
		std::vector<uint8_t> bytes;

		std::vector<ProtoField> children; // Для вложенных сообщений
	};

	uint64_t ReadVarint64(const uint8_t*& p, const uint8_t* end) {
		if (p >= end) return 0;

		uint64_t result = 0;
		int shift = 0;
		while (p < end && shift < 64) {
			uint8_t b = *p++;
			result |= (uint64_t)(b & 0x7F) << shift;
			if ((b & 0x80) == 0) break;
			shift += 7;
		}
		return result;
	}

	ProtoField ParseProtoField(const uint8_t*& p, const uint8_t* end, size_t baseOffset = 0) {
		ProtoField field = {};
		field.offset = baseOffset + (p - (end - (end - p)));

		if (p >= end) return field;

		uint32_t tag = ReadVarint64(p, end);
		field.number = tag >> 3;
		field.wireType = tag & 0x7;
		field.dataOffset = baseOffset + (p - (end - (end - p)));

		switch (field.wireType) {
		case 0: // Varint
			field.varint = ReadVarint64(p, end);
			break;

		case 1: // 64-bit
			if (end - p >= 8) {
				field.varint = *(uint64_t*)p;
				p += 8;
			}
			break;

		case 2:
		{
			uint64_t len64 = ReadVarint64(p, end);

			if (len64 > (uint64_t)(end - p)) {
				// Некорректная длина прекращаем парсинг
				p = end;
				break;
			}

			field.length = (uint32_t)len64;

			field.bytes.assign(p, p + field.length);
			p += field.length;

			break;
		}
		case 5: // 32-bit
			if (end - p >= 4) {
				field.varint = *(uint32_t*)p;
				p += 4;
			}
			break;
		}

		return field;
	}

	std::vector<ProtoField> ParseProtoMessage(const std::vector<uint8_t>& data, int maxFields = 2000) {
		std::vector<ProtoField> fields;

		const uint8_t* p = data.data();
		const uint8_t* end = p + data.size();

		int count = 0;
		while (p < end && count < maxFields) {
			const uint8_t* before = p;

			ProtoField field = ParseProtoField(p, end, 0);

			if (field.number == 0 || p <= before)
				break;

			fields.push_back(field);
			count++;
		}

		return fields;
	}

	int32_t(*oKcp_client_try_deque_event)(void* kcp_client, KcpEvent* evt);
	int32_t hkKcp_client_try_deque_event(void* kcp_client, KcpEvent* evt) {
		if (evt && evt->packet && evt->type == KcpEventType::EventRecvMsg) {
			PacketInfo info;
			if (ParsePacket((uint8_t*)evt->packet->data, evt->packet->dataLen, info)) {

				if (info.msgId == 6019) {
					Log("\n==== [RECV] AvatarChangeCostumeRsp ====\n");

					// Парсим текущий ответ
					auto fields = ParseProtoMessage(info.body);

					uint32_t retcode = 0;
					uint64_t avatarGuid = 0;
					uint32_t costumeId = 0;

					for (const auto& field : fields) {
						if (field.wireType == 0) {
							if (field.number == 12) retcode = field.varint;
							if (field.number == 14) avatarGuid = field.varint;
							if (field.number == 15) costumeId = field.varint;
						}
					}

					Log("Original response: retcode=%u, avatarGuid=%llu, costumeId=%u\n",
						retcode, avatarGuid, costumeId);

					if (retcode != 0) {
						Log("Server rejected costume change! Fixing response...\n");
					}
					else {
						Log("Server accepted costume change (retcode=0)\n");
					}
				}
			}
		}
		int32_t result = oKcp_client_try_deque_event(kcp_client, evt);

		return result;
	}


	void NetworkAnalyzer::OnInit() {
		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0x1859BA0),
			hkKcp_client_try_deque_event,
			(LPVOID*)&oKcp_client_try_deque_event
		);*/


		MH_CreateHook((LPVOID)(Mem::Signature("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 48 85 D2").Scan()), (LPVOID)hKcpNative_kcp_client_send_packet, (LPVOID*)&KcpNative_kcp_client_send_packet);
	}

	void NetworkAnalyzer::OnUpdate() {
	}
}