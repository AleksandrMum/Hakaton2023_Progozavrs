idf_component_register(SRCS "hahaton_main.c" "RCSwitch.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/servercert.pem"
                                   "certs/prvtkey.pem")
