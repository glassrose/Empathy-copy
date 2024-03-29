SUBDIRS = interactive xml certificates

CLEANFILES=

SUPPRESSIONS=tp-glib.supp dlopen.supp

AM_CPPFLAGS =						\
	$(ERROR_CFLAGS)					\
	-I$(top_srcdir)/libempathy                     	\
	-I$(top_srcdir)/libempathy-gtk                 	\
	-DPKGDATADIR=\""$(pkgdatadir)"\"		\
	-DGCR_API_SUBJECT_TO_CHANGE			\
	$(EMPATHY_CFLAGS)				\
	$(TPAW_CFLAGS)					\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED)				\
	-DSRCDIR=\""$(abs_srcdir)"\"			\
	$(NULL)

LDADD =								\
	$(top_builddir)/libempathy-gtk/libempathy-gtk.la	\
	$(top_builddir)/libempathy/libempathy.la		\
	$(TPAW_LIBS)						\
	$(EMPATHY_LIBS)						\
	$(NULL)

tests_list =  \
     empathy-irc-server-test                     \
     empathy-irc-network-test                    \
     empathy-irc-network-manager-test            \
     empathy-chatroom-test                       \
     empathy-chatroom-manager-test               \
     empathy-parser-test                         \
     empathy-live-search-test                    \
     empathy-tls-test

noinst_PROGRAMS = $(tests_list)
TESTS = $(tests_list)

empathy_tls_test_SOURCES = empathy-tls-test.c \
     test-helper.c test-helper.h \
     mock-pkcs11.c mock-pkcs11.h

empathy_irc_server_test_SOURCES = empathy-irc-server-test.c \
     test-helper.c test-helper.h                       \
     test-irc-helper.h test-irc-helper.c

empathy_irc_network_test_SOURCES = empathy-irc-network-test.c \
     test-helper.c test-helper.h                       \
     test-irc-helper.h test-irc-helper.c

empathy_irc_network_manager_test_SOURCES = empathy-irc-network-manager-test.c \
     test-helper.c test-helper.h                       \
     test-irc-helper.h test-irc-helper.c

empathy_chatroom_test_SOURCES = empathy-chatroom-test.c \
     test-helper.c test-helper.h

empathy_chatroom_manager_test_SOURCES = empathy-chatroom-manager-test.c \
     test-helper.c test-helper.h

empathy_parser_test_SOURCES = empathy-parser-test.c \
     test-helper.c test-helper.h

empathy_live_search_test_SOURCES = empathy-live-search-test.c \
     test-helper.c test-helper.h

TESTS_ENVIRONMENT = EMPATHY_SRCDIR=@abs_top_srcdir@ \
		    MC_PROFILE_DIR=@abs_top_srcdir@/tests \
		    MC_MANAGER_DIR=@abs_top_srcdir@/tests

test-report: test-report.xml
	gtester-report $(top_builddir)/tests/$@.xml > \
	  $(top_builddir)/tests/$@.html

test-report.xml: ${TESTS} test

test: ${TESTS}
	gtester -o test-report.xml -k --verbose ${TESTS}

test-%: empathy-%-test
	gtester -o $@-report.xml -k --verbose $<

.PHONY: test test-report
