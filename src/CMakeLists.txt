add_library(threed_beam_fea threed_beam_fea.cpp summary.cpp setup.cpp)

add_executable(fea_cmd cmd.cpp)
target_link_libraries(fea_cmd threed_beam_fea)
