AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS = ${CXXFLAGS} -I./include

EXTRA_DIST = autogen.sh csmain.h sysconf/clearsync.tmp m4/adl_recursive_eval.m4 \
	sysconf/clearsyncd.init sysconf/clearsync.service.in \
	clearsync.spec

sysconf_DATA = sysconf/clearsync.conf

lib_LTLIBRARIES = libclearsync.la

libclearsync_la_SOURCES = csconf.cpp csevent.cpp cslog.cpp csnetlink.cpp \
	csplugin.cpp csthread.cpp cssocket.cpp cstimer.cpp csutil.cpp
libclearsync_la_CXXFLAGS = ${AM_CXXFLAGS} -D_CS_INTERNAL=1
libclearsync_la_includedir = $(includedir)/clearsync
libclearsync_la_include_HEADERS = include/clearsync/csconf.h include/clearsync/csevent.h \
	include/clearsync/csexception.h include/clearsync/cslog.h include/clearsync/csnetlink.h \
	include/clearsync/csplugin.h include/clearsync/csthread.h include/clearsync/cssocket.h \
	include/clearsync/cstimer.h include/clearsync/csutil.h

sbin_PROGRAMS = clearsyncd

clearsyncd_SOURCES = csmain.cpp
clearsyncd_LDADD = libclearsync.la
clearsyncd_CXXFLAGS = ${AM_CXXFLAGS} -D_CS_INTERNAL=1

