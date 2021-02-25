#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

###################################################################################################################

# Select your developer board by removing the # tag from the beginning of the line

# The default board selected is the AVNET Azure Sphere Starter Kit Revision 1.

# If you are NOT using the AVNET Revision 1 board be sure to comment out the AVNET board

set(AVNET TRUE "AVNET Azure Sphere Starter Kit Revision 1 ")    
# set(AVNET_REV_2 TRUE "AVNET Azure Sphere Starter Kit Revision 2 ")             
# set(SEEED_STUDIO_RDB TRUE "Seeed Studio Azure Sphere MT3620 Development Kit (aka Reference Design Board or rdb)")
# set(SEEED_STUDIO_MINI TRUE "Seeed Studio Azure Sphere MT3620 Mini Dev Board")

###################################################################################################################

cmake_minimum_required(VERSION 3.10)

project(AzureIoT C)

azsphere_configure_tools(TOOLS_REVISION "20.10")
azsphere_configure_api(TARGET_API_SET "7")

#Ported over from Lab 2 - Send Telemetry Azure IoT Central example
add_subdirectory("../../Azure-Sphere/LearningPathLibrary" out)

set(Source
    "main.c"
    "eventloop_timer_utilities.c"
    "parson.c"
    "hart_library.c"
)
source_group("Source" FILES ${Source})

if(AVNET OR AVNET_REV_2)

    set(Oem

        "../../Azure-Sphere/LearningPathLibrary/AVNET/board.c"
    )
    source_group("Oem" FILES ${Oem})    

    add_definitions( -DOEM_AVNET=TRUE )

endif(AVNET OR AVNET_REV_2)

if(SEEED_STUDIO_RDB OR SEEED_STUDIO_MINI)

    set(Oem
        "../../Azure-Sphere/LearningPathLibrary/SEEED_STUDIO/board.c"
    )
    source_group("Oem" FILES ${Oem})

    add_definitions( -DOEM_SEEED_STUDIO=TRUE )

endif(SEEED_STUDIO_RDB OR SEEED_STUDIO_MINI)

set(ALL_FILES
    ${Source}
    ${Oem}
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_compile_definitions(${PROJECT_NAME} PUBLIC AZURE_IOT_HUB_CONFIGURED)
target_include_directories(${PROJECT_NAME} PUBLIC ../../Azure-Sphere/LearningPathLibrary
                                ${AZURE_SPHERE_API_SET_DIR}/usr/include/azureiot 
                                ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_prov_client 
                                ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_c_shared_utility )
target_link_libraries(${PROJECT_NAME} m azureiot applibs pthread gcc_s c azsphere_libs)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-pragmas)


#NOTE: changed a path in Azure-Sphere/Drivers/AVNET_SK/HighLevel/CMakeLists.txt 
if(AVNET OR AVNET_REV_2)
    add_subdirectory("../../Azure-Sphere/Drivers/AVNET_SK/HighLevel" drivers)
    target_link_libraries(${PROJECT_NAME} avnet_drivers)
    target_include_directories(${PROJECT_NAME} PUBLIC 
                            ../../Azure-Sphere/LearningPathLibrary/AVNET
                            ../../Azure-Sphere/Drivers/AVNET_SK/HighLevel
                            ../../Azure-Sphere/Drivers/AVNET_SK/Common 
                            )
endif(AVNET OR AVNET_REV_2)

if(AVNET)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Azure-Sphere/HardwareDefinitions/avnet_mt3620_sk" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif(AVNET)

if(AVNET_REV_2)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Azure-Sphere/HardwareDefinitions/avnet_mt3620_sk_rev2" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif(AVNET_REV_2)

if (SEEED_STUDIO_RDB)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Azure-Sphere/HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif()

if (SEEED_STUDIO_MINI)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Azure-Sphere/HardwareDefinitions/seeed_mt3620_mdb" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif(SEEED_STUDIO_MINI)

azsphere_target_add_image_package(${PROJECT_NAME})




#Original CMakeList configuration
# add_executable(${PROJECT_NAME} main.c eventloop_timer_utilities.c parson.c)
# target_include_directories(${PROJECT_NAME} PUBLIC ${AZURE_SPHERE_API_SET_DIR}/usr/include/azureiot 
#                            ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_prov_client 
#                            ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_c_shared_utility)
# target_compile_definitions(${PROJECT_NAME} PUBLIC AZURE_IOT_HUB_CONFIGURED)
# target_link_libraries(${PROJECT_NAME} m azureiot applibs gcc_s c)

# azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Azure-Sphere/azure-sphere-samples/HardwareDefinitions/avnet_mt3620_sk" TARGET_DEFINITION "sample_appliance.json")

# find_program(POWERSHELL powershell.exe)

# if (POWERSHELL)
#     # Run validate_manifest script during build
#     add_custom_target(ValidateManifest ALL
#         COMMAND ${POWERSHELL} -ExecutionPolicy Bypass -NoProfile -NonInteractive -File ${CMAKE_SOURCE_DIR}/script/validate_manifest.ps1
#         DEPENDS ${CMAKE_SOURCE_DIR}/app_manifest.json)
# else()
#     # Warn users without PowerShell to update their manifest
#     add_custom_target(ValidateManifest ALL
#         COMMAND echo "Please ensure that you have updated app_manifest.json as described in IoTCentral.md or IoTHub.md, as appropriate."
#         DEPENDS ${CMAKE_SOURCE_DIR}/app_manifest.json)
# endif()

# add_dependencies(ValidateManifest ${PROJECT_NAME})

# azsphere_target_add_image_package(${PROJECT_NAME})
