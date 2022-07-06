# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c" "wifi_functions.c" INCLUDE_DIRS "."
    EMBED_FILES server_certs/certs.pem)
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
