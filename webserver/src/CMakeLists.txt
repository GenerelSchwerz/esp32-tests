idf_component_register(SRCS "main.cpp" "endpoints.cpp" "constants.cpp" "helper.cpp" INCLUDE_DIRS "../include")


target_add_binary_data(${COMPONENT_TARGET} "frontend/index.html" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "frontend/index.css" TEXT)