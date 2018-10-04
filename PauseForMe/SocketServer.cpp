#include "SocketServer.h"

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
			//std::cout << "Failed send message to client..: " << "(" << e.what() << ")" << std::endl;
		}
	}
}

void SocketServer::on_open(websocketpp::connection_hdl hdl) {
	connection_data data;

	data.sessionid = this->sessionId++;
	data.name = "Connection " + std::to_string(data.sessionid);

	// std::cout << "Open connection with " << data.name << " with sessionid " << data.sessionid << std::endl;

	this->connections[hdl] = data;
	this->arraySocketClients.push_back(hdl);
}

void SocketServer::on_close(websocketpp::connection_hdl hdl) {
	connection_data& data = get_data_from_hdl(hdl);

	//std::cout << "Closing connection " << data.name << " with sessionid " << data.sessionid << std::endl;

	this->connections.erase(hdl);
}

connection_data& SocketServer::get_data_from_hdl(websocketpp::connection_hdl hdl) {
	auto it = this->connections.find(hdl);

	if (it == this->connections.end()) {
		// this connection is not in the list. This really shouldn't happen
		// and probably means something else is wrong.
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
		//std::cout << "Echo failed because: " << "(" << e.what() << ")" << std::endl;
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

		//cout << "Listening on " << this->port << "..." << std::endl;

		this->wsServer.start_accept();
		this->wsServer.run();

	}
	catch (websocketpp::exception const & e) {
		//std::cout << e.what() << std::endl;
	}
	catch (...) {
		//std::cout << "Error!!!" << std::endl;
	}
}

void SocketServer::stop() {
	this->wsServer.stop_listening();
}
