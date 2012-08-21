#
# Copyright (c) Samsung Electronics Co., Ltd.
# All rights reserved.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(download-manager C CXX)

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
	src/download-manager-util.cpp
	src/download-manager-history-db.cpp
	src/download-manager-dateTime.cpp
	src/download-manager-network.cpp
)

SET(VENDOR "tizen")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(IMAGEDIR "${RESDIR}/images")
SET(DATADIR "${PREFIX}/data")
SET(DBDATADIR "${PREFIX}/data/db")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "/opt/share/icons/default/small/")
SET(HISTORYDB ".download-history.db")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	capi-web-url-download
	capi-system-runtime-info
	capi-appfw-application
	capi-network-connection
	capi-content-media-content
	elementary
	aul
	bundle
	ecore
	edje
	icu-i18n
	xdgmime
 )

FIND_LIBRARY(LIB_DL dl)

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

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
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -mfpu=vfp -mfloat-abi=softfp")
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

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ${LIB_DL})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})

INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(DIRECTORY DESTINATION ${DBDATADIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/${PKGNAME}.xml DESTINATION /opt/share/packages)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/res/images DESTINATION ${RESDIR})

# i18n
ADD_SUBDIRECTORY(po)

