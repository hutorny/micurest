SRC = 																		\
LICENSE																		\
access_log.ccs																\
access_log.hpp																\
ccs.hpp																		\
chartypetable.cpp															\
chartypetable.inc															\
coap.hpp																	\
cojson.ccs																	\
cojson.cpp																	\
cojson_float.hpp															\
cojson_helpers.hpp															\
cojson.hpp																	\
cojson_libdep.cpp															\
configuration.h																\
enumnames.hpp																\
http_01.hpp																	\
miculog.ccs																	\
miculog.hpp																	\
miculog_spark_usart_hal.ccs													\
miculog_spark_usart_hal.cpp													\
micurest.ccs																\
micurest.cpp																\
micurest.hpp																\
micurpc.cpp																	\
micurpc.hpp																	\
network.hpp																	\
network_spark_socket.ccs													\
network_spark_socket.cpp													\
network_spark_socket.hpp													\

TRUNK = https://github.com/hutorny/micurest/trunk
TARGET_DIR = user/library/micurest/
LIBFILES = $(addprefix $(TARGET_DIR),$(SRC))
EXAMPLES = user/applications/micurest_demo user/applications/micurest_snip

all: $(LIBFILES) $(EXAMPLES) $(addsuffix /micurest,$(EXAMPLES))

$(TARGET_DIR):
	@mkdir -p $@

user/applications/micurest_demo:
	@mkdir -p $@
	svn export --force -q $(TRUNK)/examples/photon_demo $@

user/applications/micurest_demo/micurest: | user/applications/micurest_demo
	@ln -s $(realpath $(TARGET_DIR)) $@

user/applications/micurest_snip:
	@mkdir -p $@
	svn export --force -q $(TRUNK)/examples/photon_rest $@

user/applications/micurest_snip/micurest: | user/applications/micurest_snip
	@ln -s $(realpath $(TARGET_DIR)) $(shell pwd)/$@

$(TARGET_DIR)LICENSE: | $(TARGET_DIR)
	svn export  --force -q $(TRUNK)/$(notdir $@) $(TARGET_DIR)

$(TARGET_DIR)%: | $(TARGET_DIR)
	svn export --force -q $(TRUNK)/src/$(notdir $@) $(TARGET_DIR)
