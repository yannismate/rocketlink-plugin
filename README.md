[![Build](https://github.com/yannismate/rocketlink-plugin/actions/workflows/msbuild.yml/badge.svg)](https://github.com/yannismate/rocketlink-plugin/actions/workflows/msbuild.yml)  
# rocketlink-plugin
BakkesMod Plugin providing relative position data about other players in a RocketLeague game for the RocketLink Client.  
This code is heavily inspired by [SOS / Simple Overlay System](https://gitlab.com/bakkesplugins/sos/sos-plugin) made by some other great people. Check their repo out for more info.

## Build
1. Install [BakkesMod](https://bakkesmod.com/) and make sure the SDK Path is set.
2. Execute `msbuild` in the root directory of the project

## WebSocket
This plugin runs a WebSocket server on port 49124 (configurable through the cvar "RLINK_Port") emitting relative player positions when in a local server.
```json
{
   "players":[
         {
            "distance": 1000,
            "id":"PLAYERID",
            "name": "PLAYERNAME",
            "team": 1,
            "__comment__": "This is the relative XYZ position of the other player to your own camera",
            "x": 0,
            "y": 0,
            "z": 0
         }
    ],
    "self_id":"YOUR OWN PLAYERID",
    "self_name":"YOUR OWN PLAYER NAME",
    "self_team": 1
}
```
