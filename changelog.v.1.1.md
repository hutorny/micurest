## µcuREST change log v.1.1 

`ADD` Implemented lwIP session layer (network_lwip.cpp)
`MOD` Refactored miculog
`MOD` Complteted JSON-RPC implementation - µcuRPC 
`ADD` JSON-RPC example for Arduino Mega 2560 
`ADD` Example for lwIP 
`MOD` removed unused abstractions from network.hpp
`MOD` refactored network layers to make them consisten across all platforms 
`ADD` session layer for spark 
`ADD` Particle Photon examples
`MOD` refactored configuration to use CCS 
`MOD` refactored headers to use pragma once instead of guards 
`ADD` µcuREST tutorial

## cojson change log
`MOD` fixed bug with JSON 4HEXDIG 
`MOD` fixed PropertyConstString to accept cstring 
`MOD` fixed null pointer accessing with PropertyString*
`ADD` cojson::Read/Write template functions
