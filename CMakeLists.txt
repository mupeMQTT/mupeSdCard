idf_component_register(SRCS "mupeSdCard.c" "mupeDataBase.c" "mupeSdCardNvs.c" "mupeSdCardWeb.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "fatfs" "vfs" "sdmmc" "mupeClientMqtt" "mqtt" "mupeMdnsNtp"
                    EMBED_TXTFILES  "sdCard.html" )
                    
             

