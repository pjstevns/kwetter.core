#include <zmq.hpp>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <json/json.h>
#include "m2pp.hpp"

json_object * parse_json(const std::string& jsondoc) {
	json_object * jobj = json_tokener_parse(jsondoc.c_str());

	if (jobj && json_object_is_type(jobj, json_type_object))
		return jobj;

	if (jobj) json_object_put(jobj); // free json object

	return NULL;
}              

class Response {

	public:
		std::string response;
		uint16_t code;
		const std::string &status;
		std::vector<m2pp::header> hdrs;
		Response(const std::string& r, uint16_t c=200, const std::string& s="OK", std::vector<m2pp::header> h = std::vector<m2pp::header>());
};


Response::Response(const std::string& r, uint16_t c, const std::string& s, std::vector<m2pp::header> h)
	: response(r), code(c), status(s), hdrs(h)
{;}

class Kwetter {
	std::string uri; // upstream URI of kwetterd
	zmq::context_t ctx;
	zmq::socket_t sock;
	public:
		Kwetter(const std::string&);
		Response* handle(const m2pp::request &);
		std::string s_recv();
		int s_send(const std::string &);
};

Kwetter::Kwetter(const std::string& u)
	:uri(u), ctx(1), sock(ctx, ZMQ_REQ)
{ 
	sock.connect(uri.c_str());
}

std::string Kwetter::s_recv()
{
	zmq::message_t msg;
	sock.recv(&msg);
	return std::string(static_cast<const char *>(msg.data()), msg.size());
}

int Kwetter::s_send(const std::string& msg)
{
	zmq::message_t req(msg.length());
	memcpy((void *)req.data(), msg.data(), msg.length());
	return sock.send(req);
}

Response* Kwetter::handle(const m2pp::request& req)
{
	uint16_t code = 200;
	std::string status = "OK";
	std::vector<m2pp::header> hdrs;
	// kwetterd

	// mongrel2
	std::ostringstream response;
	std::vector<m2pp::header> headers = req.headers;

	for (std::vector<m2pp::header>::iterator it=headers.begin();it!=headers.end();it++) {
		if (it->first == "content-type" && it->second == "application/json") {
			json_object *payload = parse_json(req.body);
			if (! payload) {
				code = 503;
				status = "RequestError";
			} else {

				// push payload to kwetterd
				s_send(req.body);
				std::string result = s_recv();
				response << result << std::endl;
				if (result == "NO" || result == "ERR") {
					code = 502;
					status = "ServerError";
				}
				// cleanup
				json_object_put(payload);
			}
		} 
	}
	response << std::endl;

	std::cout << response.str();
	std::cout << "length: " << response.tellp() << std::endl;

	Response *R = new Response(response.str(), code, status);
	return R;
}

int main(void) {
	Kwetter *server_conn;
	std::string sender_id = "82209006-86FF-4982-B5EA-D1E29E55D481";
	std::string kwetter_uri = "tcp://localhost:5555";

	m2pp::connection conn(sender_id, "tcp://127.0.0.1:9999", "tcp://127.0.0.1:9998");
	server_conn = new Kwetter(kwetter_uri);

	while (1) {
		m2pp::request req = conn.recv();
		Response *R = server_conn->handle(req);
		conn.reply_http(req, R->response);
	}

	return 0;
}

