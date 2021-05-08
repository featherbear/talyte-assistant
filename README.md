# Talyte Assistant

Talyte makes a direction connection to the obs-websockets plugin, which tends to send quite a lot of data.  
This can cause the ESP32 running Talyte to operate rather slowly, or even crash it.  

**Talyte Assitant** is a websockets proxy that only relays the `PreviewSceneChanged` and `SwitchScenes` events sent from OBS.  
It also removes the `sources` key, since all Talyte needs is the source name

* TODO: Source in Scene - Currently Talyte devices operate via being linked to a specific Scene(s)
* (?) TODO: Subscribe to certain events via the websocket URI