BUILD_CPPFLAGS += \
	-std=c++0x \
	-DBOOST_PARAMETER_MAX_ARITY=6 \
	-I$(this_srcdir) -I$(top_srcdir)
