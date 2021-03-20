#include "RocketLink.h"
#include "json.hpp"


void RocketLink::StartWsServer()
{
	cvarManager->log("[RLINK] Starting WebSocket server");
	ws_connections = new ConnectionSet();
	ws_server = new PluginServer();

	cvarManager->log("[RLINK] Starting asio");
	ws_server->init_asio();
	ws_server->set_open_handler(websocketpp::lib::bind(&RocketLink::OnWsOpen, this, _1));
	ws_server->set_close_handler(websocketpp::lib::bind(&RocketLink::OnWsClose, this, _1));
	ws_server->set_http_handler(websocketpp::lib::bind(&RocketLink::OnHttpRequest, this, _1));

	cvarManager->log("[RLINK] Starting listen on port " + std::to_string(*cvarPort.get()));
	ws_server->listen(*cvarPort);

	ws_server->start_accept();
	ws_server->run();

}

void RocketLink::StopWsServer()
{
	if (ws_server)
	{
		ws_server->stop();
		ws_server->stop_listening();
		delete ws_server;
		ws_server = nullptr;
	}

	if (ws_connections)
	{
		ws_connections->clear();
		delete ws_connections;
		ws_connections = nullptr;
	}
}


void RocketLink::OnHttpRequest(websocketpp::connection_hdl hdl)
{
	PluginServer::connection_ptr connection = ws_server->get_con_from_hdl(hdl);
	connection->append_header("Content-Type", "application/json");
	connection->append_header("Server", "RocketLink/1.0.0");

	if (connection->get_resource() == "/init")
	{
		json response;
		response["status"] = "ok";
		connection->set_body(response.dump());
		connection->set_status(websocketpp::http::status_code::ok);
		return;
	}

	connection->set_body("Not found");
	connection->set_status(websocketpp::http::status_code::not_found);
}


void RocketLink::WsSend(const json& jayson) {
	std::string out = jayson.dump();

	try 
	{
		
		out = websocketpp::base64_encode((const unsigned char*)out.c_str(), (unsigned int)out.size());
		
		for (const connection_hdl& it : *ws_connections) {
			ws_server->send(it, out, websocketpp::frame::opcode::text);
		}


	}
	catch (std::exception e)
	{
		cvarManager->log("Error sending websocket message: " + std::string(e.what()));
	}

}