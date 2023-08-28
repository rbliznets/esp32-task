idf_component_register(SRCS "CDelayTimer.cpp" "CTraceTask.cpp" "CBaseTask.cpp"
                            "CLock.cpp"
                            "CPrintLog.cpp"
                            "CSoftwareTimer.cpp"
                            "CTrace.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_timer driver)
