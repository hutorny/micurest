## µcuREST change log v.1.1 

`ADD` Implemented lwIP session layer (network_lwip.cpp)<br/>
`MOD` Refactored miculog<br/>
`MOD` Complteted JSON-RPC implementation - µcuRPC<br/> 
`ADD` JSON-RPC example for Arduino Mega 2560<br/>
`ADD` Example for lwIP<br/>
`MOD` removed unused abstractions from network.hpp<br/>
`MOD` refactored network layers to make them consisten across all platforms<br/>
`ADD` session layer for spark<br/>
`ADD` Particle Photon examples<br/>
`MOD` refactored configuration to use CCS<br/>
`MOD` refactored headers to use pragma once instead of guards<br/>
`ADD` µcuREST tutorial<br/>

## cojson change log
`MOD` fixed bug with JSON 4HEXDIG<br/>
`MOD` fixed PropertyConstString to accept cstring<br/>
`MOD` fixed null pointer accessing with PropertyString*<br/>
`ADD` cojson::Read/Write template functions<br/>
