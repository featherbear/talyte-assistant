#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define _WEBSOCKETPP_MINGW_THREAD_
#endif

#define ASIO_STANDALONE

#define CLIENT_SPACE 50

#include <functional>
#include <mutex>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>

#include "protoTools.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> ClientType;
typedef websocketpp::server<websocketpp::config::asio> ServerType;
typedef websocketpp::config::asio_client::message_type::ptr messagePtr;

ClientType c;
ServerType s;

bool versionCheckDone = false;

void on_open(ClientType* c, websocketpp::connection_hdl hdl) {
    // Check server is the right protocol
    c->send(hdl, _getVersionRequestJSONString(), websocketpp::frame::opcode::text);
}

std::mutex connections_mutex;
std::vector<websocketpp::connection_hdl> connections;

void on_client_connect(ServerType* s, websocketpp::connection_hdl hdl) {
    connections_mutex.lock();
    if (connections.size() == connections.capacity()) {
        connections.reserve(connections.size() + CLIENT_SPACE);
    };
    connections.push_back(hdl);
    connections_mutex.unlock();
}

void on_client_disconnect(ServerType* s, websocketpp::connection_hdl hdl) {
    connections_mutex.lock();

    bool found = false;
    unsigned int i = 0;
    auto target_ptr = hdl.lock();

    for (auto conn_wptr : connections) {
        if (conn_wptr.lock() == target_ptr) {
            found = true;
            break;
        }

        i++;
    }

    if (found) connections.erase(connections.begin() + i);

    connections_mutex.unlock();
}

void on_message(ClientType* c, websocketpp::connection_hdl hdl, messagePtr msg) {
    // Read payload as JSON
    std::string payload = msg->get_payload();
    rapidjson::Document d;
    d.Parse(payload.c_str());

    // Skip non-JSON results
    if (d.HasParseError()) return;

    // Handle any proto cases
    if (d.HasMember("message-id")) {
        std::string messageId(d["message-id"].GetString());

        // Server protocol check
        if (messageId == _GetVersionString) {
            // Assume any version check response means that the server is working
            versionCheckDone = true;

            // Now check for auth requests
            c->send(hdl, _getAuthRequiredJSONString().c_str(), websocketpp::frame::opcode::text);
            return;

            // Auth require check
        } else if (messageId == _AuthRequiredString) {
            if (d["authRequired"].GetBool()) {
                if (false /* password empty */) {
                    c->close(hdl, websocketpp::close::status::invalid_subprotocol_data, "Password required and was not supplied");
                }

                c->send(hdl, _generateAuthJSONString("password", d["challenge"].GetString(), d["salt"].GetString()).c_str(), websocketpp::frame::opcode::text);
            }

            return;

            // Auth response check
        } else if (messageId == _AuthenticateString) {
            if (!(std::string(d["status"].GetString()) == "ok")) {
                c->close(hdl, websocketpp::close::status::invalid_subprotocol_data, "Supplied password was incorrect");
            }

            return;
        }
    }

    // Ignore unless obs-websocket has replied
    if (!versionCheckDone) return;

    // Skip results that do not contain `update-type`
    if (!d.HasMember("update-type")) return;
    std::string updateType(d["update-type"].GetString());

    // Only handle SwitchScenes and PreviewSceneChanged events
    // TODO: Maybe also handle the transition ones
    if (updateType != "SwitchScenes" && updateType != "PreviewSceneChanged") {
        return;
    }

    // Remove `sources` key, as it often gets large
    d.RemoveMember("sources");

    rapidjson::StringBuffer message;
    rapidjson::Writer<rapidjson::StringBuffer> writer(message);
    d.Accept(writer);
    c->get_alog().write(websocketpp::log::alevel::app, message.GetString());

    connections_mutex.lock();
    for (auto conn : connections) {
        s.send(conn, message.GetString(), websocketpp::frame::opcode::text);
    }
    connections_mutex.unlock();
}

int main() {
    // TODO: Variable uri
    std::string uri = "ws://localhost:4444";
    int port = 4443;

    // Shared async IO service
    asio::io_service async_service;

    try {
        c.clear_access_channels(websocketpp::log::alevel::all);
        c.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect | websocketpp::log::alevel::app);
        // c.set_error_channels(websocketpp::log::elevel::none);

        c.init_asio(&async_service);
        c.set_open_handler(std::bind(&on_open, &c, std::placeholders::_1));
        c.set_message_handler(std::bind(&on_message, &c, std::placeholders::_1, std::placeholders::_2));

        // TODO: Reconnect
        // c.set_close_handler(bind(&on_close, &c, ::_1));
        // c.set_fail_handler(bind(&on_fail, &c, ::_1));

        websocketpp::lib::error_code ec;
        ClientType::connection_ptr con = c.get_connection(uri, ec);

        c.get_alog().write(websocketpp::log::alevel::app, std::string("Relay connecting to ") + uri);
        c.connect(con);

    } catch (websocketpp::lib::error_code e) {
        std::cout << "Client: " << e.message() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Client: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "other client exception" << std::endl;
    }

    try {
        s.clear_access_channels(websocketpp::log::alevel::all);
        s.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect | websocketpp::log::alevel::app);
        s.set_error_channels(websocketpp::log::elevel::none);

        s.init_asio(&async_service);
        s.set_open_handler(std::bind(&on_client_connect, &s, std::placeholders::_1));
        s.set_close_handler(std::bind(&on_client_disconnect, &s, std::placeholders::_1));

        // TODO: Variable port number
        s.listen(port);
        s.get_alog().write(websocketpp::log::alevel::app, std::string("Server is listening on port ") + std::to_string(port));

        s.start_accept();
    } catch (websocketpp::lib::error_code e) {
        std::cout << "Server: " << e.message() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Server: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "other server exception" << std::endl;
    }

    async_service.run();

    return 0;
}