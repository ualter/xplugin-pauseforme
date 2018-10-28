#include "SocketServer.h"
#include "XPLMUtilities.h"

SocketServer::SocketServer(int port) {
	this->port = port;
}

SocketServer::~SocketServer() {
	this->stop();
}

void SocketServer::broadcast(string message) {
	for (auto const& pair : this->connections) {
		websocketpp::connection_hdl connection = pair.first;

		try {
			this->wsServer.send(connection, message, websocketpp::frame::opcode::text);
		}
		catch (websocketpp::exception const & e) {
			std::string error = e.what();
			std::string msg = "SocketServer::broadcast() --> ERROR --> " + error;
			XPLMDebugString(msg.c_str());
		}
	}
}

void SocketServer::on_open(websocketpp::connection_hdl hdl) {
	connection_data data;

	data.sessionid = this->sessionId++;
	data.name = "Connection " + std::to_string(data.sessionid);

	this->connections[hdl] = data;
	this->arraySocketClients.push_back(hdl);
}

void SocketServer::on_close(websocketpp::connection_hdl hdl) {
	connection_data& data = get_data_from_hdl(hdl);

	this->connections.erase(hdl);
}

connection_data& SocketServer::get_data_from_hdl(websocketpp::connection_hdl hdl) {
	auto it = this->connections.find(hdl);

	if (it == this->connections.end()) {
		throw std::invalid_argument("No data avaliable for session");
	}

	return it->second;
}

void SocketServer::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
	std::string message = msg->get_payload();

	// check for a special command to instruct the server to stop listening so
	// it can be cleanly exited.
	if (msg->get_payload() == "<<STOP>>") {
		this->wsServer.stop_listening();
		return;
	}

	try {
		this->wsServer.send(hdl, msg->get_payload(), msg->get_opcode());
	}
	catch (websocketpp::exception const & e) {
		std::string error = e.what();
		std::string msg = "SocketServer::on_message() --> ERROR --> " + error;
		XPLMDebugString(msg.c_str());
	}
}

void SocketServer::start() {
	try {
		this->wsServer.set_access_channels(websocketpp::log::alevel::none);
		this->wsServer.clear_access_channels(websocketpp::log::alevel::none);

		this->wsServer.init_asio();
		this->wsServer.set_open_handler(bind(&SocketServer::on_open, this, ::_1));
		this->wsServer.set_message_handler(bind(&SocketServer::on_message, this, ::_1, ::_2));
		this->wsServer.set_close_handler(bind(&SocketServer::on_close, this, ::_1));
		this->wsServer.listen(this->port);

		XPLMDebugString(">>>");
		XPLMDebugString("SocketServer::start() --> Listening...");
		XPLMDebugString(">>>");
		
		this->wsServer.start_accept();
		this->wsServer.run();

	}
	catch (websocketpp::exception const & e) {
		std::string error = e.what();
		std::string msg = "SocketServer::start() --> ERROR --> " + error;
		XPLMDebugString(msg.c_str());
	}
	catch (...) {
		XPLMDebugString("SocketServer::start() --> ERROR!");
	}
}

void SocketServer::stop() {
	XPLMDebugString(">>>");
	XPLMDebugString("SocketServer::stop() -->  STOPPING" );
	XPLMDebugString(">>>");

	for (auto const& pair : this->connections) {
		websocketpp::connection_hdl connection = pair.first;

		try {
			this->wsServer.pause_reading(connection);
			this->wsServer.close(connection, websocketpp::close::status::going_away, "Server Stopped");
		}
		catch (websocketpp::exception const & e) {
			std::string error  = e.what();
			std::string msg    = "SocketServer::stop() --> ERROR --> " + error;
			XPLMDebugString(msg.c_str());
		}
	}

	this->wsServer.stop_listening();

	XPLMDebugString(">>>");
	XPLMDebugString("SocketServer::stop() -->  STOPPED!");
	XPLMDebugString(">>>");
}

