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
#include "sdk/CSmokeGrenadeProjectile.h"
#include <map>
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
Event_ItemPurchase g_PlayerBuy;
Event_PlayerSpawned g_PlayerSpawnedEvent;//nowork tested
OnRoundStart g_RoundStart;

Event_PlayerConnect g_PlayerConnect;
Event_PlayerDisconnect g_PlayerDisconnect;

void TestSkinchanger(int64_t arg1, int arg2);
std::map<int, nlohmann::json> GETSKINS(int64_t steamid64);
void AddOrUpdatePlayer(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, std::map<int, nlohmann::json> skins);

void ThreadUpdate(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp);
bool firstPlayerSpawnEvent=true;

CCSPlayerController* PC;
CCSPlayerPawnBase* PP;

struct Players
{
    CCSPlayerController* PC;
    CCSPlayerPawnBase* PP;
    //nlohmann::json SKINS;
    std::map<int, nlohmann::json> PlayerSkins;
    bool firstspawn=true;
};

std::map<int64_t, std::shared_ptr<Players>> players;//TEST DYNAMIC MASSIVE

std::map<std::string, int> SearchMap;
//TEST//////

CEntityListener g_EntityListener;
bool g_bPistolRound;

float g_flUniversalTime;
float g_flLastTickedTime;
bool g_bHasTicked;

#define CHAT_PREFIX	""

typedef struct SkinParm
{
	int32_t m_iItemDefinitionIndex;
	int m_nFallbackPaintKit;
	int m_nFallbackSeed;
	float m_flFallbackWear;
	bool used = false;
}SkinParm;



#ifdef _WIN32
typedef void*(FASTCALL* SubClassChange_t)(const CCommandContext &context, const CCommand &args);
typedef void*(FASTCALL* EntityRemove_t)(CGameEntitySystem*, void*, void*,uint64_t);
typedef void(FASTCALL* GiveNamedItem_t)(void* itemService,const char* pchName, void* iSubType,void* pScriptItem, void* a5,void* a6);
typedef void(FASTCALL* UTIL_ClientPrintAll_t)(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4);
typedef void(FASTCALL *ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);

extern SubClassChange_t FnSubClassChange;
extern EntityRemove_t FnEntityRemove;
extern GiveNamedItem_t FnGiveNamedItem;
extern UTIL_ClientPrintAll_t FnUTIL_ClientPrintAll;
extern ClientPrint_t FnUTIL_ClientPrint;


EntityRemove_t FnEntityRemove;
GiveNamedItem_t FnGiveNamedItem;
UTIL_ClientPrintAll_t FnUTIL_ClientPrintAll;
ClientPrint_t FnUTIL_ClientPrint;
SubClassChange_t FnSubClassChange;

#else
void (*FnEntityRemove)(CGameEntitySystem*, void*, void*,uint64_t) = nullptr;
void (*FnGiveNamedItem)(void* itemService,const char* pchName, void* iSubType,void* pScriptItem, void* a5,void* a6) = nullptr;
void (*FnUTIL_ClientPrintAll)(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4) = nullptr;
void (*FnUTIL_ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
void (*FnSubClassChange)(const CCommandContext &context, const CCommand &args) = nullptr;
#endif

std::map<int, std::string> g_WeaponsMap;
std::map<int, std::string> g_KnivesMap;
std::map<int, int> g_ItemToSlotMap;
std::map<uint64_t, SkinParm> g_PlayerSkins;
std::map<uint64_t, int> g_PlayerMessages;
uint32_t g_iItemIDHigh = 16384;

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
	// g_KnivesMap = { {59,"weapon_knife"},{42,"weapon_knife"},{526,"weapon_knife_kukri"},{508,"weapon_knife_m9_bayonet"},{500,"weapon_bayonet"},{514,"weapon_knife_survival_bowie"},{515,"weapon_knife_butterfly"},{512,"weapon_knife_falchion"},{505,"weapon_knife_flip"},{506,"weapon_knife_gut"},{509,"weapon_knife_tactical"},{516,"weapon_knife_push"},{520,"weapon_knife_gypsy_jackknife"},{522,"weapon_knife_stiletto"},{523,"weapon_knife_widowmaker"},{519,"weapon_knife_ursus"},{503,"weapon_knife_css"},{517,"weapon_knife_cord"},{518,"weapon_knife_canis"},{521,"weapon_knife_outdoor"},{525,"weapon_knife_skeleton"},{507,"weapon_knife_karambit"} };
	g_KnivesMap = { {59,"weapon_knife"},{42,"weapon_knife"},{526,"weapon_knife"},{508,"weapon_knife"},{500,"weapon_knife"},{514,"weapon_knife"},{515,"weapon_knife"},{512,"weapon_knife"},{505,"weapon_knife"},{506,"weapon_knife"},{509,"weapon_knife"},{516,"weapon_knife"},{520,"weapon_knife"},{522,"weapon_knife"},{523,"weapon_knife"},{519,"weapon_knife"},{503,"weapon_knife"},{517,"weapon_knife"},{518,"weapon_knife"},{521,"weapon_knife"},{525,"weapon_knife"},{507,"weapon_knife"} };
	g_ItemToSlotMap = { {59, 0},{42, 0},{526, 0},{508, 0},{500, 0},{514, 0},{515, 0},{512, 0},{505, 0},{506, 0},{509, 0},{516, 0},{520, 0},{522, 0},{523, 0},{519, 0},{503, 0},{517, 0},{518, 0},{521, 0},{525, 0},{507, 0}, {42, 0}, {59, 0}, {32, 1}, {61, 1}, {1, 1}, {3, 1}, {2, 1}, {36, 1}, {63, 1}, {64, 1}, {30, 1}, {4, 1}, {14, 2}, {17, 2}, {23, 2}, {33, 2}, {28, 2}, {35, 2}, {19, 2}, {26, 2}, {29, 2}, {24, 2}, {25, 2}, {27, 2}, {34, 2}, {8, 3}, {9, 3}, {10, 3}, {60, 3}, {16, 3}, {38, 3}, {40, 3}, {7, 3}, {11, 3}, {13, 3}, {39, 3} };

	//Test MAP
	SearchMap = {{"weapon_bizon", 26},{"weapon_mac10", 17},{"weapon_mp9", 34},{"weapon_p90", 19},{"weapon_ump45", 24},{"weapon_ak47", 7},{"weapon_aug", 8},{"weapon_famas", 10},{"weapon_galilar", 13},{"weapon_m4a1", 16},{"weapon_m4a1_silencer", 60},{"weapon_sg556", 39},{"weapon_awp", 9},{"weapon_g3sg1", 11},{"weapon_scar20", 38},{"weapon_ssg08", 40},{"weapon_mag7", 27},{"weapon_nova", 35},{"weapon_sawedoff", 29},{"weapon_xm1014", 25},{"weapon_m249", 14},{"weapon_negev", 28},{"weapon_deagle", 1},{"weapon_elite", 2},{"weapon_fiveseven", 3},{"weapon_glock", 4},{"weapon_hkp2000", 32},{"weapon_p250", 36},{"weapon_tec9", 30},{"weapon_usp_silencer", 61},{"weapon_cz75a", 63},{"weapon_revolver", 64},{"weapon_mp5sd", 23},{"weapon_mp7", 33}
};
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
	#ifdef _WIN32
	FnUTIL_ClientPrintAll = (UTIL_ClientPrintAll_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18\x57\x48\x81\xEC\x70\x01\x3F\x3F\x8B\xE9");
	FnGiveNamedItem = (GiveNamedItem_t)FindSignature("server.dll", "\x48\x89\x5C\x24\x18\x48\x89\x74\x24\x20\x55\x57\x41\x54\x41\x56\x41\x57\x48\x8D\x6C\x24\xD9");
	FnEntityRemove = (EntityRemove_t)FindSignature("server.dll", "\x48\x85\xD2\x0F\x3F\x3F\x3F\x3F\x3F\x57\x48\x3F\x3F\x3F\x48\x89\x3F\x3F\x3F\x48\x8B\xF9\x48\x8B");
	FnSubClassChange = (SubClassChange_t)FindSignature("server.dll", "\x40\x55\x41\x57\x48\x83\xEC\x78\x83\xBA\x38\x04");
	#else
	CModule libserver(g_pSource2Server);
	FnUTIL_ClientPrintAll = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 D7 41 56 49 89 F6 41 55 41 89 FD").RCast< decltype(FnUTIL_ClientPrintAll) >();
	FnGiveNamedItem = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 49 89 CE 41 55 49 89 F5 41 54 49 89 D4 53 48 89").RCast<decltype(FnGiveNamedItem)>();
	FnEntityRemove = libserver.FindPatternSIMD("48 85 F6 74 0B 48 8B 76 10 E9 B2 FE FF FF").RCast<decltype(FnEntityRemove)>();
	FnUTIL_ClientPrint = libserver.FindPatternSIMD("55 48 89 E5 41 57 49 89 CF 41 56 49 89 D6 41 55 41 89 F5 41 54 4C 8D A5 A0 FE FF FF").RCast<decltype(FnUTIL_ClientPrint)>();
	FnSubClassChange = libserver.FindPatternSIMD("55 48 89 E5 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 83 BE 38 04 00 00 01 0F 8E 47 02").RCast<decltype(FnSubClassChange)>();
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
	//if (!firstPlayerSpawnEvent)
	//{
		
        	//return;
		
	//}

	//firstPlayerSpawnEvent=false;
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
    		
     		//PC=pCSPlayerController;//globalCONTROLLER
		int64_t steamid = pPlayerController->m_steamID();
		if (pCSPlayerController)
		{
    			CCSPlayerPawnBase* playerPawn = pCSPlayerController->m_hPlayerPawn();
    			if (playerPawn)
			{
				//PP=playerPawn;//globalPAWN
				//nlohmann::json jsonSkins=GETSKINS(steamid);
				if (players.find(steamid) != players.end()) 
				{
    				// Игрок существует в вашем контейнере

    					if (!players[steamid]->firstspawn) 
					{
        					return;
    					} 
					else 
					{

    					}
				} 
				else 
				{
    				// Игрок не существует в вашем контейнере, возможно, нужно выполнить какие-то действия
					//std::map<int, nlohmann::json> Temp = GETSKINS(steamid);
       					//AddOrUpdatePlayer(steamid, pCSPlayerController, playerPawn, Temp);
					AddOrUpdatePlayer(steamid,pc,pp,GETSKINS(steamid));
					std::thread([pCSPlayerController, playerPawn, steamid]() {
        						ThreadUpdate(steamid,pCSPlayerController,playerPawn);
							//std::this_thread::sleep_for(std::chrono::milliseconds(150));
			
							//TestSkinchanger(steamid, ids);
			
						}).detach();
					//return;
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




void Event_ItemPurchase::FireGameEvent(IGameEvent* event)
{
	//const 
	const std::string weapon = event->GetString("weapon");
	const int userId = event->GetInt("userid");
	CBasePlayerController* pPlayerController = static_cast<CBasePlayerController*>(event->GetPlayerController("userid"));
	g_Skin.NextFrame([hPlayerController = CHandle<CBasePlayerController>(pPlayerController), pPlayerController = pPlayerController,weapon=weapon]()
	{
	
		CCSPlayerController* pCSPlayerController = dynamic_cast<CCSPlayerController*>(pPlayerController);
    		CCSPlayerPawnBase* playerPawn = pCSPlayerController->m_hPlayerPawn();
		//7 639 1 0
		//[{"skin_id":724,"float":0.061400000000000003186340080674199271015822887420654296875,"seed":245,"nametag":"","side":1,"stickers":[],"stattrak":false,"weapon_id":7,"stattrak_count":0}]

		int ids=SearchMap[weapon];
		std::thread([pCSPlayerController, playerPawn, ids=ids]() {
        		int64_t steamid = pCSPlayerController->m_steamID();
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
			
			//TestSkinchanger(steamid, ids);
			
		}).detach();
	
		META_CONPRINTF("PLAYER BUY WEAPON %d\n",ids);
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
	//TestSkinchanger(PC, PP, 61, 657, 1, 0.0f);
	//firstPlayerSpawnEvent=true;
    	META_CONPRINTF("RoundStarted\n");
}


//Event_PlayerConnect g_PlayerConnect;
//Event_PlayerDisconnect g_PlayerDisconnect;
uint64_t ExtractSteamIDFromNetworkID(const std::string& networkID) {
try {
        size_t start = networkID.find(":1:") + 3;
        size_t end = networkID.find("]", start);

        if (start != std::string::npos && end != std::string::npos) {
            std::string accountIDStr = networkID.substr(start, end - start);
            uint32_t accountID = std::stoi(accountIDStr);
            uint64_t steamID = ((uint64_t)accountID) + 76561197960265728ULL;
            return steamID;
        } else {
            // Логгирование или вывод сообщения об ошибке
            return 0;
        }
    } catch (const std::exception& e) {
        // Обработка ошибок в блоке try
        // Логгирование или вывод сообщения об ошибке
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
	    // Это бот, выполните соответствующие действия
    			return;
		}
        uint64_t steamid = ExtractSteamIDFromNetworkID(netid);

        META_CONPRINTF("Player Disconnected: , SteamID: %llu\n", steamid);
    } catch (const std::exception& e) {
        // Обработка ошибок
    }
}

void Event_PlayerDisconnect::FireGameEvent(IGameEvent* event) {
    try {
        bool isBot = event->GetBool("bot");
        if (isBot) {
            return;
        }

        std::string netid = event->GetString("networkid");
	if (netid == "BOT")
	{
	    // Это бот, выполните соответствующие действия
    	return;
	}
        uint64_t steamid = ExtractSteamIDFromNetworkID(netid);
	players[steamid]->firstspawn=false;
	    auto it = players.find(steamid);
    	    if (it != players.end())
    		{
			META_CONPRINTF("ERASE STRUCT\n");
			players[steamid]->firstspawn=false;
       			players.erase(it);
		}
        META_CONPRINTF("Player Disconnected: , SteamID: %llu\n", steamid);
    } catch (const std::exception& e) {
        // Обработка ошибок
    }
}

//META_CONPRINTF("PLAYER BUY WEAPON\n");
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
	g_Skin.NextFrame([pBasePlayerWeapon = pBasePlayerWeapon, pCEconEntityWeapon = pCEconEntityWeapon]()
	{
		int64_t steamid = pCEconEntityWeapon->m_OriginalOwnerXuidLow() | (static_cast<int64_t>(pCEconEntityWeapon->m_OriginalOwnerXuidHigh()) << 32);
		int64_t weaponId = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex();
		
		META_CONPRINTF( "----------------------------------------------------\n");
		if(!steamid) {
			return;
		}

		if(weaponId==503)
		{
			return;
		}
		
		TestSkinchanger(steamid, weaponId);
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		auto skin_parm = g_PlayerSkins.find(steamid);
		if(skin_parm == g_PlayerSkins.end()) {
			//return;
		}

		if(skin_parm->second.m_iItemDefinitionIndex == -1 || skin_parm->second.m_nFallbackPaintKit == -1 || skin_parm->second.m_nFallbackSeed == -1 || skin_parm->second.m_flFallbackWear == -1) {
			//return;
		}

		uint64_t temp_itemID = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID();
		uint32_t temp_itemIDLow = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDLow();
		uint32_t temp_itemIDHigh = pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDHigh();

		// Combine the itemIDLow and itemIDHigh
		uint64_t itemID2 = temp_itemIDLow | (static_cast<uint64_t>(temp_itemIDHigh) << 32);

		// print out all 4 values
		META_CONPRINTF("temp_itemID: %d\n", temp_itemID);
		META_CONPRINTF("temp_itemIDLow: %d\n", temp_itemIDLow);
		META_CONPRINTF("temp_itemIDHigh: %d\n", temp_itemIDHigh);
		META_CONPRINTF("itemID2: %d\n", itemID2);

		
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemDefinitionIndex() = skin_parm->second.m_iItemDefinitionIndex;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDLow() = g_iItemIDHigh;
		pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemIDHigh() = -1;
		// pCEconEntityWeapon->m_AttributeManager().m_Item().m_iItemID() = g_iItemIDHigh++;

		META_CONPRINTF("skin_parm->second.m_nFallbackPaintKit: %d\n", skin_parm->second.m_nFallbackPaintKit);
		META_CONPRINTF("skin_parm->second.m_nFallbackSeed: %d\n", skin_parm->second.m_nFallbackSeed);
		META_CONPRINTF("skin_parm->second.m_flFallbackWear: %f\n", skin_parm->second.m_flFallbackWear);
		META_CONPRINTF("skin_parm->second.m_iItemDefinitionIndex: %d\n", skin_parm->second.m_iItemDefinitionIndex);

		//[{"skin_id":724,"float":0.061400000000000003186340080674199271015822887420654296875,"seed":245,"nametag":"","side":1,"stickers":[],"stattrak":false,"weapon_id":7,"stattrak_count":0}]
		//ONLYTEST
		pCEconEntityWeapon->m_nFallbackPaintKit() = skin_parm->second.m_nFallbackPaintKit;
		pCEconEntityWeapon->m_nFallbackSeed() = skin_parm->second.m_nFallbackSeed;
		pCEconEntityWeapon->m_flFallbackWear() = skin_parm->second.m_flFallbackWear;
		//TEST
		
		// pCEconEntityWeapon->m_OriginalOwnerXuidLow() = -1;
		// pCEconEntityWeapon->m_OriginalOwnerXuidHigh() = -1;

		pBasePlayerWeapon->m_CBodyComponent()->m_pSceneNode()->GetSkeletonInstance()->m_modelState().m_MeshGroupMask() = 2;
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

		skin_parm->second.m_iItemDefinitionIndex = -1;
		skin_parm->second.m_nFallbackPaintKit = -1;
		skin_parm->second.m_nFallbackSeed = -1;
		skin_parm->second.m_flFallbackWear = -1;

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
    CCSPlayerController* pPlayerController=players[steamid]->PC;
    CCSPlayerPawnBase* pPlayerPawn=players[steamid]->PP;
	
    if (!pPlayerPawn || pPlayerPawn->m_lifeState() != LIFE_ALIVE || !pPlayerController) {
        META_CONPRINTF("TestSkinchanger: Invalid player or controller\n");
	    
        return;
    }

    char buf[255] = { 0 };

   // auto weapon_name = g_WeaponsMap.find(weapon_id);
    bool isKnife = false;
    //int64_t steamid = pPlayerController->m_steamID();
    META_CONPRINTF("STEAM IDIDIDIDIDIID %lld\n", steamid);
    META_CONPRINTF("Weapon IDIDIDIDIDIID %lld\n", weapon_id);
    //nlohmann::json jsonResponse=GETSKINS(steamid);
    //auto it=Temp.find(weapon_id);



   std::map<int, nlohmann::json> Temp=players[steamid]->PlayerSkins;
    //auto it=Temp.find(weapon_id);
    nlohmann::json jsonResponse = Temp[weapon_id];
    std::string jsonString = jsonResponse.dump();
	
	 sprintf(buf, "%s\x02 JSONSTR", jsonString.c_str());
    FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
	
	META_CONPRINTF("JSON %lld\n", jsonString.c_str());
	//TEST API STATE
	//nlohmann::json jsonResponse=GETSKINS(steamid);
	int skin_id = -1;
        float skin_float = -1.0f;
        int seed = -1;
        std::string nametag = "NULL";
        int side = -1;
        bool stattrak = false;
        int weapon_id_API = -1;
        int stattrak_count = -1;
	
   // for (const auto& entry : Temp[weapon_id]) {
        //skin_id = entry["skin_id"];
        //skin_float = entry["float"];
        //seed = entry["seed"];
       // nametag = entry["nametag"];
        //side = entry["side"];
        //stattrak = entry["stattrak"];
        //weapon_id_API = entry["weapon_id"];
        //stattrak_count = entry["stattrak_count"];
    //}
	try
	{
		auto it = Temp.find(weapon_id);
		if (it != Temp.end()) {
		nlohmann::json& weaponData = it->second; // Ссылка на json для удобства
		skin_id = weaponData["skin_id"];
		skin_float = weaponData["float"];
		seed = weaponData["seed"];
		nametag = weaponData["nametag"];
		side = weaponData["side"];
		stattrak = weaponData["stattrak"];
		weapon_id_API = weaponData["weapon_id"];
		stattrak_count = weaponData["stattrak_count"];
		} else {
		return;
		}
	}
	catch(const std::exception& e)
	{
		META_CONPRINTF("ERRROR %lld\n");
	}
	
    auto weapon_name = g_WeaponsMap.find(weapon_id_API);
    g_PlayerSkins[steamid].m_iItemDefinitionIndex = weapon_id_API;
    g_PlayerSkins[steamid].m_nFallbackPaintKit = skin_id;
    g_PlayerSkins[steamid].m_nFallbackSeed = seed;
    g_PlayerSkins[steamid].m_flFallbackWear = skin_float;
    
	
    //META_CONPRINTF("TestSkinchanger: Weapon id %lld\n", jsonString.c_str());

    //sprintf(buf, "%s\x02 JSONSTR", jsonString.c_str());
   //FnUTIL_ClientPrint(pPlayerController, 3, buf, nullptr, nullptr, nullptr, nullptr);
	
    //TEST END
//if(g_PlayerSkins[steamid].m_iItemDefinitionIndex != 0 && g_PlayerSkins[steamid].m_nFallbackPaintKit !=0 && g_PlayerSkins[steamid].m_nFallbackSeed != 0 &&  g_PlayerSkins[steamid].m_flFallbackWear !=0.0f)
//{
		
	
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

    //g_PlayerSkins[steamid].m_iItemDefinitionIndex = weapon_id;
    //g_PlayerSkins[steamid].m_nFallbackPaintKit = paint_kit;
    //g_PlayerSkins[steamid].m_nFallbackSeed = pattern_id;
   //g_PlayerSkins[steamid].m_flFallbackWear = wear;

	
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
            //FnEntityRemove(g_pGameEntitySystem, static_cast<CBasePlayerWeapon*>(currentWeapon.Get()), nullptr, -1);
            META_CONPRINTF("TestSkinchanger: Removed weapon in slot %lld\n", weapon_slot);
        }
    }
	
	META_CONPRINTF("TestSkinchanger: Delete entity %s\n", weapon_name->second.c_str());//ТАЙМЕР ДЛЯ ТЕСТА
	new CTimer(0.05f, false, false, [pPlayerPawn, weapon_name]() {
        	META_CONPRINTF("TestSkinchanger: try  to give %s\n", weapon_name->second.c_str());
		//FnGiveNamedItem(pPlayerPawn->m_pItemServices(), weapon_name->second.c_str(), nullptr, nullptr, nullptr, nullptr);
		//break;
	});
	//delete CTimer;
    META_CONPRINTF("TestSkinchanger: Gave named item %s\n", weapon_name->second.c_str());
    //}
    //else
    //{
	//META_CONPRINTF("ANY PARAMS = 0 %s\n");
    //}
}

//TEST END

void ThreadUpdate(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp)
{
	//AddOrUpdatePlayer(steamid,pc,pp,GETSKINS(steamid));
	try
	{
		//while (players.find(steamid) != players.end())
		while(players.find(steamid) != players.end())
		{
			//if(!players[steamid]->firstspawn)
			//{
				//break;
			//}
		//std::map<int, nlohmann::json> Temp=GETSKINS(steamid);
		AddOrUpdatePlayer(steamid,pc,pp,GETSKINS(steamid));
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		//CCSPlayerPawnBase* base= pc->m_hPlayerPawn();
		META_CONPRINTF("UPDATESKINS SUCCESS %lld\n");
		
			//if(!pp){
			//META_CONPRINTF("TestSkinchanger: Invalid player or controller\n");
			//players.erase(steamid);
        		//break;
			//}
    		}
		META_CONPRINTF("UPDATESKINS THREAD DELETE %lld\n");
	}
	catch(const std::exception& e)
	{
		META_CONPRINTF("PlayerDisconected\n");
	}
}

//TEST ADDMAP
void AddOrUpdatePlayer(int64_t steamid, CCSPlayerController* pc, CCSPlayerPawnBase* pp, std::map<int, nlohmann::json> skins)
{
    auto player = std::make_shared<Players>();
    player->PC = pc;
    player->PP = pp;
    player->PlayerSkins = skins;
    //player->firstspawn=false;
    players[steamid] = player;
}
//TEST END

//TEST FUNC GETSKINS

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total_size = size * nmemb;
	std::string* response = static_cast<std::string*>(userp);
	response->append(static_cast<char*>(contents), total_size);
	return total_size;
}

std::map<int, nlohmann::json> GETSKINS(int64_t steamid64) {
	CURL* curl;
	CURLcode res;

	std::string steamid = std::to_string(steamid64);
	nlohmann::json jsonResponse;
	std::map<int, nlohmann::json> TempSkins;
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
					int weapon_id=skin["weapon_id"];
					TempSkins[weapon_id]=skin;
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
    CCSPlayerController* pPlayerController = PC;//(CCSPlayerController*)g_pEntitySystem->GetBaseEntity((CEntityIndex)(context.GetPlayerSlot().Get() + 1)); тест глобального контроллера 
    CCSPlayerPawnBase* pPlayerPawn = PP;// pPlayerController->m_hPlayerPawn(); тест глобальной пешки(PAWN)
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
