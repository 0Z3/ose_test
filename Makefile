############################################################
# Testing
############################################################
ifndef LIBOSE_DIR
LIBOSE_DIR=../../../libose
endif

TEST_DIR=..
SRC_DIR=../..

LIBOSE_CFILES :=\
$(LIBOSE_DIR)/ose.c\
$(LIBOSE_DIR)/ose_context.c\
$(LIBOSE_DIR)/ose_print.c\
$(LIBOSE_DIR)/ose_match.c\
$(LIBOSE_DIR)/ose_stackops.c\
$(LIBOSE_DIR)/ose_util.c\
$(LIBOSE_DIR)/ose_vm.c\
$(LIBOSE_DIR)/osevm_lib.c

UNIT_TEST_TARGETS=$(foreach f,$(shell ls $(TEST_DIR) | egrep '^ut_.*\.(c|cpp|cc)$$'),$(basename $(f)))

TESTS=$(UNIT_TEST_TARGETS)
$(info $(TESTS))

CFLAGS_TEST_DEBUG=-Wall \
-I$(LIBOSE_DIR) -I$(SRC_DIR) -I$(TEST_DIR) -I.\
-DOSE_CONF_DEBUG \
-O0 \
-g \
-DOSE_QUIET_ASSERT \
-DOSE_CONF_PROVIDE_TYPE_SYMBOL \
-DOSE_CONF_PROVIDE_TYPE_DOUBLE \
-DOSE_CONF_PROVIDE_TYPE_INT8 \
-DOSE_CONF_PROVIDE_TYPE_UINT8 \
-DOSE_CONF_PROVIDE_TYPE_UINT32 \
-DOSE_CONF_PROVIDE_TYPE_INT64 \
-DOSE_CONF_PROVIDE_TYPE_UINT64 \
-DOSE_CONF_PROVIDE_TYPE_TIMETAG \
-DOSE_CONF_PROVIDE_TYPE_TRUE \
-DOSE_CONF_PROVIDE_TYPE_FALSE \
-DOSE_CONF_PROVIDE_TYPE_NULL \
-DOSE_CONF_PROVIDE_TYPE_INFINITUM \
-Wno-string-plus-int

%: CFLAGS=$(CFLAGS_TEST_DEBUG)
%: $(CFILES)
	clang $(CFLAGS) -o $@ $(TEST_DIR)/$@.c \
	$(LIBOSE_CFILES)

.PHONY: all clean
all: $(TESTS) common.h ut_common.h
clean:
	$(if $(UNIT_TEST_TARGETS),rm -rf $(UNIT_TEST_TARGETS),)
