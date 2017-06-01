# Building Photon examples

Obtain Particle SDK

`git clone https://github.com/kbowerma/particle`

Obtain and install a gcc-arm compiler (4.9 or higher)
from [`ARM`](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
or from [`launchpad.net`](https://launchpad.net/gcc-arm-embedded)
(tested with 4.9 and 5.4)

Make sure new arm gcc is in the path<br>
`export PATH=<path-to>/gcc-arm-none-eabi-4_9/bin:$PATH`

Change directory to Particle's SDK

`cd <path-to>/particle`

Place library sources and examples in the SKD's tree

* from prepackaged zip:<br>
`wget https://github.com/hutorny/download/raw/master/Micurest_Particle.zip`<br>
`unzip Micurest_Particle.zip`
* from git repository (requires svn!):<br>
`wget https://raw.githubusercontent.com/hutorny/micurest/master/examples/photon_rest/micurest4particle.mk`<br>
`make -f  micurest4particle.mk`

Build photon core<br>
`make PLATFORM=photon`

Build the example application<br>
`make PLATFORM=photon APP=micurest_snip CPPFLAGS="-std=gnu++14 -Os"`

or demo application<br>
`make PLATFORM=photon APP=micurest_demo CPPFLAGS="-std=gnu++14 -Os"`
