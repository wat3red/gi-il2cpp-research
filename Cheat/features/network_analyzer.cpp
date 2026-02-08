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
	bool reload = false;
	void NetworkAnalyzer::DrawUI() {
		if (ImGui::Button("reload")) {
			reload = !reload;
		}
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
		const uint8_t* p = data.data();
		const uint8_t* end = p + data.size();

		while (p < end) {
			uint32_t tag = ReadVarint32(p, end);
			uint32_t fieldNumber = tag >> 3;
			uint32_t wireType = tag & 0x7;

			for (int i = 0; i < indent; i++) Log("  ");
			Log("Field %u (wire=%u): ", fieldNumber, wireType);

			if (wireType == 0) {
				uint32_t value = ReadVarint32(p, end);
				Log("varint=%u\n", value);
			}
			else if (wireType == 2) {
				uint32_t length = ReadVarint32(p, end);
				Log("length=%u\n", length);
				// Рекурсивно дампим вложенные сообщения
				std::vector<uint8_t> sub(p, p + length);
				DumpProtobufStructure(sub, indent + 1);
				p += length;
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
			if (info.msgId == 8017) {
				Log(
					"[KCP][SEND][OK] AvatarChangeCostumeReq, msgId=%u head=%zu body=%zu\n",
					info.msgId,
					info.head.size(),
					info.body.size()
				);
				DumpProtobufStructure(info.body);

				return 0;
			}
		}
		else {
			Log("[KCP][SEND][SKIP] Failed to parse packet\n");
		}
		int32_t result = KcpNative_kcp_client_send_packet(kcp, packet);
		Log("[KCP][SEND] result=%d\n", result);
		return result;
	}

	template<typename T>
	struct RepeatedMessageField : Il2CppObject {
		Unity::List<T>* values;
		bool isInPool;
		int32_t count;
	};

	template<typename T>
	struct RepeatedPrimitiveField : Il2CppObject {
		Unity::Array<T>* array;
		int32_t count;
	};

	struct AvatarInfo {
		char _pad0[0x18];

		void* some_pointer1;   // 0x18
		void* some_pointer2;   // 0x20
		void* some_pointer3;   // 0x28
		void* some_pointer4;   // 0x30
		void* some_pointer5;   // 0x38
		void* some_pointer6;   // 0x40
		void* some_pointer7;   // 0x48
		void* some_pointer8;   // 0x50
		void* some_pointer9;   // 0x58
		void* some_pointer10;  // 0x60
		void* some_pointer11;  // 0x68
		void* some_pointer12;  // 0x70
		void* some_pointer13;  // 0x78
		void* some_pointer14;  // 0x80
		void* some_pointer15;  // 0x88
		void* some_pointer16;  // 0x90
		void* some_pointer17;  // 0x98
		void* some_pointer18;  // 0xA0

		uint32_t some_uint1;   // 0xA8
		uint32_t some_uint2;   // 0xAC
		uint32_t some_uint3;   // 0xB0
		uint32_t some_uint4;   // 0xB4
		uint32_t some_uint5;   // 0xB8
		uint32_t some_uint6;   // 0xBC

		uint64_t some_uint64_1; // 0xC0

		uint32_t some_uint7;   // 0xC8
		bool     some_bool1;   // 0xCC

		uint32_t config_id;   // 0xD0
		uint32_t some_uint9;   // 0xD4
		uint32_t costume_id;  // 0xD8
		int32_t  some_int1;    // 0xDC
		uint32_t some_uint11;  // 0xE0
		uint32_t some_uint12;  // 0xE4
		uint32_t some_uint13;  // 0xE8
		uint32_t some_uint14;  // 0xEC
	};

	void LogAvatarInfo(AvatarInfo* v) {
		if (!v) {
			Log("AvatarInfo = NULL\n");
			return;
		}

		/*Log("  some_pointer1   = %p\n", v->some_pointer1);
		Log("  some_pointer2   = %p\n", v->some_pointer2);
		Log("  some_pointer3   = %p\n", v->some_pointer3);
		Log("  some_pointer4   = %p\n", v->some_pointer4);
		Log("  some_pointer5   = %p\n", v->some_pointer5);
		Log("  some_pointer6   = %p\n", v->some_pointer6);
		Log("  some_pointer7   = %p\n", v->some_pointer7);
		Log("  some_pointer8   = %p\n", v->some_pointer8);
		Log("  some_pointer9   = %p\n", v->some_pointer9);
		Log("  some_pointer10  = %p\n", v->some_pointer10);
		Log("  some_pointer11  = %p\n", v->some_pointer11);
		Log("  some_pointer12  = %p\n", v->some_pointer12);
		Log("  some_pointer13  = %p\n", v->some_pointer13);
		Log("  some_pointer14  = %p\n", v->some_pointer14);
		Log("  some_pointer15  = %p\n", v->some_pointer15);
		Log("  some_pointer16  = %p\n", v->some_pointer16);
		Log("  some_pointer17  = %p\n", v->some_pointer17);
		Log("  some_pointer18  = %p\n", v->some_pointer18);*/

		Log("  some_uint1      = %u\n", v->some_uint1);
		Log("  some_uint2      = %u\n", v->some_uint2);
		Log("  some_uint3      = %u\n", v->some_uint3);
		Log("  some_uint4      = %u\n", v->some_uint4);
		Log("  some_uint5      = %u\n", v->some_uint5);
		Log("  some_uint6      = %u\n", v->some_uint6);

		Log("  some_uint64_1   = %llu\n", (unsigned long long)v->some_uint64_1);

		Log("  some_uint7      = %u\n", v->some_uint7);
		Log("  some_bool1      = %d\n", v->some_bool1 ? 1 : 0);

		Log("  configId        = %u\n", v->config_id);

		Log("  some_uint9      = %u\n", v->some_uint9);
		Log("  costume_id     = %u\n", v->costume_id);
		Log("  some_int1       = %d\n", v->some_int1);
		Log("  some_uint11     = %u\n", v->some_uint11);
		Log("  some_uint12     = %u\n", v->some_uint12);
		Log("  some_uint13     = %u\n", v->some_uint13);
		Log("  some_uint14     = %u\n", v->some_uint14);
	}

	struct AvatarDataNotify {
		char _[0x18];
		RepeatedPrimitiveField<uint64_t>* some_list1; // 0x18
		RepeatedPrimitiveField<uint32_t>* ownedCostumeList_; // 0x20
		void* NBCKLBCDFDH; // 0x28
		RepeatedMessageField<AvatarInfo*>* avatarList_; // 0x30
		void* NOGFCBAFIAK; // 0x38
		RepeatedPrimitiveField<uint32_t>* some_list3; // 0x40
		RepeatedPrimitiveField<uint32_t>* some_list4; // 0x48
		RepeatedPrimitiveField<uint32_t>* some_list5; // 0x50
		uint64_t chooseAvatarGuid_; // 0x58
		uint32_t curAvatarTeamId_; // 0x60
	};

	struct SimpleSafeUInt32 {
		uint32_t value;

		uint32_t get_Value() {
			return (((uint32_t(*)(SimpleSafeUInt32*))(g_game_base + 0x8EC70))(this));
		}
	};

	struct SimpleSafeFloat {
		uint32_t value;

		float get_Value() {
			return (((float(*)(SimpleSafeFloat*))(g_game_base + 0xBA940))(this));
		}
	};

	struct AvatarCostumeExcelConfig : Il2CppObject {
		Il2CppString* some_string1; // 0x10
		Il2CppString* json_name; // 0x18
		Il2CppString* some_string3; // 0x20
		uint64_t some_ulong0; // 0x28
		uint32_t some_uint1; // 0x30
		uint32_t some_uint2; // 0x34
		uint32_t some_uint3; // 0x38
		uint64_t some_ulong1; // 0x40
		uint64_t some_ulong2; // 0x48
		uint64_t some_ulong3; // 0x50
		uint64_t some_ulong4; // 0x58
		uint64_t some_ulong5; // 0x60
		uint64_t some_ulong6; // 0x68
		SimpleSafeUInt32 some_safeuint1; // 0x70
		SimpleSafeUInt32 some_safeuint2; // 0x74
		uint64_t some_ulong7; // 0x78
		uint64_t some_ulong8; // 0x80
		uint64_t some_ulong9; // 0x88
		SimpleSafeUInt32 some_safeuint3; // 0x90
		SimpleSafeUInt32 some_safeuint4; // 0x94
		SimpleSafeUInt32 some_safeuint5; // 0x98
		SimpleSafeUInt32 some_safeuint6; // 0x9C
		uint64_t some_ulong10; // 0xA0
		bool some_bool1; // 0xA8
		bool some_bool2; // 0xA9
		bool some_bool3; // 0xAA
		bool some_bool4; // 0xAB
		bool some_bool5; // 0xAC
	};

	// Namespace: 
	struct AvatarExcelConfig {
		char _[0xC0];
		Unity::Array<int32_t>* ACFKHKHMKJL; // 0xC0
		Il2CppString* idk_empty; // 0xC8
		Unity::Array<SimpleSafeUInt32>* EPFADMHLAJP; // 0xD0
		Unity::Array<SimpleSafeUInt32>* OBOABMJFAEG; // 0xD8
		Il2CppString* _imageName; // 0xE0
		Il2CppString* _iconName; // 0xE8
		Unity::Array<SimpleSafeUInt32>* AJHBEFMJMFA; // 0xF0
		Il2CppString* _sideIconName; // 0xF8
		int32_t NDHGDFNPOMA; // 0x100
		SimpleSafeUInt32 LPJBOJIDOGM; // 0x104
		SimpleSafeUInt32 IGIGEFEHEGB; // 0x108
		SimpleSafeFloat FFEEGCJCBGB; // 0x10C
		uint64_t MNLCGMLCEOJ; // 0x110
		uint64_t LELIHKNAOLI; // 0x118
		uint64_t BKCLJKHLOHI; // 0x120
		uint64_t AEJAOKJELGB; // 0x128
		SimpleSafeUInt32 HAGKGEEAAIK; // 0x130
		int32_t DJHBFCEFEMH; // 0x134
		uint64_t NAMLGBCJDPN; // 0x138
		uint32_t MFKBHKOOCKE; // 0x140
		SimpleSafeUInt32 GFHKLHKMCPJ; // 0x144
		SimpleSafeFloat JCDGLGIEIOP; // 0x148
		int32_t NEDENPLBPOG; // 0x14C
		uint64_t OBGGFJKAOJC; // 0x150
		uint64_t BCKLNKFFIBN; // 0x158
		uint64_t KLONFAHGEOE; // 0x160
		SimpleSafeUInt32 OOMNOELEHIL; // 0x168
		SimpleSafeUInt32 OLHONFHJPJN; // 0x16C
		uint64_t NAKPAEBDJAC; // 0x170
		uint64_t FCONGEPGDOI; // 0x178
		uint64_t KKFPOIBHPEK; // 0x180
		uint64_t FDLBGDHIIOP; // 0x188
		uint64_t OHNKPJFCKIC; // 0x190
		uint64_t KBLLPKCJBCA; // 0x198
		int32_t LOOECLFAHPE; // 0x1A0
		int32_t GBGOEFNFDLJ; // 0x1A4
		uint64_t BMJJNGBPOCH; // 0x1A8
		uint64_t NIDCKFMJHLB; // 0x1B0
		uint64_t LNMIDNHMJFC; // 0x1B8
		int32_t CKANNPNKIPF; // 0x1C0
		bool BALJJCELJFD; // 0x1C4
		bool INIIGAGFDMG; // 0x1C5
		bool GAPNFMHMEJI; // 0x1C6
		SimpleSafeFloat MGPMHIOHGMK; // 0x1C8
		int32_t LMAACLELEMN; // 0x1CC
		SimpleSafeUInt32 NMLPBBNLLFA; // 0x1D0
		SimpleSafeFloat CBCFOLHOMDM; // 0x1D4
		int32_t POBGBOLAJFP; // 0x1D8
		uint32_t EEPHCBGPCAA; // 0x1DC
	};

	// Proto.AvatarChangeCostumeRsp : CJALBADBHKA
	// Proto.AvatarDataNotify : MBDKEBFNKOE
	// OnAvatarDataNotify : IKMOJIKGBFB
	// MoleMole.Config.AvatarExcelConfig : DMCPPFHDBHK
	// SimpleSafeUInt32 : JNBFFKMNBLO
	void(*PlayerModule_OnAvatarDataNotify)(void* playerModule, AvatarDataNotify* notify);
	void hPlayerModule_OnAvatarDataNotify(void* _this, AvatarDataNotify* notify) {
		Log("[AvatarNotify] _this=%p notify=%p\n", _this, notify);
		Log("chooseAvatarGuid_=%lu, curAvatarTeamId_=%u\n", notify->chooseAvatarGuid_, notify->curAvatarTeamId_);


		/*Log("ownedCostumeList_ before=%d\n", notify->ownedCostumeList_->count);
		for (size_t i = 0; i < notify->ownedCostumeList_->count; i++) {
			uint32_t value = notify->ownedCostumeList_->array->items[i];
			Log("[%zu] val=%d\n", i, value);
		}

		int oldCount = notify->ownedCostumeList_->count;

		auto newArr = (Unity::Array<uint32_t>*)
			il2cpp_array_new(Il2Cpp::Class::FromName("System", "UInt32"), oldCount + 1);

		memcpy(newArr->items,
			notify->ownedCostumeList_->array->items,
			sizeof(uint32_t) * oldCount);

		newArr->items[oldCount] = 200501;

		notify->ownedCostumeList_->array = newArr;
		notify->ownedCostumeList_->count = oldCount + 1;

		Log("avatarList_=%d\n", notify->avatarList_->count);
		Log("avatarList_->values=%p\n", notify->avatarList_->values);
		Log("avatarList_->values size=%d\n", notify->avatarList_->values->size);*/

		((void(*)())(g_game_base + 0x8B61E50))(); // prepare
		auto costumes = (((Unity::Dictionary<uint32_t, AvatarCostumeExcelConfig*> *(*)())(g_game_base + 0x8B61E40))());
		if (costumes) {
			auto costumes_vector = costumes->to_vector();
			Log("Available costumes (%zu total):\n", costumes_vector.size());
			for (size_t i = 0; i < costumes_vector.size(); i++) {
				Log("  Costume ID: %u - %s\n",
					costumes_vector[i].first,
					costumes_vector[i].second->json_name->ToCStr());
			}
		}

		((void(*)())(g_game_base + 0xD4CD3A0))(); // prepare
		auto avatars = (((Unity::Dictionary<uint32_t, AvatarExcelConfig*> *(*)())(g_game_base + 0xD4CD380))());
		if (avatars) {
			auto avatars_vector = avatars->to_vector();
			Log("Available avatars (%zu total):\n", avatars_vector.size());
			for (size_t i = 0; i < avatars_vector.size(); i++) {
				Log("  Avatar ID: %u - '%s'\n",
					avatars_vector[i].first,
					((Il2CppString * (*)(AvatarExcelConfig*))(g_game_base + 0xEC558E0))(avatars_vector[i].second)->ToCStr());
			}
		}

		for (size_t i = 0; i < notify->avatarList_->values->size; i++) {
			AvatarInfo* value = notify->avatarList_->values->array->items[i];

			Log("[%zu] val=%p\n", i, value);
			LogAvatarInfo(value);

			if (value->config_id == 10000015) {
				Log("----------------- FOUND Kaeya\n");
				notify->chooseAvatarGuid_ = value->some_uint64_1;
			}

			if (value->config_id == 10000005) {
				Log("----------------- FOUND\n");
				value->costume_id = 200501;
			}
		}

		PlayerModule_OnAvatarDataNotify(_this, notify);
	}

	std::vector<uint8_t> ModifyCostumeResponse(const std::vector<uint8_t>& body) {
		const uint8_t* p = body.data();
		const uint8_t* end = p + body.size();

		std::vector<uint8_t> result;

		while (p < end) {
			const uint8_t* tagStart = p;
			uint32_t tag = ReadVarint32(p, end);
			uint32_t fieldNumber = tag >> 3;
			uint32_t wireType = tag & 0x7;

			if (fieldNumber == 12 && wireType == 0) {
				// Нашли поле retcode!
				uint32_t oldRetcode = ReadVarint32(p, end);

				Log("  Modifying retcode: %u -> 0\n", oldRetcode);

				// Записываем новый tag и значение 0
				WriteVarint(result, (12 << 3) | 0);
				WriteVarint(result, 0); // SUCCESS
			}
			else {
				// Копируем поле как есть
				result.insert(result.end(), tagStart, p);

				if (wireType == 0) {
					const uint8_t* valStart = p;
					ReadVarint32(p, end);
					result.insert(result.end(), valStart, p);
				}
				else if (wireType == 2) {
					const uint8_t* lenStart = p;
					uint32_t len = ReadVarint32(p, end);
					result.insert(result.end(), lenStart, p);
					if (p + len <= end) {
						result.insert(result.end(), p, p + len);
						p += len;
					}
				}
			}
		}

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
		{ // Length-delimited
			field.length = ReadVarint64(p, end);
			if (p + field.length <= end) {
				field.bytes.assign(p, p + field.length);
				p += field.length;
			}
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
			ProtoField field = ParseProtoField(p, end, 0);
			if (field.number == 0) break;
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

				// Перехватываем AvatarChangeCostumeRsp
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

						info.body = ModifyCostumeResponse(info.body);
						RebuildAndEncryptPacket(evt->packet->data, evt->packet->dataLen, info);

						Log("Response fixed! Costume change should work now.\n");
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

	/*void (*AppearancePageContext_OnCostumeSelect)(void* _this, int32_t index, bool isSetup, bool isJoypadInit);
	void hAppearancePageContext_OnCostumeSelect(void* _this, int32_t index, bool isSetup, bool isJoypadInit) {
		Log("hAppearancePageContext_OnCostumeSelect\n");
		AppearancePageContext_OnCostumeSelect(_this, index, isSetup, isJoypadInit);
	}*/

	bool (*CanChangeAvatarEntity)(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision);
	bool hCanChangeAvatarEntity(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision) {
		Log("hCanChangeAvatarEntity, name=%s, configId=%u\n", entity->GetName()->ToCStr(), configId);
		//return CanChangeAvatarEntity(entity, configId, failType, ignoreStateLayer, isSpecialChange, ignoreCollision);
		return true;
	}

	struct AvatarChangeCostumeRsp {
		char _[0x18];
		uint32_t costume_id;
		int32_t ret_code;
		uint64_t some_ulong;
	};

	void (*OnAvatarChangeCostumeRsp)(void* _this, AvatarChangeCostumeRsp* rsp);
	void hOnAvatarChangeCostumeRsp(void* _this, AvatarChangeCostumeRsp* rsp) {
		Log("hOnAvatarChangeCostumeRsp, costume_id=%u, ret_code=%d, some_ulong=%lu\n", rsp->costume_id, rsp->ret_code, rsp->some_ulong);
		//rsp->ret_code = 0;
		OnAvatarChangeCostumeRsp(_this, rsp);
	}

	void NetworkAnalyzer::OnInit() {
		MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7CA9D0),
			hPlayerModule_OnAvatarDataNotify,
			(LPVOID*)&PlayerModule_OnAvatarDataNotify
		);

		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0x1859BA0),
			hkKcp_client_try_deque_event,
			(LPVOID*)&oKcp_client_try_deque_event
		);*/

		MH_CreateHook(
			(LPVOID)(g_game_base + 0xFD7E180),
			hCanChangeAvatarEntity,
			(LPVOID*)&CanChangeAvatarEntity
		);

		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7C4200),
			hOnAvatarChangeCostumeRsp,
			(LPVOID*)&OnAvatarChangeCostumeRsp
		);*/

		// private void [A-Z]{11}\(int32 [A-Z]{11}, boolean [A-Z]{11}, boolean [A-Z]{11}\)
		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0x108F7440),
			hAppearancePageContext_OnCostumeSelect,
			(LPVOID*)&AppearancePageContext_OnCostumeSelect
		);*/

		//MH_CreateHook((LPVOID)(Mem::Signature("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 48 85 D2").Scan()), (LPVOID)hKcpNative_kcp_client_send_packet, (LPVOID*)&KcpNative_kcp_client_send_packet);
	}

	void NetworkAnalyzer::OnUpdate() {
		if (reload) {
			MoleMole::PlayerModule* player_module = MoleMole::PlayerModule::Instance();
			((void(*)(MoleMole::PlayerModule*, int32_t))(g_game_base + 0xE7B9290))(player_module, 1);
			reload = false;
		}
	}
}