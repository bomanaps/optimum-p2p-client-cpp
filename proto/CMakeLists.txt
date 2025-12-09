# Protobuf and gRPC are already found by parent CMakeLists.txt
# The protobuf_generate_cpp function is available from the parent's find_package(Protobuf)

# Set output directory for generated files
set(PROTO_OUT_DIR ${CMAKE_BINARY_DIR}/proto)

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/p2p_stream.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/proxy_stream.proto
)

# Generate protobuf C++ files manually using protoc
# This avoids needing protobuf_generate_cpp function
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    set(PROTO_SRCS ${PROTO_SRCS} ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc)
    set(PROTO_HDRS ${PROTO_HDRS} ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h)
    
    add_custom_command(
        OUTPUT ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc
               ${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h
        COMMAND protobuf::protoc
        ARGS --cpp_out=${PROTO_OUT_DIR}
             -I${CMAKE_CURRENT_SOURCE_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running protobuf C++ compiler on ${PROTO_FILE}"
    )
endforeach()

# Generate gRPC C++ files
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    get_filename_component(PROTO_PATH ${PROTO_FILE} PATH)
    
    set(GRPC_SRCS ${GRPC_SRCS} ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.cc)
    set(GRPC_HDRS ${GRPC_HDRS} ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.h)
    
    add_custom_command(
        OUTPUT ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.cc
               ${PROTO_OUT_DIR}/${PROTO_NAME}.grpc.pb.h
        COMMAND protobuf::protoc
        ARGS --grpc_out=${PROTO_OUT_DIR}
             --cpp_out=${PROTO_OUT_DIR}
             --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
             -I${CMAKE_CURRENT_SOURCE_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running gRPC C++ protocol buffer compiler on ${PROTO_FILE}"
    )
endforeach()

# Create proto library
add_library(optimum_proto STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)

target_link_libraries(optimum_proto
    PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

# Include generated headers
target_include_directories(optimum_proto
    PUBLIC
    ${PROTO_OUT_DIR}
)

