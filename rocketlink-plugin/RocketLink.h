#pragma once
#pragma comment( lib, "pluginsdk.lib" )

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <set>
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include <json.hpp>

#include "bakkesmod/plugin/bakkesmodplugin.h"

// USINGS
using namespace std::chrono;
using namespace std::placeholders;
using websocketpp::connection_hdl;
using PluginServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionSet = std::set<connection_hdl, std::owner_less<connection_hdl>>;

// for convenience
using json = nlohmann::json;

class RocketLink : public BakkesMod::Plugin::BakkesModPlugin
{
public:
    void onLoad() override;
    void onUnload() override;

private:

    std::shared_ptr<bool> cvarEnabled;
    std::shared_ptr<int> cvarPort;

    json GetCurrentPositioningData();
    void OnViewportTick();
    bool ShouldSendUpdates();
    ServerWrapper GetCurrentGameState();

    //WS Stuff
    PluginServer* ws_server;
    ConnectionSet* ws_connections;

    void StartWsServer();
    void StopWsServer();

    void OnHttpRequest(websocketpp::connection_hdl hdl);
    void WsSend(const json &jayson);
    void OnWsOpen(connection_hdl hdl) { this->ws_connections->insert(hdl); }
    void OnWsClose(connection_hdl hdl) { this->ws_connections->erase(hdl); }

    

};

