##############################################################################
## File: README.txt
##
## Purpose: CFS DS application unit test instructions, results, and coverage
##############################################################################

-------------------------
DS Unit Test Instructions
-------------------------
This unit test was run in a virtual machine running Ubuntu 18.04 and uses the
ut-assert stubs and default hooks for the cFE, OSAL and PSP.

To run the unit test enter the following commands at the command line prompt in
the top-level cFS directory (after ensuring that DS and cfs_lib are listed as
targets).  Note that in order to successfully compile the unit tests the
"-Werror" compilation flag must be disabled.

make distclean
make SIMULATION=native ENABLE_UNIT_TESTS=true prep
make
make test
make lcov

DS 2.6.0 Unit Test Results:

Tests Executed:    246
Assert Pass Count: 1090
Assert Fail Count: 0

==========================================================================
ds_app.c - Line Coverage:       98.9%
           Function Coverage:  100.0%
           Branch Coverage:     98.6%

Line and branch coverage gaps are caused by a current inability to force
the return value from snprintf.

==========================================================================
ds_cmds.c - Line Coverage:     100.0%
            Function Coverage: 100.0%
            Branch Coverage:   100.0%

==========================================================================
ds_file.c - Line Coverage:     100.0%
            Function Coverage: 100.0%
            Branch Coverage:    96.3%

Branch coverage gap is caused by an inability to force certain length
combinations in DS_FileCreateName.  This can be solved when we are able
to force the return value of strlen.

==========================================================================
ds_table.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:    99.5%

Branch coverage gap is due to an unreachable condition in DS_TableFindMsgID.

==========================================================================
