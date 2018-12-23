ifeq ($(LIB_UIP), yes)
export LIBS += -luip
export INCS += -I$(ROOT)/library/uIP/uip -I$(ROOT)/library/uIP/uip-funs
endif

ifeq ($(LIB_FATFS), yes)
export LIBS += -lfatfs
export INCS += -I$(ROOT)/library/FatFs
endif

ifeq ($(LIB_CJSON), yes)
export LIBS += -lcjson
export INCS += -I$(ROOT)/library/cJSON
endif
