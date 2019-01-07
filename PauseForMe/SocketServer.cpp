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
	server::connection_ptr con           = wsServer.get_con_from_hdl(hdl);
	std::string            remoteAddress = con->get_socket().remote_endpoint().address().to_string();
	std::string            remotePort    = std::to_string(con->get_socket().remote_endpoint().port());

	connection_data data;
	data.sessionid = this->sessionId++;
	data.name      = remoteAddress + ":" + remotePort;

	this->connections[hdl] = data;
	this->arraySocketClients.push_back(hdl);
}

void SocketServer::on_close(websocketpp::connection_hdl hdl) {
	this->connections.erase(hdl);
}

connection_data& SocketServer::get_data_from_hdl(websocketpp::connection_hdl hdl) {
	auto it = this->connections.find(hdl);

	if (it == this->connections.end()) {
		throw std::invalid_argument("No data avaliable for session");
	}

	return it->second;
}

void SocketServer::setCallBack(CallBackHandler* callBackHandler)
{	
	this->callBackhandler = callBackHandler;
}

void SocketServer::on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
	// Reading data of the connection
	std::string      message        = msg->get_payload();
	connection_data& dataConnection = get_data_from_hdl(hdl);

	// Extracting information from connection 
	std::string origin = dataConnection.name;
	std::string logMsg = "SocketServer::on_message() --> RECEIVED(origin, message) --> (" + origin + "," + message + ")";

	// Accept the message sent
	this->callBackhandler->acceptMessage(origin, message);

	// Check if the message is a request for close the connection and then do it
	if ( message.rfind("{CLOSE}", 0) == 0 ) {
		try {
			XPLMDebugString( ("Requested CLOSE the Connection by Client " + origin).c_str());
			this->connections.erase(hdl);
			this->wsServer.pause_reading(hdl);
			this->wsServer.close(hdl, websocketpp::close::status::going_away, "Requested by Client " + origin);
		}
		catch (websocketpp::exception const & e) {
			std::string error = e.what();
			std::string msg = "SocketServer::stop() --> ERROR --> " + error;
			XPLMDebugString(msg.c_str());
		}
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



