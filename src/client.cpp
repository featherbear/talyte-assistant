#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define _WEBSOCKETPP_MINGW_THREAD_
#endif

#define ASIO_STANDALONE

#include <functional>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>

#include "protoTools.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> ClientType;
typedef websocketpp::client<websocketpp::config::asio> ServerType;

typedef websocketpp::config::asio_client::message_type::ptr messagePtr;

bool versionCheckDone = false;
bool authRequiredCheckDone = false;
bool authRequired = true;
bool authDone = false;

void on_open(ClientType* c, websocketpp::connection_hdl hdl) {
    c->send(hdl, _getVersionRequestJSONString(), websocketpp::frame::opcode::text);
}

void on_message(ClientType* c, websocketpp::connection_hdl hdl, messagePtr msg) {
    // Read payload as JSON
    std::string payload = msg->get_payload();
    std::cout << "Payload:" << payload << std::endl;

    rapidjson::Document d;
    d.Parse(payload.c_str());

    // Skip non-JSON results
    if (d.HasParseError()) return;

    // Handle any proto cases
    if (d.HasMember("message-id")) {
        std::string messageId(d["message-id"].GetString());

        // Client response
        if (!versionCheckDone && messageId == _GetVersionString) {
            versionCheckDone = true;

            // Now check for auth requests
            c->send(hdl, _getAuthRequiredJSONString().c_str(), websocketpp::frame::opcode::text);
        } else if (!authRequiredCheckDone && messageId == _AuthRequiredString) {
            if (authRequired = d["authRequired"].GetBool()) {
                c->send(hdl, _generateAuthJSONString("password", d["challenge"].GetString(), d["salt"].GetString()).c_str(), websocketpp::frame::opcode::text);
            }
        }
    }

    // Ignore unless obs-websocket has replied
    if (!versionCheckDone) return;

    // Skip results that do not contain `update-type`
    if (!d.HasMember("update-type")) return;

    std::string updateType(d["update-type"].GetString());

    c->get_alog().write(websocketpp::log::alevel::app, payload);
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
    // c->close(hdl, websocketpp::close::status::normal, "");
}

int main() {
    ClientType c;

    std::string uri = "ws://localhost:4444";

    try {
        // set logging policy if needed
        c.clear_access_channels(websocketpp::log::alevel::frame_header);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        //c.set_error_channels(websocketpp::log::elevel::none);

        // Initialize ASIO
        c.init_asio();

        // Register our handlers
        c.set_open_handler(websocketpp::lib::bind(&on_open, &c, std::placeholders::_1));
        c.set_message_handler(websocketpp::lib::bind(&on_message, &c, std::placeholders::_1, std::placeholders::_2));
        // c.set_fail_handler(bind(&on_fail, &c, ::_1));
        // c.set_close_handler(bind(&on_close, &c, ::_1));

        // Create a connection to the given URI and queue it for connection once the event loop starts
        websocketpp::lib::error_code ec;
        ClientType::connection_ptr con = c.get_connection(uri, ec);
        c.connect(con);

        // Start the ASIO io_service run loop
        c.run();
    } catch (websocketpp::lib::error_code e) {
        std::cout << "M" << e.message() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "E" << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
    return 0;
}