# Talyte Assistant

> Talyte Assistant is a WebSockets proxy to reduce the amount of traffic that [Talyte](https://github.com/featherbear/talyte) receives, thus making [Talyte](https://github.com/featherbear/talyte) perform more optimally.

---

<!--
## Download

Prebuilt binaries for Windows, Linux and MacOS are [available via GitHub Actions](https://github.com/featherbear/talyte-assistant/actions/workflows/cmake.yml)
-->

## Usage

Run the Talyte Assistant software, and point your Talyte devices to the same machine

Note: Talyte Assistant only relays data from OBS to [Talyte](https://github.com/featherbear/talyte) clients, **not** the other way around.

---

## Rationale

[Talyte](https://github.com/featherbear/talyte) makes a direction connection to the obs-websockets plugin, which tends to send quite a lot of data. This can cause the ESP32 running Talyte to operate rather slowly, or even crash it.  

Talyte Assistant relays the `PreviewSceneChanged` and `SwitchScenes` events sent from OBS. It also removes the `sources` key, since all [Talyte](https://github.com/featherbear/talyte) needs is the source name

---

## TODO

* Parse command line arguments
* Source in Scene - Currently Talyte devices operate via being linked to a specific Scene(s)
* Subscribe to certain events via the websocket URI query

---

## License

This software is licensed under the MIT license, as can be viewed [here](LICENSE.md).
