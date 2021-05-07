#define _USE_MATH_DEFINES

#include "RocketLink.h"
#include <cmath>
#include "bakkesmod/wrappers/GameObject/Stats/StatEventWrapper.h"

BAKKESMOD_PLUGIN(RocketLink, "RocketLink", "1.0.1", PLUGINTYPE_THREADED)

void RocketLink::onLoad()
{
	cvarManager->log("[RLINK] Loading...");

	cvarEnabled = std::make_shared<bool>(false);
	CVarWrapper registeredEnabledCvar = cvarManager->registerCvar("RLINK_Enabled", "1", "Enable RocketLink plugin", true, true, 0, true, 1);
	registeredEnabledCvar.bindTo(cvarEnabled);

	cvarPort = std::make_shared<int>(49122);
	cvarManager->registerCvar("RLINK_Port", "49124", "WebSocket port for RocketLink client", true).bindTo(cvarPort);

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&RocketLink::OnViewportTick, this));

	StartWsServer();

}

void RocketLink::onUnload()
{
	StopWsServer();
	cvarManager->log("[RLINK] Unloaded");
}

void RocketLink::OnViewportTick()
{
	if (!*cvarEnabled) { return; }


	static steady_clock::time_point lastCallTime = steady_clock::now(); // this line only fires first time function is called
	float timeSinceLastCall = duration_cast<duration<float>>(steady_clock::now() - lastCallTime).count();
	if (timeSinceLastCall >= (100.f / 1000.f))
	{
		if (ShouldSendUpdates())
		{
			WsSend(GetCurrentPositioningData());
		}
		lastCallTime = steady_clock::now();
	}

}

bool RocketLink::ShouldSendUpdates()
{
	ServerWrapper server = this->GetCurrentGameState();
	return (!server.IsNull() && server.GetPlaylist().IsLanMatch()) && ws_connections->size() > 0 && !gameWrapper->GetCamera().IsNull();
}

json RocketLink::GetCurrentPositioningData()
{
	json data;
	data["players"] = json::array();

	ServerWrapper server = GetCurrentGameState();

	CameraWrapper cam = gameWrapper->GetCamera();
	Vector camLoc = cam.GetLocation();
	Rotator camRot = cam.GetCameraRotation();

	double camYaw = -(camRot.Yaw / 182.02222f) * (M_PI / 180.0f);
	double camPitch = -(camRot.Pitch / 90.77777f) * (M_PI / 180.0f);
	double camRoll = -(camRot.Roll / 182.02222f) * (M_PI / 180.0f);

	ArrayWrapper<PriWrapper> PRIs = server.GetPRIs();
	for (int i = 0; i < PRIs.Count(); ++i)
	{
		PriWrapper pri = PRIs.Get(i);
		if (pri.IsNull()) {continue;}

		std::string playerId = std::to_string(pri.GetUniqueIdWrapper().GetUID());
		std::string playerName = pri.GetPlayerName().IsNull() ? "" : pri.GetPlayerName().ToString();

		if (pri.IsLocalPlayerPRI())
		{
			data["self_id"] = playerId;
			data["self_name"] = playerName;
			if (!pri.IsSpectator()) 
			{ 
				data["self_team"] = pri.GetTeamNum();
			}
			else {
				//Use in client to set to spectator
				data["self_team"] = -1;
			}
			continue;
		}
		if (pri.IsSpectator() || pri.GetTeamNum() == 255) {continue;}

		CarWrapper car = pri.GetCar();
		if (car.IsNull() || car.GetbHidden()) {
			//No car or demoed -> Client sets gain to 0
			continue;
		}

		
		Vector carLocation = car.GetLocation();
		float dist = (carLocation - camLoc).magnitude();
		Vector diffVec = carLocation - camLoc;

		double newX = (diffVec.X * (cos(camYaw) * cos(camPitch))) + (diffVec.Y * (cos(camYaw) * sin(camPitch) * sin(camRoll) - sin(camYaw) * cos(camRoll))) + (diffVec.Z * (cos(camYaw) * sin(camPitch) * cos(camRoll) + sin(camYaw) * sin(camRoll)));
		double newY = (diffVec.X * (sin(camYaw) * cos(camPitch))) + (diffVec.Y * (sin(camYaw) * sin(camPitch) * sin(camRoll) + cos(camYaw) * cos(camRoll))) + (diffVec.Z * (sin(camYaw) * sin(camPitch) * cos(camRoll) - cos(camYaw) * sin(camRoll)));
		double newZ = (diffVec.X * (sin(camPitch))) + (diffVec.Y * (cos(camPitch) * sin(camRoll))) + (diffVec.Z * (cos(camPitch) * cos(camRoll)));

		json playerObject;
		playerObject["id"] = playerId;
		playerObject["name"] = playerName;
		playerObject["x"] = newX;
		playerObject["y"] = newY;
		playerObject["z"] = newZ;
		playerObject["distance"] = dist;
		playerObject["team"] = pri.GetTeamNum();

		data["players"].push_back(playerObject);
	}

	return data;
	

}


ServerWrapper RocketLink::GetCurrentGameState()
{
	if (gameWrapper->IsInReplay())
		return gameWrapper->GetGameEventAsReplay().memory_address;
	else if (gameWrapper->IsInOnlineGame())
		return gameWrapper->GetOnlineGame();
	else
		return gameWrapper->GetGameEventAsServer();
}
