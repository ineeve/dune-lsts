############################################################################
# This file is subject to the terms and conditions defined in file         #
# 'LICENCE.md', which is part of this source code package.                 #
############################################################################
# Tests if libcurl is present and can be used.                             #
############################################################################

dune_test_lib(EposCmd VCS_OpenDevice)

if(DUNE_SYS_HAS_LIB_EPOSCMD)
  set(DUNE_USING_EPOS 1 CACHE INTERNAL "EPOS CMD library")
else()
  set(DUNE_USING_EPOS 0 CACHE INTERNAL "EPOS CMD library")
endif()
