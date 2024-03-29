#include <stdio.h>
#include "Skin.h"
#include "metamod_oslink.h"
#include "utils.hpp"
#include <utlstring.h>
#include <KeyValues.h>
#include "sdk/schemasystem.h"
#include "sdk/CBaseEntity.h"
#include "sdk/CGameRulesProxy.h"
#include "sdk/CBasePlayerPawn.h"
#include "sdk/CCSPlayerController.h"
#include "sdk/CCSPlayer_ItemServices.h"
#include "sdk/CPlayer_ItemServices.cpp"
#include "sdk/CSmokeGrenadeProjectile.h"
#include <map>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>
#else
#include "utils/module.h"
#endif
#include <string>
#include "utils/ctimer.h"



Skin g_Skin;
PLUGIN_EXPOSE(Skin, g_Skin);
IVEngineServer2* engine = nullptr;
IGameEventManager2* gameeventmanager = nullptr;
IGameResourceServiceServer* g_pGameResourceService = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;
CSchemaSystem* g_pCSchemaSystem = nullptr;
CCSGameRules* g_pGameRules = nullptr;
CPlayerSpawnEvent g_PlayerSpawnEvent;
CRoundPreStartEvent g_RoundPreStartEvent;

//TEST//////
#include <stdexcept>
#include <testUtils/json.hpp>
#include <curl/curl.h>

#include <curl/easy.h>

#include <curl/mprintf.h>
#include <curl/multi.h>

#include <curl/stdcheaders.h>
#include <curl/system.h>

#include <curl/urlapi.h>

#include <memory>

#include <thread>

#include <regex>
#include <cstdint>
//#include <cstring>

#include <mutex>



#include "sdk/CBsePlayer.h"
//#include "baseviewmodel_shared.h"
//#include "c_baseplayer.h"
Event_ItemPurchase g_PlayerBuy;
Event_PlayerSpawned g_PlayerSpawnedEvent;//tested
OnRoundStart g_RoundStart;

Event_PlayerConnect g_PlayerConnect;
Event_PlayerDisconnect g_PlayerDisconnect;

void TestSkinchanger(int64_t arg1, int arg2);
void SkinChangerKnife(int64_t arg1);
std::map<int, std::vector<nlohmann::json>> GETSKINS(int64_t steamid64);
void AddOrUpdatePlayer(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, std::map<int, std::vector<nlohmann::json>> skins, SC_CBaseEntity* pbe);
void ClearPlayer(int64_t steamid);

void ForceUpdate(int64_t steamid);

void ThreadUpdate(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, SC_CBaseEntity* pbe);

CCSPlayerController* PC;
CCSPlayerPawnBase* PP;

struct Players
{
    CCSPlayerController* PC;
    CCSPlayerPawnBase* PP;
    SC_CBaseEntity* PBE;
    //nlohmann::json SKINS;
    std::map<int, std::vector<nlohmann::json>> PlayerSkins;
    bool firstspawn=true;
    std::map<int,CEntityInstance*> PlayerWeapons;
};

//std::map<int64_t, std::shared_ptr<Players>> players;//TEST DYNAMIC MASSIVE
std::map<int64_t, Players> players;//TEST DYNAMIC MASSIVE
std::map<int64_t, bool> state;//TEST DYNAMIC MASSIVE

std::mutex playersMutex;

std::map<std::string, int> SearchMap;
//TEST//////

CEntityListener g_EntityListener;
bool g_bPistolRound;

float g_flUniversalTime;
float g_flLastTickedTime;
bool g_bHasTicked;

#define CHAT_PREFIX	""
#define DEBUG_OUTPUT 1
#define FEATURE_STICKERS 0

typedef struct SkinParm
{
	int32_t m_iItemDefinitionIndex;
	int m_nFallbackPaintKit;
	int m_nFallbackSeed;
	float m_flFallbackWear;
	std::string m_nameTag;
	bool used = false;
}SkinParm;


typedef struct StickerParm
{
	int stickerDefIndex1;
	float stickerWear1;
	int stickerDefIndex2;
	float stickerWear2;
	int stickerDefIndex3;
	float stickerWear3;
	int stickerDefIndex4;
	float stickerWear4;
}StickerParm;


#ifdef _WIN32
typedef void*(FASTCALL* StateChanged_t)(void *networkTransmitComponent, CEntityInstance *ent, int64 offset, int16 a4, int16 a5);
typedef void*(FASTCALL* SubClassChange_t)(const CCommandContext &context, const CCommand &args);
typedef void*(FASTCALL* EntityRemove_t)(CGameEntitySystem*, void*, void*,uint64_t);
typedef void(FASTCALL* GiveNamedItem_t)(void* itemService,const char* pchName, void* iSubType,void* pScriptItem, void* a5,void* a6);
typedef void(FASTCALL* UTIL_ClientPrintAll_t)(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4);
typedef void(FASTCALL *ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);

//TEST
//using fGetNextSceneEventIDOffset = int64_t (__fastcall*)(void*, void*, int, bool);
typedef int64_t (FASTCALL* GetNextSceneEventIDOffset_t)(float* ent, long* magicNrPtr, long magicNr, bool flag);



extern SubClassChange_t FnSubClassChange;
extern EntityRemove_t FnEntityRemove;
extern GiveNamedItem_t FnGiveNamedItem;
extern UTIL_ClientPrintAll_t FnUTIL_ClientPrintAll;
extern ClientPrint_t FnUTIL_ClientPrint;

//TEST
extern GetNextSceneEventIDOffset_t GetNextSceneEventIDOffset;



EntityRemove_t FnEntityRemove;
GiveNamedItem_t FnGiveNamedItem;
UTIL_ClientPrintAll_t FnUTIL_ClientPrintAll;
ClientPrint_t FnUTIL_ClientPrint;
SubClassChange_t FnSubClassChange;
StateChanged_t FnStateChanged;

//TEST
GetNextSceneEventIDOffset_t GetNextSceneEventIDOffset;

#else
void (*FnEntityRemove)(CGameEntitySystem*, void*, void*,uint64_t) = nullptr;
void (*FnGiveNamedItem)(void* itemService,const char* pchName, void* iSubType,void* pScriptItem, void* a5,void* a6) = nullptr;
void (*FnUTIL_ClientPrintAll)(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4) = nullptr;
void (*FnUTIL_ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
void (*FnSubClassChange)(const CCommandContext &context, const CCommand &args) = nullptr;
void (*FnStateChanged)(void *networkTransmitComponent, CEntityInstance *ent, int offset, int16_t a4, int16_t a5) = nullptr;

//TEST
//
int64_t (*GetNextSceneEventIDOffset)(float* ent, long* magicNrPtr, long magicNr, bool flag)=nullptr;
#endif

std::map<int, std::string> g_WeaponsMap;
std::map<int, std::string> g_KnivesMap;
std::map<int, int> g_ItemToSlotMap;
std::map<uint64_t, SkinParm> g_PlayerSkins;

std::map<uint64_t, StickerParm> g_PlayerStickers;

std::map<uint64_t, int> g_PlayerMessages;
uint32_t g_iItemIDHigh = 16384;


//std::map<int,std::string> g_GlovesMap;
//bool zxczxc=false;

class GameSessionConfiguration_t { };
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();
	if(!server) {
		return nullptr;
	}
	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

#ifdef _WIN32
inline void* FindSignature(const char* modname,const char* sig)
{
	DWORD64 hModule = (DWORD64)GetModuleHandle(modname);
	if (!hModule)
	{
		return NULL;
	}
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	MODULEENTRY32 mod = {sizeof(MODULEENTRY32)};
	
	while (Module32Next(hSnap, &mod))
	{
		if (!strcmp(modname, mod.szModule))
		{
			if(!strstr(mod.szExePath,"metamod"))
				break;
		}
	}
	CloseHandle(hSnap);
	byte* b_sig = (byte*)sig;
	int sig_len = strlen(sig);
	byte* addr = (byte*)mod.modBaseAddr;
	for (int i = 0; i < mod.modBaseSize; i++)
	{
		int flag = 0;
		for (int n = 0; n < sig_len; n++)
		{
			if (i + n >= mod.modBaseSize)break;
			if (*(b_sig + n)=='\x3F' || *(b_sig + n) == *(addr + i+ n))
			{
				flag++;
			}
		}
		if (flag == sig_len)
		{
			return (void*)(addr + i);
		}
	}
	return NULL;
}
#endif

bool Skin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceService, IGameResourceServiceServer, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	// Get CSchemaSystem
	{
		HINSTANCE m_hModule = dlmount(WIN_LINUX("schemasystem.dll", "libschemasystem.so"));
		g_pCSchemaSystem = reinterpret_cast<CSchemaSystem*>(reinterpret_cast<CreateInterfaceFn>(dlsym(m_hModule, "CreateInterface"))(SCHEMASYSTEM_INTERFACE_VERSION, nullptr));
		dlclose(m_hModule);
	}

	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Skin::StartupServer), true);
	SH_ADD_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &Skin::GameFrame), true);

	///TEST
	//SH_ADD_HOOK(IGameEventManager2, Event_PlayerConnect, gameeventmanager, SH_MEMBER(this, &Skin::PlayerConnect), true);
   	//SH_ADD_HOOK(IGameEventManager2, Event_PlayerDisconnect, gameeventmanager, SH_MEMBER(this, &Skin::PlayerDisconnect), true);
	///TEST

	gameeventmanager = static_cast<IGameEventManager2*>(CallVFunc<IToolGameEventAPI*, 91>(g_pSource2Server));

	ConVar_Register(FCVAR_GAMEDLL);

	g_WeaponsMap = { {26,"weapon_bizon"},{17,"weapon_mac10"},{34,"weapon_mp9"},{19,"weapon_p90"},{24,"weapon_ump45"},{7,"weapon_ak47"},{8,"weapon_aug"},{10,"weapon_famas"},{13,"weapon_galilar"},{16,"weapon_m4a1"},{60,"weapon_m4a1_silencer"},{39,"weapon_sg556"},{9,"weapon_awp"},{11,"weapon_g3sg1"},{38,"weapon_scar20"},{40,"weapon_ssg08"},{27,"weapon_mag7"},{35,"weapon_nova"},{29,"weapon_sawedoff"},{25,"weapon_xm1014"},{14,"weapon_m249"},{9,"weapon_awp"},{28,"weapon_negev"},{1,"weapon_deagle"},{2,"weapon_elite"},{3,"weapon_fiveseven"},{4,"weapon_glock"},{32,"weapon_hkp2000"},{36,"weapon_p250"},{30,"weapon_tec9"},{61,"weapon_usp_silencer"},{63,"weapon_cz75a"},{64,"weapon_revolver"},{23, "weapon_mp5sd"},{33, "weapon_mp7"} };
	g_KnivesMap = { {59,"weapon_knife"},{42,"weapon_knife"},{526,"weapon_knife_kukri"},{508,"weapon_knife_m9_bayonet"},{500,"weapon_bayonet"},{514,"weapon_knife_survival_bowie"},{515,"weapon_knife_butterfly"},{512,"weapon_knife_falchion"},{505,"weapon_knife_flip"},{506,"weapon_knife_gut"},{509,"weapon_knife_tactical"},{516,"weapon_knife_push"},{520,"weapon_knife_gypsy_jackknife"},{522,"weapon_knife_stiletto"},{523,"weapon_knife_widowmaker"},{519,"weapon_knife_ursus"},{503,"weapon_knife_css"},{517,"weapon_knife_cord"},{518,"weapon_knife_canis"},{521,"weapon_knife_outdoor"},{525,"weapon_knife_skeleton"},{507,"weapon_knife_karambit"} };
	//g_KnivesMap = { {59,"weapon_knife"},{42,"weapon_knife"},{526,"weapon_knife"},{508,"weapon_knife"},{500,"weapon_knife"},{514,"weapon_knife"},{515,"weapon_knife"},{512,"weapon_knife"},{505,"weapon_knife"},{506,"weapon_knife"},{509,"weapon_knife"},{516,"weapon_knife"},{520,"weapon_knife"},{522,"weapon_knife"},{523,"weapon_knife"},{519,"weapon_knife"},{503,"weapon_knife"},{517,"weapon_knife"},{518,"weapon_knife"},{521,"weapon_knife"},{525,"weapon_knife"},{507,"weapon_knife"} };
	g_ItemToSlotMap = { {59, 0},{42, 0},{526, 0},{508, 0},{500, 0},{514, 0},{515, 0},{512, 0},{505, 0},{506, 0},{509, 0},{516, 0},{520, 0},{522, 0},{523, 0},{519, 0},{503, 0},{517, 0},{518, 0},{521, 0},{525, 0},{507, 0}, {42, 0}, {59, 0}, {32, 1}, {61, 1}, {1, 1}, {3, 1}, {2, 1}, {36, 1}, {63, 1}, {64, 1}, {30, 1}, {4, 1}, {14, 2}, {17, 2}, {23, 2}, {33, 2}, {28, 2}, {35, 2}, {19, 2}, {26, 2}, {29, 2}, {24, 2}, {25, 2}, {27, 2}, {34, 2}, {8, 3}, {9, 3}, {10, 3}, {60, 3}, {16, 3}, {38, 3}, {40, 3}, {7, 3}, {11, 3}, {13, 3}, {39, 3} };

	//Test MAP
	SearchMap = {{"weapon_bizon", 26},{"weapon_mac10", 17},{"weapon_mp9", 34},{"weapon_p90", 19},{"weapon_ump45", 24},{"weapon_ak47", 7},{"weapon_aug", 8},{"weapon_famas", 10},{"weapon_galilar", 13},{"weapon_m4a1", 16},{"weapon_m4a1_silencer", 60},{"weapon_sg556", 39},{"weapon_awp", 9},{"weapon_g3sg1", 11},{"weapon_scar20", 38},{"weapon_ssg08", 40},{"weapon_mag7", 27},{"weapon_nova", 35},{"weapon_sawedoff", 29},{"weapon_xm1014", 25},{"weapon_m249", 14},{"weapon_negev", 28},{"weapon_deagle", 1},{"weapon_elite", 2},{"weapon_fiveseven", 3},{"weapon_glock", 4},{"weapon_hkp2000", 32},{"weapon_p250", 36},{"weapon_tec9", 30},{"weapon_usp_silencer", 61},{"weapon_cz75a", 63},{"weapon_revolver", 64},{"weapon_mp5sd", 23},{"weapon_mp7", 33}};
	
	
	//g_GlovesMap={{5031,"Driving gloves"}, {5033,"Motorcycle gloves"}};
	//TESTEND
	

	#ifdef _WIN32	
	byte* vscript = (byte*)FindSignature("vscript.dll", "\xBE\x01\x3F\x3F\x3F\x2B\xD6\x74\x61\x3B\xD6");
	if(vscript)
	{
		DWORD pOld;
		VirtualProtect(vscript, 2, PAGE_EXECUTE_READWRITE, &pOld);
		*(vscript + 1) = 2;
		VirtualProtect(vscript, 2, pOld, &pOld);
	}
	#endif
	return true;
}

bool Skin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IServerGameDLL, GameFrame, g_pSource2Server, SH_MEMBER(this, &Skin::GameFrame), true);
	SH_REMOVE_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Skin::StartupServer), true);

	//SH_REMOVE_HOOK(IGameEventManager2, PlayerConnect, gameeventmanager, SH_MEMBER(this, &Skin::PlayerConnect), true);
   	//SH_REMOVE_HOOK(IGameEventManager2, PlayerDisconnect, gameeventmanager, SH_MEMBER(this, &Skin::PlayerDisconnect), true);
	
	gameeventmanager->RemoveListener(&g_PlayerSpawnEvent);
	gameeventmanager->RemoveListener(&g_RoundPreStartEvent);
	gameeventmanager->RemoveListener(&g_PlayerBuy);
	gameeventmanager->RemoveListener(&g_PlayerSpawnedEvent);
	gameeventmanager->RemoveListener(&g_RoundStart);

	gameeventmanager->RemoveListener(&g_PlayerConnect);
	gameeventmanager->RemoveListener(&g_PlayerDisconnect);
	
	//TEST
	g_pGameEntitySystem->RemoveListenerEntity(&g_EntityListener);//work
	//g_pGameEntitySystem->RemoveListenerEntity(&g_PlayerSpawnedEvent);//nowork
	//TEST

	ConVar_Unregister();
	
	return true;
}

void Skin::NextFrame(std::function<void()> fn)
{
	m_nextFrame.push_back(fn);
}

void Skin::StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession*, const char*)
{
	//#ifdef _WIN32
	//FnUTIL_ClientPrintAll = (UTIL_ClientPrintAll_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18\x57\x48\x81\xEC\x70\x01\x3F\x3F\x8B\xE9");
	//FnGiveNamedItem = (GiveNamedItem_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x18\x48\x89\x74\x24\x20\x55\x57\x41\x54\x41\x56\x41\x57\x48\x8D\x6C\x24\xD9");
	//FnEntityRemove = (EntityRemove_t)FindSignature("server.dll", "\x48\x85\xD2\x0F\x3F\x3F\x3F\x3F\x3F\x57\x48\x3F\x3F\x3F\x48\x89\x3F\x3F\x3F\x48\x8B\xF9\x48\x8B");
	//FnSubClassChange = (SubClassChange_t)FindSignature("server.dll", "\x40\x55\x41\x57\x48\x83\xEC\x78\x83\xBA\x38\x04");
	//#else
	//CModule libserver(g_pSource2Server);
	//FnUTIL_ClientPrintAll = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 D7 41 56 49 89 F6 41 55 41 89 FD").RCast< decltype(FnUTIL_ClientPrintAll) >();
	//FnGiveNamedItem = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 49 89 CE 41 55 49 89 F5 41 54 49 89 D4 53 48 89").RCast<decltype(FnGiveNamedItem)>();
	//FnEntityRemove = libserver.FindPatternSIMD("48 85 F6 74 0B 48 8B 76 10 E9 B2 FE FF FF").RCast<decltype(FnEntityRemove)>();
	//FnUTIL_ClientPrint = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 CF 41 56 49 89 D6 41 55 41 89 F5 41 54 4C 8D A5 A0 FE FF FF").RCast<decltype(FnUTIL_ClientPrint)>();
	//FnSubClassChange = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 83 BE 38 04 00 00 01 0F 8E 47 02").RCast<decltype(FnSubClassChange)>();
	//#endif

	// Signature for sub_F518D0:
// 55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC 38 01 00 00 48 89 95 B8 FE FF FF 
// \x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x53\x48\x81\xEC\x38\x01\x00\x00\x48\x89\x95\xB8\xFE\xFF\xFF
	#ifdef _WIN32
	FnUTIL_ClientPrintAll = (UTIL_ClientPrintAll_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18\x57\x48\x81\xEC\x70\x01\x3F\x3F\x8B\xE9");
	FnGiveNamedItem = (GiveNamedItem_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x18\x48\x89\x74\x24\x20\x55\x57\x41\x54\x41\x56\x41\x57\x48\x8D\x6C\x24\xD9");
	FnEntityRemove = (EntityRemove_t)FindSignature("server.dll", "\x48\x85\xD2\x0F\x3F\x3F\x3F\x3F\x3F\x57\x48\x3F\x3F\x3F\x48\x89\x3F\x3F\x3F\x48\x8B\xF9\x48\x8B");
	FnSubClassChange = (SubClassChange_t)FindSignature("server.dll", "\x40\x55\x41\x57\x48\x83\xEC\x78\x83\xBA\x38\x04");
	FnStateChanged = (StateChanged_t)FindSignature("server.dll", "\x48\x89\x54\x24\x10\x55\x53\x57\x41\x55");

	//TEST

	//constexpr auto PATTERN_GETNEXTSCENEEVENTOFFSET_PTR_OFFSET = "\xe8\x00\x00\x00\x00\x4c\x63\xf0\x49\xc1\xe6";
//constexpr auto MASK_GETNEXTSCENEEVENTOFFSET_PTR_OFFSET = "x????xxxxxx";
//constexpr auto OFFSETSTART_GETNEXTSCENEEVENTOFFSET = 1;
//constexpr auto OFFSETEND_GETNEXTSCENEEVENTOFFSET = 5;
//constexpr auto SIGNATURE_GETNEXTSCENEEVENTOFFSET_PTR_OFFSET = PATTERN_GETNEXTSCENEEVENTOFFSET_PTR_OFFSET MASK_GETNEXTSCENEEVENTOFFSET_PTR_OFFSET;

// Получаем сигнатуру
GetNextSceneEventIDOffset = (GetNextSceneEventIDOffset_t)FindSignature("server.dll", "\x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x53\x48\x81\xEC\x38\x01\x00\x00\x48\x89\x95\xB8\xFE\xFF\xFF");

// Получаем смещение
//int32_t offsetFromInstruction = *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(GetNextSceneEventIDOffset) + OFFSETSTART_GETNEXTSCENEEVENTOFFSET);
//GetNextSceneEventIDOffset = reinterpret_cast<GetNextSceneEventIDOffset_t>(reinterpret_cast<uint8_t*>(GetNextSceneEventIDOffset) + OFFSETEND_GETNEXTSCENEEVENTOFFSET + offsetFromInstruction);
	
	#else
	CModule libserver(g_pSource2Server);
	FnUTIL_ClientPrintAll = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 D7 41 56 49 89 F6 41 55 41 89 FD").RCast< decltype(FnUTIL_ClientPrintAll) >();
	// 55 48 89 E5 41 57 41 56 49 89 CE 41 55 49 89 F5 41 54 49 89 D4 53 48 89
	// 55 48 89 E5 41 57 41 56 45 31 F6 41 55 49 89 CD
	FnGiveNamedItem = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 49 89 CE 41 55 49 89 F5 41 54 49 89 D4").RCast<decltype(FnGiveNamedItem)>();
	FnEntityRemove = libserver.FindPatternSIMD("48 85 F6 74 0B 48 8B 76 10 E9 B2 FE FF FF").RCast<decltype(FnEntityRemove)>();
	FnUTIL_ClientPrint = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 CF 41 56 49 89 D6 41 55 41 89 F5 41 54 4C 8D A5 A0 FE FF FF").RCast<decltype(FnUTIL_ClientPrint)>();
	FnSubClassChange = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 83 BE 38 04 00 00 01 0F 8E 47 02").RCast<decltype(FnSubClassChange)>();
	FnStateChanged = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 41 55 41 54 53 89 D3").RCast<decltype(FnStateChanged)>();

	//TEST
	
	// Signature for sub_F518D0:
	// 55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC 38 01 00 00 48 89 95 B8 FE FF FF 
	// \x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x53\x48\x81\xEC\x38\x01\x00\x00\x48\x89\x95\xB8\xFE\xFF\xFF
/*
	constexpr uint8_t PATTERN_FUNCTION_PTR[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x53, 0x48, 0x81, 0xEC, 0x38, 0x01, 0x00, 0x00, 0x48, 0x89, 0x95, 0xB8, 0xFE, 0xFF, 0xFF};
	constexpr auto MASK_FUNCTION_PTR = "xxxxxxxxxxxxxxxxxx??xxxxxxxx";
	constexpr auto OFFSETSTART_FUNCTION_PTR = 1;
	constexpr int32_t OFFSETEND_FUNCTION_PTR = 5;

auto patternResult = libserver.FindPatternSIMD(PATTERN_FUNCTION_PTR, MASK_FUNCTION_PTR);
if (patternResult)
{
    uint8_t* relCallPtr = reinterpret_cast<uint8_t*>(patternResult.RCast<uint64_t*>());
    int32_t offsetFromInstruction = *reinterpret_cast<int32_t*>(relCallPtr + OFFSETSTART_FUNCTION_PTR);

    // Изменяем размер вектора, чтобы он был достаточно большим
    size_t patternLength = sizeof(PATTERN_FUNCTION_PTR);
    std::vector<uint8_t> name(patternLength);

    // Копируем значения из области памяти в вектор
    std::memcpy(name.data(), relCallPtr + OFFSETEND_FUNCTION_PTR + offsetFromInstruction, patternLength);

    uint8_t temppat[name.size()];
    std::copy(name.begin(), name.end(), temppat);
	std::string patternString = reinterpret_cast<char*>(temppat);
	GetNextSceneEventIDOffset = libserver.FindPatternSIMD(patternString.c_str()).RCast<decltype(GetNextSceneEventIDOffset)>();
}
	*/
	GetNextSceneEventIDOffset = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC 38 01 00 00 48 89 95 B8 FE FF FF").RCast<decltype(GetNextSceneEventIDOffset)>();
	#endif

	
	g_pGameRules = nullptr;

	static bool bDone = false;
	if (!bDone)
	{
		g_pGameEntitySystem = *reinterpret_cast<CGameEntitySystem**>(reinterpret_cast<uintptr_t>(g_pGameResourceService) + WIN_LINUX(0x58, 0x50));
		g_pEntitySystem = g_pGameEntitySystem;

		g_pGameEntitySystem->AddListenerEntity(&g_EntityListener);

		gameeventmanager->AddListener(&g_PlayerSpawnEvent, "player_spawn", true);
		gameeventmanager->AddListener(&g_RoundPreStartEvent, "round_prestart", true);

		//Test//////////////////////
		gameeventmanager->AddListener(&g_PlayerBuy, "item_purchase", true);//work
		gameeventmanager->AddListener(&g_PlayerSpawnedEvent,"player_spawned",true);
		gameeventmanager->AddListener(&g_RoundStart,"round_start",true);
		gameeventmanager->AddListener(&g_PlayerConnect,"player_connect",true);//tested
		gameeventmanager->AddListener(&g_PlayerDisconnect,"player_disconnect",true);//tested
		//test/////////////////////
		bDone = true;
	}
}

void Skin::GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
	// CTimer
	
	if (simulating && g_bHasTicked) {
		g_flUniversalTime += GetGameGlobals()->curtime - g_flLastTickedTime;
	} else {
		g_flUniversalTime += GetGameGlobals()->interval_per_tick;
	}

	g_flLastTickedTime = GetGameGlobals()->curtime;
	g_bHasTicked = true;

	for (int i = g_timers.Tail(); i != g_timers.InvalidIndex();) {
		auto timer = g_timers[i];

		int prevIndex = i;
		i = g_timers.Previous(i);

		if (timer->m_flLastExecute == -1) {
			timer->m_flLastExecute = g_flUniversalTime;
		}

		// Timer execute
		if (timer->m_flLastExecute + timer->m_flTime <= g_flUniversalTime) {
			timer->Execute();
			if (!timer->m_bRepeat) {
				delete timer;
				g_timers.Remove(prevIndex);
			} else {
				timer->m_flLastExecute = g_flUniversalTime;
			}
		}
	}
	
	if (!g_pGameRules) {
		CCSGameRulesProxy* pGameRulesProxy = static_cast<CCSGameRulesProxy*>(UTIL_FindEntityByClassname(nullptr, "cs_gamerules"));
		if (pGameRulesProxy) {
			g_pGameRules = pGameRulesProxy->m_pGameRules();
		}
	}
	
	while (!m_nextFrame.empty()) {
		m_nextFrame.front()();
		m_nextFrame.pop_front();
	}
}

void CPlayerSpawnEvent::FireGameEvent(IGameEvent* event)
{
	if (!g_pGameRules)//TEST
	{
        	return;
	}

	
	CBasePlayerController* pPlayerController = static_cast<CBasePlayerController*>(event->GetPlayerController("userid"));
	
    	if (!pPlayerController)
    	{
        	return;
    	}

	if (!pPlayerController || pPlayerController->m_steamID() == 0) // Ignore bots
	{
		return;
	}
	g_Skin.NextFrame([hPlayerController = CHandle<CBasePlayerController>(pPlayerController), pPlayerController = pPlayerController]()
	{
		CCSPlayerController* pCSPlayerController = dynamic_cast<CCSPlayerController*>(pPlayerController);
		
		int64_t steamid = pPlayerController->m_steamID();
		if (pCSPlayerController)
		{
    			CCSPlayerPawnBase* playerPawn = pCSPlayerController->m_hPlayerPawn();

			
			//CCSPlayer_ViewModelServices* vms = playerPawn->m_pViewModelServices();
			//C_CSGOViewModel* csgoview = vms->m_hViewModel();
			//META_CONPRINTF("vms %p\n", vms);
            		//META_CONPRINTF("csgoview %p\n",csgoview);
			
    			if (playerPawn)
			{
				if (players.find(steamid) != players.end()) 
				{
    				// Игрок существует в вашем контейнере
					if(players[steamid].PC==nullptr)
					{
						
						SC_CBaseEntity* pSCBaseEntity = dynamic_cast<SC_CBaseEntity*>(pPlayerController);
    						META_CONPRINTF("Player ENTITY: %llu\n", pSCBaseEntity);

						
						META_CONPRINTF("Player Connect: , SteamID: %llu\n", steamid);
						state[steamid]=true;
						
						AddOrUpdatePlayer(steamid,pCSPlayerController,playerPawn,GETSKINS(steamid),pSCBaseEntity);
						state[steamid]=false;
						std::thread([pCSPlayerController, playerPawn, steamid,pSCBaseEntity]() {
        						ThreadUpdate(steamid,pCSPlayerController,playerPawn,pSCBaseEntity);
			
						}).detach();
					}
					
				} 
				else 
				{
    				// Игрок не существует в вашем контейнере, возможно, нужно выполнить какие-то действия

					SC_CBaseEntity* pSCBaseEntity = dynamic_cast<SC_CBaseEntity*>(pPlayerController);

					state[steamid]=true;
					AddOrUpdatePlayer(steamid,pCSPlayerController,playerPawn,GETSKINS(steamid),pSCBaseEntity);

					state[steamid]=false;
					std::thread([pCSPlayerController, playerPawn, steamid,pSCBaseEntity]() {
        						ThreadUpdate(steamid,pCSPlayerController,playerPawn,pSCBaseEntity);
							std::this_thread::sleep_for(std::chrono::milliseconds(150));
			
			
						}).detach();
					return;
				}
				
    			}
    			else
			{
        			return;
			}
		}
		else
		{
			return;
		}
		});
	return;
}


//TEST GLOVES

void ForceGlovesUpdate(CCSGOViewModel* viewModel) {
    long magicNr = 4047747114;
    META_CONPRINTF("magic %lld\n",magicNr);
    // Разыменовываем указатель для получения значения float
	
    //float viewTargetY = viewModel->m_CachedViewTarget().y;
	
    //float viewTargetY = viewModel->m_viewtarget().y;
	//m_vLookTargetPosition
    //META_CONPRINTF("viewTargetY %p\n",viewModel->m_viewtarget().y);
	META_CONPRINTF("viewTargetY %p\n",viewModel->m_viewtarget().y);
    // Передаем значение float вместо указателя и преобразуем long в int64_t
   //int64_t offset = GetNextSceneEventIDOffset(&viewModel->m_viewtarget().y, &magicNr, magicNr, false);
	int64_t offset = GetNextSceneEventIDOffset(&viewModel->m_viewtarget().y, &magicNr, magicNr, false);
   META_CONPRINTF("offset %lld\n",offset);
   //uint8_t* dataLoc = *reinterpret_cast<uint8_t**>(&viewModel->m_viewtarget().y) + offset * 0x10;

	uint8_t* dataLoc = *reinterpret_cast<uint8_t**>(&viewModel->m_viewtarget().y) + offset * 0x10;

   META_CONPRINTF("dataLoc %lld\n",dataLoc);
    *reinterpret_cast<int*>(dataLoc + 0xc) -= 1;
}

// weird shit to make the gloves update properly
void forceAsyncUpdate(CCSPlayerPawn* pawn, CCSGOViewModel* viewModel) {
//[{"skin_id":10006,"float":0.000100000000000000004792173602385929598312941379845142364501953125,"seed":0,"nametag":"","side":0,"stickers":[],"stattrak":false,"stattrak_count":0,"type":"glove","weapon_id":5027}]
	std::thread([pawn,viewModel]() {
		for (int i = 0; i < 10; i++) {
		// we should probably try checking if memory addresses have been allocated...
		//pawn->m_EconGloves().m_iItemDefinitionIndex() = pref.weaponID; // this will be the gloves id
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture prefab", static_cast<float>(pref.paintKitID));
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture seed", static_cast<float>(pref.seed));
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture wear", static_cast<float>(pref.wearValue));

		META_CONPRINTF("TRY M_ECONGLOVES \n");
		pawn->m_EconGloves().m_iItemDefinitionIndex() = 5027; // this will be the gloves id
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture prefab", static_cast<float>(10006));
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture seed", static_cast<float>(0));
		//pawn->m_EconGloves().SetAttributeValueByName("set item texture wear", static_cast<float>(0.0001f));
		META_CONPRINTF("TRY M_ECONGLOVES.m_iItemDefinitionIndex() %lld\n", pawn->m_EconGloves().m_iItemDefinitionIndex());
		//pawn->m_EconGloves().m_bInitialized() = true;
		
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        		
			ForceGlovesUpdate(viewModel);
		
		//pawn->m_bNeedToReApplyGloves() = true;
		
		//std::this_thread::sleep_for(std::chrono::milliseconds(150));
		}
			}).detach();
}
//TEST END
void Event_ItemPurchase::FireGameEvent(IGameEvent* event)
{
	const std::string weapon = event->GetString("weapon");
	const int userId = event->GetInt("userid");
	CBasePlayerController* pPlayerController = static_cast<CBasePlayerController*>(event->GetPlayerController("userid"));
	//dynamic_cast<SC_CBaseEntity*>(
	CCSPlayerController* pCSPlayerController = dynamic_cast<CCSPlayerController*>(pPlayerController);
    	CCSPlayerPawnBase* playerPawn = pCSPlayerController->m_hPlayerPawn();
	
	CCSPlayer_ViewModelServices* vms = playerPawn->m_pViewModelServices();
	CCSGOViewModel* viewModel = vms->m_hViewModel();
	META_CONPRINTF("vms %p\n", vms);
	META_CONPRINTF("viewModel %p\n", viewModel);
	
	CCSPlayerPawn* pawn = dynamic_cast<CCSPlayerPawn*>(playerPawn);
	C_EconItemView CEcon= pawn->m_EconGloves();
	META_CONPRINTF("good CEcon \n");
	forceAsyncUpdate(pawn, viewModel);
	
	//META_CONPRINTF("CEcon %c\n", CEcon);
	g_Skin.NextFrame([hPlayerController = CHandle<CBasePlayerController>(pPlayerController), pPlayerController = pPlayerController,weapon=weapon]()
	{
	
		//dynamic_cast<SC_CBaseEntity*>(

		
		
		

    		//SC_ViewModel* pSCViewModel = ToBaseViewModel(pSCBaseEntity);

        	//CBaseViewModel* pViewModel = ToBaseViewModel(pSCViewModel);

            // Теперь у вас есть указатель на CBaseViewModel, который можно использовать
           // META_CONPRINTF("CBaseViewModel %p\n", pSCViewModel);

		//SC_CBasePlayer* player= pSCBaseEntity->m_pPredictionPlayer();
		//CBasePlayer* player = dynamic_cast<CBasePlayer*>(pSCBaseEntity);
		//META_CONPRINTF("SC_CBasePlayer %lld\n",vm);
		//SC_CBasePlayer* scbase= dynamic_cast<SC_CBasePlayer*>(playerPawn->GetViewModel());
		//int ids=SearchMap[weapon];
		//std::thread([pCSPlayerController, playerPawn, ids=ids]() {
        	//	int64_t steamid = pCSPlayerController->m_steamID();
		//	std::this_thread::sleep_for(std::chrono::milliseconds(150));
		//	
		//	//TestSkinchanger(steamid, 5031);
			
		//}).detach();
	
		//META_CONPRINTF("PLAYER BUY WEAPON %d\n",ids);
	});
}

void Event_PlayerSpawned::FireGameEvent(IGameEvent* event)
{
	CBasePlayerController* pPlayerController = static_cast<CBasePlayerController*>(event->GetPlayerController("userid"));
	if (!pPlayerController || pPlayerController->m_steamID() == 0) // Ignore bots
	{
		return;
	}
	else
	{
	}

	
	META_CONPRINTF("PLAYER Spawned\n");
}

void OnRoundStart::FireGameEvent(IGameEvent* event) 
{
    	META_CONPRINTF("RoundStarted\n");
}

uint64_t ExtractSteamIDFromNetworkID(const std::string& networkID) {
try {
        size_t start = networkID.find(":1:") + 3;
        size_t end = networkID.find("]", start);

        if (start != std::string::npos && end != std::string::npos) {
            std::string accountIDStr = networkID.substr(start, end - start);
            uint32_t accountID = std::stoi(accountIDStr);
            uint64_t steamID = ((uint64_t)accountID) + 76561197960265728ULL;//const+accountid
            return steamID;
        } else {
            return 0;
        }
    } catch (const std::exception& e) {
        return 0;
    }
}

void Event_PlayerConnect::FireGameEvent(IGameEvent* event) {
    try {
        bool isBot = event->GetBool("bot");
        if (isBot) {
            return;
        }
	
        std::string netid = event->GetString("networkid");
	    if (netid == "BOT")
		{
    			return;
		}
        uint64_t steamid = ExtractSteamIDFromNetworkID(netid);

        META_CONPRINTF("Player Connected: , SteamID: %llu\n", steamid);
    } catch (const std::exception& e) {
    }
}

void Event_PlayerDisconnect::FireGameEvent(IGameEvent* event) {
    try {
        bool isBot = event->GetBool("bot");
        if (isBot) {
        	return;
        }

        std::string netid = event->GetString("networkid");
	if (netid == "<BOT>")
	{
    		return;
	}
        uint64_t steamid = ExtractSteamIDFromNetworkID(netid);
	
            players[steamid].firstspawn = false;
            auto it = players.find(steamid);
            if (it != players.end()) {
		ClearPlayer(steamid);
                players.erase(it);
		META_CONPRINTF("ERASE STRUCT\n");
            }
        
        META_CONPRINTF("Player Disconnected: , SteamID: %llu\n", steamid);
    } catch (const std::exception& e) {
    }
}

//TEST END

void CRoundPreStartEvent::FireGameEvent(IGameEvent* event)
{
	if (g_pGameRules)
	{
		g_bPistolRound = g_pGameRules->m_totalRoundsPlayed() == 0 || (g_pGameRules->m_bSwitchingTeamsAtRoundReset() && g_pGameRules->m_nOvertimePlaying() == 0) || g_pGameRules->m_bGameRestart();
	}
}

void CEntityListener::OnEntityParentChanged(CEntityInstance *pEntity, CEntityInstance *pNewParent) {
	CBasePlayerWeapon* pBasePlayerWeapon = dynamic_cast<CBasePlayerWeapon*>(pEntity);
	CEconEntity* pCEconEntityWeapon = dynamic_cast<CEconEntity*>(pEntity);
	/*if(!pBasePlayerWeapon) return;
	g_Skin.NextFrame([pBasePlayerWeapon = pBasePlayerWeapon, pCEconEntityWeapon = pCEconEntityWeapon]()
	{
		int64_t steamid = pCEconEntityWeapon->m_OriginalOwnerXuidLow() | (static_cast<int64_t>(pCEconEntityWeapon->m_OriginalOwnerXuidHigh()) << 32);
		int64_t weaponId = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex();
		if(!steamid) {
			return;
		}

		auto skin_parm = g_PlayerSkins.find(steamid);
		if(skin_parm == g_PlayerSkins.end()) {
			return;
		}

		if(skin_parm->second.m_iItemDefinitionIndex == -1 || skin_parm->second.m_nFallbackPaintKit == -1 || skin_parm->second.m_nFallbackSeed == -1 || skin_parm->second.m_flFallbackWear == -1) {
			return;
		}
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex() = skin_parm->second.m_iItemDefinitionIndex;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDLow() = -1;
		// pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDHigh() = g_iItemIDHigh;
		// pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID() = g_iItemIDHigh++;
		

		META_CONPRINTF("skin_parm->second.m_nFallbackPaintKit: %d\n", skin_parm->second.m_nFallbackPaintKit);
		META_CONPRINTF("skin_parm->second.m_nFallbackSeed: %d\n", skin_parm->second.m_nFallbackSeed);
		META_CONPRINTF("skin_parm->second.m_flFallbackWear: %f\n", skin_parm->second.m_flFallbackWear);
		META_CONPRINTF("skin_parm->second.m_iItemDefinitionIndex: %d\n", skin_parm->second.m_iItemDefinitionIndex);

		pCEconEntityWeapon->m_nFallbackPaintKit() = skin_parm->second.m_nFallbackPaintKit;
		pCEconEntityWeapon->m_nFallbackSeed() = skin_parm->second.m_nFallbackSeed;
		pCEconEntityWeapon->m_flFallbackWear() = skin_parm->second.m_flFallbackWear;

		// print those 4 values from the item itself
		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackPaintKit: %d\n", pCEconEntityWeapon->m_nFallbackPaintKit());
		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackSeed: %d\n", pCEconEntityWeapon->m_nFallbackSeed());
		META_CONPRINTF("pCEconEntityWeapon->m_flFallbackWear: %f\n", pCEconEntityWeapon->m_flFallbackWear());
		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackPaintKit: %d\n", pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());


	});*/
}

void CEntityListener::OnEntityCreated(CEntityInstance *pEntity) {
	CBasePlayerWeapon* pBasePlayerWeapon = dynamic_cast<CBasePlayerWeapon*>(pEntity);
	if(!pBasePlayerWeapon) return;
}

void CEntityListener::OnEntityDeleted(CEntityInstance *pEntity) {
	// META_CONPRINTF("OnEntityDeleted\n");
}

void CEntityListener::OnEntitySpawned(CEntityInstance* pEntity)
{
	META_CONPRINTF("OnEntitySpawned\n");
	CBasePlayerWeapon* pBasePlayerWeapon = dynamic_cast<CBasePlayerWeapon*>(pEntity);
	CEconEntity* pCEconEntityWeapon = dynamic_cast<CEconEntity*>(pEntity);
	if(!pBasePlayerWeapon) return;
	
	
	
	g_Skin.NextFrame([pBasePlayerWeapon = pBasePlayerWeapon, pCEconEntityWeapon = pCEconEntityWeapon, pEntity=pEntity]()
	{
		
		int64_t steamid = pCEconEntityWeapon->m_OriginalOwnerXuidLow() | (static_cast<int64_t>(pCEconEntityWeapon->m_OriginalOwnerXuidHigh()) << 32);
		int64_t weaponId = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex();
		META_CONPRINTF( "----------------------------------------------------\n");
		if(!steamid) {
			return;
		}

		
		
		TestSkinchanger(steamid, weaponId);
		
		
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto skin_parm = g_PlayerSkins.find(steamid);
		if(skin_parm == g_PlayerSkins.end()) {
			return;
		}

		if(skin_parm->second.m_iItemDefinitionIndex == -1 || skin_parm->second.m_nFallbackPaintKit == -1 || skin_parm->second.m_nFallbackSeed == -1 || skin_parm->second.m_flFallbackWear == -1) {
			return;
		}

		uint64_t newItemID = 16384;
		uint32_t newItemIDLow = newItemID & 0xFFFFFFFF;
		uint32_t newItemIDHigh = newItemID >> 32;

		

		
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex() = skin_parm->second.m_iItemDefinitionIndex;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDLow() = newItemIDLow;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDHigh() = newItemIDHigh;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID() = newItemID;
		
		if (!skin_parm->second.m_nameTag.empty()) {
    			pCEconEntityWeapon->m_AttributeManager().m_Item().m_szCustomName() = static_cast<char>(skin_parm->second.m_nameTag[0]);
			//pCEconEntityWeapon->m_AttributeManager().m_Item().m_szCustomName() = skin_parm->second.m_nameTag*;
		}
		// pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID() = g_iItemIDHigh++;

		META_CONPRINTF("skin_parm->second.m_nFallbackPaintKit: %d\n", skin_parm->second.m_nFallbackPaintKit);
		META_CONPRINTF("skin_parm->second.m_nFallbackSeed: %d\n", skin_parm->second.m_nFallbackSeed);
		META_CONPRINTF("skin_parm->second.m_flFallbackWear: %f\n", skin_parm->second.m_flFallbackWear);
		META_CONPRINTF("skin_parm->second.m_iItemDefinitionIndex: %d\n", skin_parm->second.m_iItemDefinitionIndex);

		//ONLYTEST
		pCEconEntityWeapon->m_nFallbackPaintKit() = skin_parm->second.m_nFallbackPaintKit;
		pCEconEntityWeapon->m_nFallbackSeed() = skin_parm->second.m_nFallbackSeed;
		pCEconEntityWeapon->m_flFallbackWear() = skin_parm->second.m_flFallbackWear;
		
		//pCEconEntityWeapon->m_nFallbackStatTrak() = 100;
		//TEST
		
		// pCEconEntityWeapon->m_OriginalOwnerXuidLow() = -1;
		// pCEconEntityWeapon->m_OriginalOwnerXuidHigh() = -1;

		//????????????????????????????????????????????????????

	//try
	//{
		auto sticker_parm = g_PlayerStickers.find(steamid);
			if(sticker_parm != g_PlayerStickers.end() && FEATURE_STICKERS) {
				// Work in progress
				if (sticker_parm->second.stickerDefIndex1 != 0) {
					pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(113 + 1 * 4, sticker_parm->second.stickerDefIndex1);
					//if (sticker_parm->second.stickerWear1 != 0) {
						//pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(114 + 1 * 4, sticker_parm->second.stickerWear1);
					//}
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerDefIndex1: %d\n", sticker_parm->second.stickerDefIndex1); }
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerWear1: %f\n", sticker_parm->second.stickerWear1); }
				}

				if (sticker_parm->second.stickerDefIndex2 != 0) {
					pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(117 + 2 * 4, sticker_parm->second.stickerDefIndex2);
					//if (sticker_parm->second.stickerWear2 != 0) {
						//pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(118 + 2 * 4, sticker_parm->second.stickerWear2);
					//}
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerDefIndex2: %d\n", sticker_parm->second.stickerDefIndex2); }
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerWear2: %f\n", sticker_parm->second.stickerWear2); }
				}

				if (sticker_parm->second.stickerDefIndex3 != 0) {
					pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(121 + 3 * 4, sticker_parm->second.stickerDefIndex3);
					//if (sticker_parm->second.stickerWear3 != 0) {
						//pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(122 + 3 * 4, sticker_parm->second.stickerWear3);
					//}
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerDefIndex3: %d\n", sticker_parm->second.stickerDefIndex3); }
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerWear3: %f\n", sticker_parm->second.stickerWear3); }
				}

				if (sticker_parm->second.stickerDefIndex4 != 0) {
					pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(125 + 4 * 4, sticker_parm->second.stickerDefIndex4);
					//if (sticker_parm->second.stickerWear4 != 0) {
						//pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().AddAttribute(126 + 4 * 4, sticker_parm->second.stickerWear4);
					//}
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerDefIndex4: %d\n", sticker_parm->second.stickerDefIndex4); }
					if (DEBUG_OUTPUT) { META_CONPRINTF("sticker_parm->second.stickerWear4: %f\n", sticker_parm->second.stickerWear4); }
				}



				sticker_parm->second.stickerDefIndex1 = 0;
				sticker_parm->second.stickerDefIndex2 = 0;
				sticker_parm->second.stickerDefIndex3 = 0;
				sticker_parm->second.stickerDefIndex4 = 0;
				sticker_parm->second.stickerWear1 = 0;
				sticker_parm->second.stickerWear2 = 0;
				sticker_parm->second.stickerWear3 = 0;
				sticker_parm->second.stickerWear4 = 0;

				META_CONPRINTF("m_AttributeList().m_Attributes().Count(): %d\n", pBasePlayerWeapon->m_AttributeManager().m_Item().m_AttributeList().m_Attributes.Count());
				}
		//}
		//catch (const std::exception& e)
		//{
    			//META_CONPRINTF("ERRORRRRRRRRRRRRRRRRRRRRRRRRRRR STICCCCCCCKERSSSSSSSSSSSSSSSSSSSSS %lld\n");
		//}

		//????????????????????????????????????????????????????


			pBasePlayerWeapon-> m_CBodyComponent ()-> m_pSceneNode ()-> GetSkeletonInstance ()-> m_modelState (). m_MeshGroupMask () = 2 ;
		
		// pCEconEntityWeapon->m_AttributeManager().m_Item().m_iAccountID() = 9727743;

		auto knife_name = g_KnivesMap.find(weaponId);
		if(knife_name != g_KnivesMap.end()) {
			char buf[64] = {0};
			int index = static_cast<CEntityInstance*>(pBasePlayerWeapon)->m_pEntity->m_EHandle.GetEntryIndex();
			sprintf(buf, "i_subclass_change %d %d", skin_parm->second.m_iItemDefinitionIndex, index);
			engine->ServerCommand(buf);
			META_CONPRINTF( "i_subclass_change triggered\n");

			new CTimer(1.0f, false, false, [pCEconEntityWeapon, skin_parm]() {
				char buf[255] = { 0 };
				//sprintf(buf, "%s Timer executed", CHAT_PREFIX);
				//FnUTIL_ClientPrintAll(3, buf,nullptr, nullptr, nullptr, nullptr);
				pCEconEntityWeapon->m_nFallbackPaintKit() = skin_parm->second.m_nFallbackPaintKit;
				pCEconEntityWeapon->m_nFallbackSeed() = skin_parm->second.m_nFallbackSeed;
				pCEconEntityWeapon->m_flFallbackWear() = skin_parm->second.m_flFallbackWear;
			});
			//delete CTimer;
		}

		META_CONPRINTF("low: %d\n", pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDLow());
		META_CONPRINTF("high: %d\n", pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDHigh());
		META_CONPRINTF("id: %d\n", pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID());

		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackPaintKit: %d\n", pCEconEntityWeapon->m_nFallbackPaintKit());
		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackSeed: %d\n", pCEconEntityWeapon->m_nFallbackSeed());
		META_CONPRINTF("pCEconEntityWeapon->m_flFallbackWear: %f\n", pCEconEntityWeapon->m_flFallbackWear());
		META_CONPRINTF("pCEconEntityWeapon->m_nFallbackPaintKit: %d\n", pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());

		META_CONPRINTF( "weaponId: %d\n", weaponId);
		META_CONPRINTF( "class: %s\n", static_cast<CEntityInstance*>(pBasePlayerWeapon)->m_pEntity->m_designerName.String());
		META_CONPRINTF("New Item: %s\n", pBasePlayerWeapon->GetClassname());
		META_CONPRINTF("index = %d\n", pBasePlayerWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());
		META_CONPRINTF("initialized = %d\n", pBasePlayerWeapon->m_AttributeManager().m_Item().m_bInitialized());
		META_CONPRINTF( "steamId: %lld itemId: %d itemId2: %d\n", steamid, skin_parm->second.m_iItemDefinitionIndex, pBasePlayerWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());

		//TESTFORCE//nowork
		//if (players.find(steamid) != players.end()) {
        		//players[steamid].PlayerWeapons[weaponId] = pEntity;
    		//}
		//TESTFORCE
		
		skin_parm->second.m_iItemDefinitionIndex = -1;
		skin_parm->second.m_nFallbackPaintKit = -1;
		skin_parm->second.m_nFallbackSeed = -1;
		skin_parm->second.m_flFallbackWear = -1;
		skin_parm->second.m_nameTag.clear();

		META_CONPRINTF( "----------------------------------------------------\n");
		
	});
	
}
//TEST FUNC CHANGE
void TestSkinchanger(int64_t steamid, int weapon_id)
{
    if(weapon_id==0)
    {
	    return;
    }
	
    if(weapon_id==59|| weapon_id==42)
    {
	    	META_CONPRINTF("Weapon IDIDIDIDIDIID %lld\n", weapon_id);
	    	std::thread([steamid, weapon_id]() {
			
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
        		SkinChangerKnife(steamid);
			
		}).detach();
	
	return;
    }

    CCSPlayerController* pPlayerController=players[steamid].PC;
    CCSPlayerPawnBase* pPlayerPawn=players[steamid].PP;
    SC_CBaseEntity* pSCBaseEntity=players[steamid].PBE;
    int teamnum= pSCBaseEntity->m_iTeamNum();
	
    if (!pPlayerPawn || pPlayerPawn->m_lifeState() != LIFE_ALIVE || !pPlayerController) {
        META_CONPRINTF("TestSkinchanger: Invalid player or controller\n");
	    
        return;
    }

    char buf[255] = { 0 };

    bool isKnife = false;
    META_CONPRINTF("STEAM IDIDIDIDIDIID %lld\n", steamid);
    META_CONPRINTF("Weapon IDIDIDIDIDIID %lld\n", weapon_id);



std::map<int, std::vector<nlohmann::json>> Temp = players[steamid].PlayerSkins;

int skin_id = -1;
float skin_float = -1.0f;
int seed = -1;
std::string nametag;
int side = -1;
bool stattrak = false;
int weapon_id_API = -1;
int stattrak_count = -1;

try
{
    auto it = Temp.find(weapon_id);
    if (it != Temp.end())
    {
        std::vector<nlohmann::json>& weaponDataList = it->second;

        for (const auto& weaponData : weaponDataList)
        {
            side = static_cast<int>(weaponData["side"]);
            if (side == teamnum || side == 0)
            {
                skin_id = weaponData["skin_id"];
                skin_float = weaponData["float"];
                seed = weaponData["seed"];
		if (!weaponData["nametag"].empty()) {
    			nametag = weaponData["nametag"];
		}
                //nametag = weaponData["nametag"];
                //stattrak = weaponData["stattrak"];
                weapon_id_API = weaponData["weapon_id"];
                //stattrak_count = weaponData["stattrak_count"];

		     nlohmann::json stickersJson = {
        		{"stickers", weaponData["stickers"]}
    		};
if (!stickersJson["stickers"].empty()) {

    for (const auto& sticker : stickersJson["stickers"]) {
        int position = sticker["position"];

        switch (position) {
				case 0:
        				g_PlayerStickers[steamid].stickerDefIndex1 = sticker["id"];
        				g_PlayerStickers[steamid].stickerWear1 = static_cast<float>(sticker["wear"]) / 100.0f - 0.0000001f;
					META_CONPRINTF("sticker_parm->second.stickerDefIndex1: %d\n", g_PlayerStickers[steamid].stickerDefIndex1);
					META_CONPRINTF("sticker_parm->second.stickerWear1: %f\n", g_PlayerStickers[steamid].stickerWear1);

       					break;
    				case 1:
        				g_PlayerStickers[steamid].stickerDefIndex2 = sticker["id"];
        				g_PlayerStickers[steamid].stickerWear1 = static_cast<float>(sticker["wear"]) / 100.0f - 0.0000001f;
					META_CONPRINTF("sticker_parm->second.stickerDefIndex1: %d\n", g_PlayerStickers[steamid].stickerDefIndex2);
					META_CONPRINTF("sticker_parm->second.stickerWear1: %f\n", g_PlayerStickers[steamid].stickerWear2);
        				break;
    				case 2:
        				g_PlayerStickers[steamid].stickerDefIndex3 = sticker["id"];
        				g_PlayerStickers[steamid].stickerWear1 = static_cast<float>(sticker["wear"]) / 100.0f - 0.0000001f;
					META_CONPRINTF("sticker_parm->second.stickerDefIndex1: %d\n", g_PlayerStickers[steamid].stickerDefIndex3);
					META_CONPRINTF("sticker_parm->second.stickerWear1: %f\n", g_PlayerStickers[steamid].stickerWear3);
       	 				break;
    				case 3:
        				g_PlayerStickers[steamid].stickerDefIndex4 = sticker["id"];
        				g_PlayerStickers[steamid].stickerWear1 = static_cast<float>(sticker["wear"]) / 100.0f - 0.0000001f;
					META_CONPRINTF("sticker_parm->second.stickerDefIndex1: %d\n", g_PlayerStickers[steamid].stickerDefIndex4);
					META_CONPRINTF("sticker_parm->second.stickerWear1: %f\n", g_PlayerStickers[steamid].stickerWear4);
        				break;
            default:
                break;
        }
    }
} else {
    META_CONPRINTF("STICKERS IS NOT FOUND\n");
}

                META_CONPRINTF("FOUND TEAMNUM AND SIDE %lld\n");
                break;
            }
            else
            {
                side = -1;
                skin_id = -1;
            }
        }
    }
}
catch (const std::exception& e)
{
    META_CONPRINTF("ERROR %lld\n");
}

if (weapon_id_API < 0)
{
    return;
}
	
    auto weapon_name = g_WeaponsMap.find(weapon_id_API);
    g_PlayerSkins[steamid].m_iItemDefinitionIndex = weapon_id_API;
    g_PlayerSkins[steamid].m_nFallbackPaintKit = skin_id;
    g_PlayerSkins[steamid].m_nFallbackSeed = seed;
    g_PlayerSkins[steamid].m_flFallbackWear = skin_float;
    if (!nametag.empty()) {
    	 g_PlayerSkins[steamid].m_nameTag = nametag;
    }
	
    //TEST END
if(g_PlayerSkins[steamid].m_iItemDefinitionIndex != 0 && g_PlayerSkins[steamid].m_nFallbackPaintKit !=0 && g_PlayerSkins[steamid].m_nFallbackSeed != 0 &&  g_PlayerSkins[steamid].m_flFallbackWear !=0.0f)
{
		
	
    CPlayer_WeaponServices* pWeaponServices = pPlayerPawn->m_pWeaponServices();

    META_CONPRINTF("TestSkinchanger: Weapon id %lld\n", weapon_id_API);
    META_CONPRINTF("TestSkinchanger: paint_kit %lld\n", skin_id);
    META_CONPRINTF("TestSkinchanger: pattern_id %lld\n", seed);
    META_CONPRINTF("TestSkinchanger: wear %f\n", skin_float);

    if (weapon_name == g_WeaponsMap.end()) {
        weapon_name = g_KnivesMap.find(weapon_id);
        isKnife = true;
    }

    if (weapon_name == g_KnivesMap.end()) {
        sprintf(buf, "%s\x02 Unknown Weapon/Knife ID", CHAT_PREFIX);
        FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
        META_CONPRINTF("TestSkinchanger: Unknown Weapon/Knife ID\n");
        return;
    }

	
    CBasePlayerWeapon* pPlayerWeapon = pWeaponServices->m_hActiveWeapon();

    if (!pPlayerWeapon) {
        META_CONPRINTF("TestSkinchanger: No active weapon\n");
        return;
    }

    const auto pPlayerWeapons = pWeaponServices->m_hMyWeapons();
    auto weapon_slot_map = g_ItemToSlotMap.find(weapon_id);

    if (weapon_slot_map == g_ItemToSlotMap.end()) {
        sprintf(buf, "%s\x02 Unknown Weapon/Knife ID", CHAT_PREFIX);
        FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
        META_CONPRINTF("TestSkinchanger: Unknown Weapon/Knife ID\n");
        return;
    }

    auto weapon_slot = weapon_slot_map->second;

    for (size_t i = 0; i < pPlayerWeapons.m_size; i++) {
        auto currentWeapon = pPlayerWeapons.m_data[i];

        if (!currentWeapon)
            continue;

        auto weapon = static_cast<CEconEntity*>(currentWeapon.Get());

        if (!weapon)
            continue;

        auto weapon_slot_map_my_weapon = g_ItemToSlotMap.find(weapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());

        if (weapon_slot_map_my_weapon == g_ItemToSlotMap.end()) {
            continue;
        }

        auto weapon_slot_my_weapon = weapon_slot_map_my_weapon->second;

        if (weapon_slot == weapon_slot_my_weapon) {
            pWeaponServices->RemoveWeapon(static_cast<CBasePlayerWeapon*>(currentWeapon.Get()));
            META_CONPRINTF("TestSkinchanger: Removed weapon in slot %lld\n", weapon_slot);
        }
    }

	META_CONPRINTF("TestSkinchanger: Delete entity %s\n", weapon_name->second.c_str());//ТАЙМЕР ДЛЯ ТЕСТА
	new CTimer(0.05f, false, false, [pPlayerPawn, weapon_name]() {
        	META_CONPRINTF("TestSkinchanger: try  to give %s\n", weapon_name->second.c_str());
	});

    META_CONPRINTF("TestSkinchanger: Gave named item %s\n", weapon_name->second.c_str());
    }
    else
    {
	META_CONPRINTF("ANY PARAMS = 0 %s\n");
    }
}

void SkinChangerKnife(int64_t steamid)
{
	CCSPlayerController* pPlayerController = players[steamid].PC;
	CCSPlayerPawnBase* pPlayerPawn = players[steamid].PP;
        SC_CBaseEntity* pSCBaseEntity=players[steamid].PBE;
	int teamnum= pSCBaseEntity->m_iTeamNum();
	CPlayer_WeaponServices* pWeaponServices = pPlayerPawn->m_pWeaponServices();
	const auto pPlayerWeapons = pWeaponServices->m_hMyWeapons();
    std::map<int, std::vector<nlohmann::json>> Temp = players[steamid].PlayerSkins;

    int knife_id_API = -1;
    int side = -1;
    bool exitOuterLoop = false;  
for (const auto& entry : g_KnivesMap) {
    int knifeIdToFind = entry.first;
    META_CONPRINTF("knifeIdToFind %lld\n", knifeIdToFind);
    const std::string& knifeName = entry.second;
    auto& KnifeDataVector = Temp[knifeIdToFind];

    try {
        for (const auto& KnifeData : KnifeDataVector) {
            side = static_cast<int>(KnifeData["side"]);
            if (side == teamnum || side == 0) {
                knife_id_API = KnifeData["weapon_id"];
                META_CONPRINTF("knife_id_API %lld\n", knife_id_API);
                exitOuterLoop = true;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return;
    }

    if (exitOuterLoop) {
        break;
    }
}

    if (knife_id_API < 0) {
        return;
    }
auto weapon_slot_map = g_ItemToSlotMap.find(knife_id_API);
auto weapon_name = g_KnivesMap.find(knife_id_API);
auto weapon_slot = weapon_slot_map->second;
	

for (size_t i = 0; i < pPlayerWeapons.m_size; i++) {
	auto currentWeapon = pPlayerWeapons.m_data[i];

	if (!currentWeapon)
		continue;

	auto weapon = static_cast<CEconEntity*>(currentWeapon.Get());

	if (!weapon)
		continue;

	auto weapon_slot_map_my_weapon = g_ItemToSlotMap.find(weapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());

	if (weapon_slot_map_my_weapon == g_ItemToSlotMap.end()) {
		continue;
	}

	auto weapon_slot_my_weapon = weapon_slot_map_my_weapon->second;

	if (weapon_slot == weapon_slot_my_weapon) {
		pWeaponServices->RemoveWeapon(static_cast<CBasePlayerWeapon*>(currentWeapon.Get()));
			FnEntityRemove(g_pGameEntitySystem, static_cast<CBasePlayerWeapon*>(currentWeapon.Get()), nullptr, -1);
			
	}
}

META_CONPRINTF("TestSkinchanger: Delete entity %s\n", weapon_name->second.c_str());//ТАЙМЕР ДЛЯ ТЕСТА
	META_CONPRINTF("TestSkinchanger: try  to give %s\n", weapon_name->second.c_str());
	FnGiveNamedItem(pPlayerPawn->m_pItemServices(), weapon_name->second.c_str(), nullptr, nullptr, nullptr, nullptr);
META_CONPRINTF("TestSkinchanger: Gave named item %s\n", weapon_name->second.c_str());
}


void ForceUpdate(int64_t steamid)//makework(at time no work)
{
    std::map<int, CEntityInstance*> TempWeapons = players[steamid].PlayerWeapons;
    uint64_t newItemID = 16384;
    uint32_t newItemIDLow = newItemID & 0xFFFFFFFF;
    uint32_t newItemIDHigh = newItemID >> 32;

    std::map<int, std::vector<nlohmann::json>> Temp = players[steamid].PlayerSkins;

    for (const auto& entry : TempWeapons) {
        int weapon_id = entry.first;
        META_CONPRINTF("FORCE UPDATE WEAPON_ID %lld\n", weapon_id);

        try {
            auto it = Temp.find(weapon_id);
            if (it != Temp.end()) {
                std::vector<nlohmann::json>& weaponDataList = it->second;

                CEntityInstance* pEntity = entry.second;
                CBasePlayerWeapon* pBasePlayerWeapon = dynamic_cast<CBasePlayerWeapon*>(pEntity);
                CEconEntity* pCEconEntityWeapon = dynamic_cast<CEconEntity*>(pEntity);

                g_Skin.NextFrame([=]() {
                    if (!pBasePlayerWeapon) return;

                    bool Loop = true;
                    int local_skin_id = -1;
                    float local_skin_float = -1.0f;
                    int local_seed = -1;
                    int local_weapon_id = -1;

                    for (const auto& weaponData : weaponDataList) {
                        if (Loop) {
                            local_skin_id = weaponData["skin_id"];
                            local_skin_float = weaponData["float"];
                            local_seed = weaponData["seed"];
                            local_weapon_id = weaponData["weapon_id"];
			    META_CONPRINTF("local_skin_id %lld\n", local_skin_id);
			    META_CONPRINTF("local_skin_float %f\n", local_skin_float);
			    META_CONPRINTF("local_seed %lld\n", local_skin_id);
			    META_CONPRINTF("local_skin_id %lld\n", local_weapon_id);
                             if (local_skin_id > 0 && local_skin_float > 0.0f && local_seed > 0 && local_weapon_id > 0) {
                                (dynamic_cast<CEconEntity*>(pEntity))->m_AttributeManager().m_Item().m_iItemDefinitionIndex() = local_weapon_id;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_AttributeManager().m_Item().m_iItemIDLow() = newItemIDLow;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_AttributeManager().m_Item().m_iItemIDHigh() = newItemIDHigh;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_AttributeManager().m_Item().m_iItemID() = newItemID;


				(dynamic_cast<CEconEntity*>(pEntity))->m_nFallbackPaintKit() = 0;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_nFallbackSeed() = 0;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_flFallbackWear() = 0.0f;
				     
                                (dynamic_cast<CEconEntity*>(pEntity))->m_nFallbackPaintKit() = local_skin_id;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_nFallbackSeed() = local_seed;
                                (dynamic_cast<CEconEntity*>(pEntity))->m_flFallbackWear() = local_skin_float;
                                (dynamic_cast<CBasePlayerWeapon*>(pEntity))->m_CBodyComponent()->m_pSceneNode()->GetSkeletonInstance()->m_modelState().m_MeshGroupMask() = 2;
                             }

                            META_CONPRINTF("FOUND TEAMNUM AND SIDE %lld\n", local_weapon_id);
                        }
                    }
                });
            }
        } catch (const std::exception& e) {
            META_CONPRINTF("ERROR %s\n", e.what());
        }
    }
}

//TEST END

void ThreadUpdate(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, SC_CBaseEntity* pbe)
{
	try
	{
		while(players[steamid].firstspawn)
		{
			if(players[steamid].PC==nullptr)
			{
				break;
			}
			
		AddOrUpdatePlayer(steamid,pc,pp,GETSKINS(steamid),pbe);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));//api request 1sec/200ms=5 request per minut
		
			
		//META_CONPRINTF("UPDATESKINS SUCCESS %lld\n", players[steamid].PC);//debug
    		}
		 ClearPlayer(steamid);
		META_CONPRINTF("UPDATESKINS THREAD DELETE %lld\n");
	}
	catch(const std::exception& e)
	{
	}
}

//TEST ADDMAP
void AddOrUpdatePlayer(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, std::map<int, std::vector<nlohmann::json>> skins, SC_CBaseEntity* pbe)
{
	//Players player;
	if(players[steamid].PC==nullptr&& !state[steamid])
	{	Players player;
		player.firstspawn=false;
	 	player.PC = pc;
    		player.PP = pp;
	        player.PBE = pbe;
    		player.PlayerSkins = skins;
    		//player->firstspawn=true;
    		players[steamid] = player;
	}
	else
	{
		if(!players[steamid].firstspawn)
		{
			Players player;
	 		player.PC = pc;
    			player.PP = pp;
			player.PBE = pbe;
    			player.PlayerSkins = skins;
    			//player->firstspawn=true;
    			players[steamid] = player;
		}
		else
		{
		 players[steamid].firstspawn=true;
		 players[steamid].PC=pc;
		 players[steamid].PP=pp;
		 players[steamid].PBE=pbe;
			if(players[steamid].PlayerSkins!=skins)
			{
				players[steamid].PlayerSkins=skins;
				//ForceUpdate(steamid);//nowork
				SkinChangerKnife(steamid);
				
			}
			else
			{
				players[steamid].PlayerSkins=skins;
			}
		 
		}
	}
}

void ClearPlayer(int64_t steamid) {

    auto it = players.find(steamid);
    if (it != players.end()) {
        META_CONPRINTF("CLEAR STRUCT\n");
        players[steamid].PC = nullptr;
        players[steamid].PP = nullptr;
	players[steamid].PBE = nullptr;
        players[steamid].PlayerSkins.clear();
        players[steamid].firstspawn = false;
    }
}
//TEST END

//TEST FUNC GETSKINS

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total_size = size * nmemb;
	std::string* response = static_cast<std::string*>(userp);
	response->append(static_cast<char*>(contents), total_size);
	return total_size;
}

std::map<int, std::vector<nlohmann::json>> GETSKINS(int64_t steamid64) {
	CURL* curl;
	CURLcode res;

	std::string steamid = std::to_string(steamid64);
	nlohmann::json jsonResponse;
	std::map<int, std::vector<nlohmann::json>> TempSkins;
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		std::string response;

		std::string url = "https://api.cstrigon.net/api/v1/get_skins/" + steamid;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);

		if (res == CURLE_OK) {
			try {
				jsonResponse = nlohmann::json::parse(response);

				for(const auto& skin:jsonResponse)
				{
    					int weapon_id = skin["weapon_id"];
    					TempSkins[weapon_id].push_back(skin);
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Ошибка при парсинге JSON: " << e.what() << std::endl;
			}
		}
		else {}
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return TempSkins;
}

//TEST END


CON_COMMAND_F(skin, "modify skin", FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1) {
		return;
	}
    CCSPlayerController* pPlayerController =(CCSPlayerController*)g_pEntitySystem->GetBaseEntity((CEntityIndex)(context.GetPlayerSlot().Get() + 1));
    CCSPlayerPawnBase* pPlayerPawn = pPlayerController->m_hPlayerPawn();
    if (!pPlayerPawn || pPlayerPawn->m_lifeState() != LIFE_ALIVE) {
		return;
	}

	//TEST
	META_CONPRINTF("CCSPlayerController %lld\n", pPlayerController);
	META_CONPRINTF("CCSPlayerPawnBase %lld\n", pPlayerPawn);
	//TEST
    char buf[255] = { 0 };
    if (args.ArgC() != 5)
    {
        char buf2[255] = { 0 };
		sprintf(buf, "%s\x02 Wrong usage!", CHAT_PREFIX);
		sprintf(buf2, "%s Console command: \x06skin \x04ItemDefIndex PaintKit PatternID Float\x01", CHAT_PREFIX);
		FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
		FnUTIL_ClientPrint(pPlayerController, 3, buf2, nullptr, nullptr, nullptr, nullptr);
        return;
    }

	int32_t weapon_id = atoi(args.Arg(1));
	int64_t paint_kit = atoi(args.Arg(2));
	int64_t pattern_id = atoi(args.Arg(3));
	float wear = atof(args.Arg(4));

	META_CONPRINTF("Weapon id %lld\n", weapon_id);
	META_CONPRINTF("paint_kit %lld\n", paint_kit);
	META_CONPRINTF("pattern_id %lld\n", pattern_id);
	META_CONPRINTF("wear %lld\n", wear);
	
    auto weapon_name = g_WeaponsMap.find(weapon_id);
	bool isKnife = false;
	int64_t steamid = pPlayerController->m_steamID();
    CPlayer_WeaponServices* pWeaponServices = pPlayerPawn->m_pWeaponServices();

	if (weapon_name == g_WeaponsMap.end()) {
		weapon_name = g_KnivesMap.find(weapon_id);
		isKnife = true;
	}

	if (weapon_name == g_KnivesMap.end()) {
		sprintf(buf, "%s\x02 Unknown Weapon/Knife ID", CHAT_PREFIX);
		FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
		return;
	}

	g_PlayerSkins[steamid].m_iItemDefinitionIndex = weapon_id;
	g_PlayerSkins[steamid].m_nFallbackPaintKit = paint_kit;
	g_PlayerSkins[steamid].m_nFallbackSeed = pattern_id;
	g_PlayerSkins[steamid].m_flFallbackWear = wear;
    CBasePlayerWeapon* pPlayerWeapon = pWeaponServices->m_hActiveWeapon();
	const auto pPlayerWeapons = pWeaponServices->m_hMyWeapons();
	
	META_CONPRINTF("pPlayerWeapont %lld\n\n", pPlayerWeapon);
	META_CONPRINTF("pPlayerWeapons %lld\n\n", pPlayerWeapons);
	
	auto weapon_slot_map = g_ItemToSlotMap.find(weapon_id);
	if (weapon_slot_map == g_ItemToSlotMap.end()) {
		sprintf(buf, "%s\x02 Unknown Weapon/Knife ID", CHAT_PREFIX);
		FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	META_CONPRINTF("WeaponSlot %lld\n", weapon_slot_map);
	
	auto weapon_slot = weapon_slot_map->second;
	
	META_CONPRINTF("WeaponSlot %lld\n", weapon_slot);
	
	for (size_t i = 0; i < pPlayerWeapons.m_size; i++)
	{
		auto currentWeapon = pPlayerWeapons.m_data[i];
		if (!currentWeapon)
			continue;
		auto weapon = static_cast<CEconEntity*>(currentWeapon.Get());
		if (!weapon)
			continue;
		auto weapon_slot_map_my_weapon = g_ItemToSlotMap.find(weapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex());
		if (weapon_slot_map_my_weapon == g_ItemToSlotMap.end()) {
			continue;
		}
		auto weapon_slot_my_weapon = weapon_slot_map_my_weapon->second;
		if (weapon_slot == weapon_slot_my_weapon) {
			pWeaponServices->RemoveWeapon(static_cast<CBasePlayerWeapon*>(currentWeapon.Get()));
			FnEntityRemove(g_pGameEntitySystem, static_cast<CBasePlayerWeapon*>(currentWeapon.Get()), nullptr, -1);
		}
	}
	FnGiveNamedItem(pPlayerPawn->m_pItemServices(), weapon_name->second.c_str(), nullptr, nullptr, nullptr, nullptr);
	// pPlayerWeapon->m_AttributeManager().m_Item().m_iAccountID() = 9727743;
    // FnGiveNamedItem(pPlayerPawn->m_pItemServices(), weapon_name->second.c_str(), nullptr, nullptr, nullptr, nullptr);
    // pWeaponServices->m_hActiveWeapon()->m_AttributeManager().m_Item().m_iAccountID() = 9727743;
    META_CONPRINTF("called by %lld\n", steamid);
    //sprintf(buf, "%s\x04 Success!\x01 ItemDefIndex:\x04 %d\x01 PaintKit:\x04 %d\x01 PatternID:\x04 %d\x01 Float:\x04 %f\x01", CHAT_PREFIX, g_PlayerSkins[steamid].m_iItemDefinitionIndex, g_PlayerSkins[steamid].m_nFallbackPaintKit, g_PlayerSkins[steamid].m_nFallbackSeed, g_PlayerSkins[steamid].m_flFallbackWear);
	//FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
}

CON_COMMAND_F(test, "test", FCVAR_CLIENT_CAN_EXECUTE) {
	new CTimer(10.0f, false, false, []() {
        char buf[255] = { 0 };
		//sprintf(buf, "%s Timer executed", CHAT_PREFIX);
		//FnUTIL_ClientPrintAll(3, buf,nullptr, nullptr, nullptr, nullptr);
	});
	char buf[255] = { 0 };
	//sprintf(buf, "%s Timer started", CHAT_PREFIX);
	//FnUTIL_ClientPrintAll(3, buf,nullptr, nullptr, nullptr, nullptr);
}


CON_COMMAND_F(i_subclass_change, "subclass change", FCVAR_NONE)
{
	FnSubClassChange(context,args);
}

const char* Skin::GetLicense()
{
	return "GPL";
}

const char* Skin::GetVersion()
{
	return "1.0.0";
}

const char* Skin::GetDate()
{
	return __DATE__;
}

const char* Skin::GetLogTag()
{
	return "skin";
}

const char* Skin::GetAuthor()
{
	return "Monsoon X tiltedboyxddd";
}

const char* Skin::GetDescription()
{
	return "Private Skinchanger for cstrigon.net";
}

const char* Skin::GetName()
{
	return "Skinchanger";
}

const char* Skin::GetURL()
{
	return "cstrigon.net";
}
