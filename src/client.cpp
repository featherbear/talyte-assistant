#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define _WEBSOCKETPP_MINGW_THREAD_
#endif

#define ASIO_STANDALONE

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <functional>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

typedef websocketpp::client<websocketpp::config::asio_client> ClientType;
typedef websocketpp::client<websocketpp::config::asio> ServerType;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

void on_message(ClientType* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::string payload = msg->get_payload();
    rapidjson::Document d;

    d.Parse(payload.c_str());

    std::string updateType = d["update-type"].GetString();
    if (updateType != "SwitchScenes" && updateType != "PreviewSceneChanged") {
        return;
    }

    // Remove sources key, as it often gets large
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
        // c.set_open_handler(bind(&on_open, &c, ::_1));
        // c.set_fail_handler(bind(&on_fail, &c, ::_1));
        c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));
        // c.set_close_handler(bind(&on_close, &c, ::_1));

        // Create a connection to the given URI and queue it for connection once
        // the event loop starts
        websocketpp::lib::error_code ec;
        ClientType::connection_ptr con = c.get_connection(uri, ec);
        c.connect(con);

        // Start the ASIO io_service run loop
        c.run();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
    return 0;
}