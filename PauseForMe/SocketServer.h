#pragma once
#pragma warning(disable: 4267)

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <string>
#include <vector>
#include "CallBackHandler.h"

using namespace std;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

struct connection_data {
	int sessionid;
	std::string name;
};

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::map<websocketpp::connection_hdl, connection_data, std::owner_less<websocketpp::connection_hdl>> con_list;
typedef server::message_ptr message_ptr;

class SocketServer
{
public:
	SocketServer(int port);
	~SocketServer();
	void start();
	void stop();
	void broadcast(string message);
	void on_open(websocketpp::connection_hdl hdl);
	void on_close(websocketpp::connection_hdl hdl);
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
	connection_data& get_data_from_hdl(websocketpp::connection_hdl hdl);
	void setCallBack(CallBackHandler* callBack);
private:
	int port;
	server wsServer;
	int sessionId;
	con_list connections;
	vector<websocketpp::connection_hdl> arraySocketClients;
	CallBackHandler* callBackhandler;
};