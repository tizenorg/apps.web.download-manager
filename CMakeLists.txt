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
SET(IMAGEDIR "${RESDIR}/images")
SET(DATADIR "${PREFIX}/data")
SET(DBDATADIR "/opt/usr/apps/${PKGNAME}/data/db")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "/usr/share/icons/default/small/")
SET(HISTORYDB ".download-history.db")
SET(EDJE_DIR "${RESDIR}/edje")
SET(EDJE_PATH "${EDJE_DIR}/${PROJ_NAME}.edj")

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
	capi-system-runtime-info
	capi-appfw-application
	capi-network-connection
	capi-content-media-content
	elementary
	aul
	ecore
	edje
	icu-i18n
	xdgmime
	edje
	libcurl
	appsvc
	notification
	db-util
	sqlite3
)

SET(LIB_DL dl)
SET(LIB_OMA_PARSER oma-parser)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
FOREACH(flag ${pkgs_include_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

MESSAGE("ARCH: ${ARCH}")
MESSAGE("LIB_DL: ${LIB_DL}")

SET(CMAKE_C_FLAGS "${INC_FLAGS}${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall")
SET(CMAKE_CXX_FLAGS "${INC_FLAGS} ${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

IF("${ARCH}" MATCHES "^arm.*")
	ADD_DEFINITIONS("-DTARGET")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
ENDIF("${ARCH}" MATCHES "^arm.*")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -Wl,--hash-style=both")

ADD_DEFINITIONS("-DVENDOR=\"${VENDOR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DDBDATADIR=\"${DBDATADIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DHISTORYDB=\"${HISTORYDB}\"")
ADD_DEFINITIONS("-DEDJE_DIR=\"${EDJE_DIR}\"")
ADD_DEFINITIONS("-DEDJE_PATH=\"${EDJE_PATH}\"")
ADD_DEFINITIONS("-D_ENABLE_OMA_DOWNLOAD")
ADD_DEFINITIONS("-D_ENABLE_ROTATE")
ADD_DEFINITIONS("-D_BOX_NOTI_TYPE")

ADD_EXECUTABLE(${PROJ_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJ_NAME} ${pkgs_LDFLAGS} ${LIB_DL} ${LIB_OMA_PARSER})

ADD_CUSTOM_TARGET(download-manager.edj
	COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
	${CMAKE_SOURCE_DIR}/res/download-manager.edc
	${CMAKE_BINARY_DIR}/res/download-manager.edj
	DEPENDS ${CMAKE_SOURCE_DIR}/res/download-manager.edc)
ADD_DEPENDENCIES(${PROJ_NAME} download-manager.edj)

INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(DIRECTORY DESTINATION ${DBDATADIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/${PKGNAME}.xml DESTINATION /usr/share/packages)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images DESTINATION ${RESDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/download-manager.edj DESTINATION ${EDJE_DIR})
INSTALL(TARGETS ${PROJ_NAME} DESTINATION ${BINDIR})
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.Flora share/license/${PKGNAME})
INSTALL(FILES share/license/${PKGNAME} DESTINATION /usr/share/license)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.download-manager.rule DESTINATION /etc/smack/accesses.d)

# i18n
ADD_SUBDIRECTORY(po)
