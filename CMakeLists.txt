#
# Copyright (c) Samsung Electronics Co., Ltd.
# All rights reserved.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(download-manager C CXX)

ADD_SUBDIRECTORY(oma-parser)

SET(PROJ_NAME "download-manager")
SET(VENDOR "tizen")
SET(PACKAGE ${PROJ_NAME})
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
IF(TIZEN_2_3_UX)
SET(EDJEDIR "${RESDIR}/edje")
SET(TABLEDIR "${RESDIR}/tables")
ENDIF(TIZEN_2_3_UX)
SET(IMAGEDIR "${RESDIR}/images")
SET(DATADIR "${PREFIX}/data")
SET(DBDATADIR "/home/owner/apps_rw/${PKGNAME}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "/usr/share/icons/default/small/")
SET(HISTORYDB ".download-history.db")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/include" "${CMAKE_SOURCE_DIR}/oma-parser/")

SET(SRCS
	src/main.cpp
	src/download-manager-event.cpp
	src/download-manager-network.cpp
	src/download-manager-view.cpp
	src/download-manager-viewItem.cpp
	src/download-manager-items.cpp
	src/download-manager-item.cpp
	src/download-manager-downloadItem.cpp
	src/download-manager-downloadRequest.cpp
	src/download-manager-notification.cpp
	src/download-manager-util.cpp
	src/download-manager-history-db.cpp
	src/download-manager-dateTime.cpp
	src/download-manager-network.cpp
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	capi-web-url-download
	capi-system-system-settings
	capi-appfw-application
	capi-network-connection
	capi-content-media-content
	capi-content-mime-type
	capi-base-utils-i18n
	elementary
	ecore
	edje
	xdgmime
	libcurl
	notification
	sqlite3
	storage
	vconf
	xproto
	efl-extension
	libxml-2.0
)

SET(LIB_DL dl)
SET(LIB_OMA_PARSER oma-parser)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
FOREACH(flag ${pkgs_include_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

IF(SUPPORT_WAITING_RO)
MESSAGE("SUPPORT_WAITING_RO:On")
pkg_check_modules(drm_pkgs REQUIRED drm-client)
FOREACH(flag ${drm_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
ADD_DEFINITIONS("-D_ENABLE_WAITING_RO")
#This is request of DRM Team.
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
ELSE (SUPPORT_WAITING_RO)
MESSAGE("SUPPORT_WAITING_RO:Off")
ENDIF(SUPPORT_WAITING_RO)
IF (TIZEN_2_3_UX)
MESSAGE("TIZEN_2_3_UX:On")
ADD_DEFINITIONS("-D_TIZEN_2_3_UX")
ENDIF(TIZEN_2_3_UX)

MESSAGE("ARCH: ${ARCH}")
MESSAGE("LIB_DL: ${LIB_DL}")

SET(CMAKE_C_FLAGS "${INC_FLAGS}${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall")
SET(CMAKE_CXX_FLAGS "${INC_FLAGS} ${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -Wall -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

IF("${ARCH}" MATCHES "^arm.*")
	ADD_DEFINITIONS("-DTARGET")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
ENDIF("${ARCH}" MATCHES "^arm.*")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -Wl,--hash-style=both")

ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPROJ_NAME=\"${PROJ_NAME}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DDBDATADIR=\"${DBDATADIR}\"")
ADD_DEFINITIONS("-DHISTORYDB=\"${HISTORYDB}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-D_ENABLE_OMA_DOWNLOAD")
#ADD_DEFINITIONS("-D_ENABLE_OMA_UNSUPPROTED_CONTENT")
#ADD_DEFINITIONS("-D_TEST_SUSPEND")
IF(TIZEN_2_3_UX)
ADD_DEFINITIONS("-DEDJEDIR=\"${EDJEDIR}\"")
ADD_DEFINITIONS("-DTABLEDIR=\"${TABLEDIR}\"")
ENDIF(TIZEN_2_3_UX)

ADD_EXECUTABLE(${PROJ_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJ_NAME} ${pkgs_LDFLAGS} ${drm_pkgs_LDFLAGS} ${LIB_DL} ${LIB_OMA_PARSER})

IF(TIZEN_2_3_UX)
ADD_CUSTOM_TARGET(download-manager-view.edj
                        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/res/images/wvga
                        ${CMAKE_SOURCE_DIR}/edc/download-manager-view.edc
                        ${CMAKE_BINARY_DIR}/edc/download-manager-view.edj
                        DEPENDS ${CMAKE_SOURCE_DIR}/edc/download-manager-view.edc
                )
ADD_DEPENDENCIES(${PROJECT_NAME} download-manager-view.edj)
ENDIF(TIZEN_2_3_UX)

INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(DIRECTORY DESTINATION ${DBDATADIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/${PKGNAME}.xml DESTINATION /usr/share/packages)
IF(TIZEN_2_3_UX)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images/wvga/ DESTINATION ${IMAGEDIR})
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/tables/wvga/ DESTINATION ${TABLEDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/download-manager-view.edj DESTINATION ${EDJEDIR})
ELSE(TIZEN_2_3_UX)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images/hd/ DESTINATION ${IMAGEDIR})
ENDIF(TIZEN_2_3_UX)
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/org.tizen.download-manager.png DESTINATION ${ICONDIR})
INSTALL(TARGETS ${PROJ_NAME} DESTINATION ${BINDIR})
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.Flora share/license/${PKGNAME})
INSTALL(FILES share/license/${PKGNAME} DESTINATION /usr/share/license)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PKGNAME}.efl DESTINATION ${SMACK_DIR})

# i18n
IF(TIZEN_2_3_UX)
ADD_SUBDIRECTORY(po/wvga)
ELSE(TIZEN_2_3_UX)
ADD_SUBDIRECTORY(po/hd)
ENDIF(TIZEN_2_3_UX)
