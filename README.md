# MOOS-pGeodesy
This is a translator between a position source in earth coordinates (both lat-lon and UTM) and local coordinates. In order to accommodate trans-oceanic travel, it is able to re-base the local coordinate system to the current earth coordinate position either on command or when the vessel passes beyond a certain distance from the origin.

## Dependencies
* MOOS-IvP
* libMOOSGeodesy
* https://github.com/Tencent/rapidjson/ -- provides JSON parse/deparse

## Coordinate Systems
This module exists to provide translation between different coordinate systems the various MOOS modules use to represent and reason about the current location of the boat. IvP-Helm and most MOOS modules use local coordinates, while most position sources (such as GPS) provide earth coordinates.

### Earth coordinates
GPS and GNSS systems provide the current latitude and longitude, typically in terms of the WGS84 ellipsoid. Latitude is positive north of the equator and negative south of it. Longitude is counted from the longitude of the Royal Observatory in Greenwich, England. Points east of the observatory have positive longitude and points west have negative longitude. Latitude and longitudes are typically expressed in decimal degrees, but may also be expressed as degrees-minutes-seconds or in radians.

This module provides a translation of latitude and longitude into Universal Transverse Mercator (UTM) coordinates. UTM divides the Earth into sixty zones of longitude, each six degrees wide. Within each band, coordinates are expressed as meters east and north of the point on the equator at the midpoint of the band.

### Local Coordinates
Local coordinates are defined as meters east and north of a defined origin point. These coordinates are accurate within approximately 100 km of the origin point. There are three different ways to define the origin point -- either at startup, in the .moos file, or by re-basing the coordinates while underway.

If no origin point is provided in the .moos file (as LatOrigin and LongOrigin), this module will average the first ten location samples received and use that value as the origin.

The local coordinates can also be re-based to the current location of the boat if it is too far from the current origin point or if it is commanded to do so.

## Configuration parameters
* InputLat -- the variable this module subscribes to for latitude.
* InputLon -- the variable this module subscribes to for longitude.
* InputFixStatus -- the variable that this module subscribes to get the current fix mode.
* OutputX -- the variable that this module publishes containing the current X coordinate in local coordinates.
* OutputY -- the variable that this module publishes containing the current X coordinate in local coordinates.
* OutputEasting -- the variable that this module publishes containing the current easting.
* OutputNorthing -- the variable that this module publishes containing the current northing.
* OutputUTMzone -- the variable that this module publishes containing the current UTM zone.
* OutputOriginPeriod -- the number of ticks between transmission of the current origin.
* OutputOriginLat -- the variable this module publishes containing the current origin latitude
* OutputOriginLon -- the variable this module publishes containing the current origin longitude
* RebaseConf -- is a JSON string defining the conditions under which the origin point will be re-based. It must correspond to the following schema:
```
{
	"$schema": "http://json-schema.org/schema#",
	"id": "Geodesy_rebase",
    "type": "object",
    "properties": {
        "rebase_condition": {"type": "string", "enum": ["none", "distance", "triggered"]},
        "rebase_distance": {"type": "number"},
        "rebase_trigger_var": {"type": "string"},
        "rebase_notify_var": {"type": "string"}
    }
}
```
