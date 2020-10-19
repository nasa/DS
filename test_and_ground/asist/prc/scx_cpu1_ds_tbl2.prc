PROC scx_cpu1_ds_tbl2
;*******************************************************************************
;  Test Name:  ds_tbl2
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates the initial CFS Data Storage (DS) Destination
;	File and Packet Filter Table load image files. These files are currently
;	used by the File Write test procedure.
;	
;  Prerequisite Conditions
;	None.
;
;  Assumptions and Constraints
;	None.
;
;  Change History
;	Date		   Name		Description
;	11/04/09	Walt Moleski	Inital implemetation.
;       12/08/10        Walt Moleski    Modified the procedure to use variables
;                                       for the application name and ram disk.
;       01/31/17        Walt Moleski    Updated for DS 2.5.0.0 using CPU1 for
;                                       commanding and added a hostCPU variable
;                                       for the utility procs to connect to the
;                                       proper host IP address.
;
;  Arguments
;	None.
;
;  Procedures Called
;	Name			Description
;
;  Expected Test Results and Analysis
;
;**********************************************************************

local logging = %liv (log_procedure)
%liv (log_procedure) = FALSE

#include "ut_statusdefs.h"
#include "ut_cfe_info.h"
#include "cfe_platform_cfg.h"
#include "ds_platform_cfg.h"
#include "ds_events.h"
#include "ds_appdefs.h"

write ";***********************************************************************"
write ";  Step 1.0: Define DS Destination File And Packet Filter Tables."
write ";***********************************************************************"
;; Setup the appid based upon the CPU being used
local fileAppid = 0x0F76
local filterAppid = 0x0F77
local seqFileMID = 0x1000
local timeFileMID = 0x1001
local DSAppName = "DS"  
local fileTblName = DSAppName & "." & DS_DESTINATION_TBL_NAME
local filterTblName = DSAppName & "." & DS_FILTER_TBL_NAME
local hostCPU = "CPU3"

;; Setup the Destination File Table
;; Entry 1
SCX_CPU1_DS_DF_TBL_Description  = "File Write Test"
SCX_CPU1_DS_DF_TBL[0].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[0].Basename  = "seq"
SCX_CPU1_DS_DF_TBL[0].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[0].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[0].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[0].FileSize = 1024
SCX_CPU1_DS_DF_TBL[0].FileAge = 600
SCX_CPU1_DS_DF_TBL[0].SeqCnt = 100

;; Entry 2
SCX_CPU1_DS_DF_TBL[1].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[1].Basename  = "ti"
SCX_CPU1_DS_DF_TBL[1].Extension = ".txt"
SCX_CPU1_DS_DF_TBL[1].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[1].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[1].FileSize = 1024
SCX_CPU1_DS_DF_TBL[1].FileAge = 60
SCX_CPU1_DS_DF_TBL[1].SeqCnt = 200

local maxEntry = DS_DEST_FILE_CNT - 1

;; Clear the remaining entries in the table
;;for i = 2 to maxEntry do
;;  SCX_CPU1_DS_DF_TBL[i].Pathname  = DS_EMPTY_STRING
;;  SCX_CPU1_DS_DF_TBL[i].Basename  = DS_EMPTY_STRING
;;  SCX_CPU1_DS_DF_TBL[i].Extension = DS_EMPTY_STRING
;;  SCX_CPU1_DS_DF_TBL[i].FileNameType = DS_UNUSED
;;  SCX_CPU1_DS_DF_TBL[i].FileState = DS_UNUSED
;;  SCX_CPU1_DS_DF_TBL[i].FileSize = DS_UNUSED
;;  SCX_CPU1_DS_DF_TBL[i].FileAge = DS_UNUSED
;;  SCX_CPU1_DS_DF_TBL[i].SeqCnt = DS_UNUSED
;;enddo

;;local endmnemonic = "SCX_CPU1_DS_DF_TBL[" & maxEntry & "].SeqCnt"
local endmnemonic = "SCX_CPU1_DS_DF_TBL[1].SeqCnt"

s create_tbl_file_from_cvt(hostCPU,fileAppid,"File Write Test File Table","ds_fwfile.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

;; Setup the Packet Filter Table
SCX_CPU1_DS_PF_TBL_Description  = "File Write Test"
;; Entry 1
SCX_CPU1_DS_PF_TBL[0].MessageID = seqFileMID
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].Index = 0
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[0].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[0].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[0].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[0].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[0].FilterParams[j].O_Value = DS_UNUSED
enddo

;; Entry 2
SCX_CPU1_DS_PF_TBL[1].MessageID = timeFileMID
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].Index = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[1].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[1].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[1].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[1].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[1].FilterParams[j].O_Value = DS_UNUSED
enddo

maxEntry = DS_PACKETS_IN_FILTER_TABLE - 1
local maxFilterParams = DS_FILTERS_PER_PACKET - 1

;; Loop for the remaining entries defined in the table
for entry = 2 to maxEntry do
  SCX_CPU1_DS_PF_TBL[entry].MessageID = DS_UNUSED
  for j = 0 to DS_FILTERS_PER_PACKET-1 do
    SCX_CPU1_DS_PF_TBL[entry].FilterParams[j].Index = DS_UNUSED
    SCX_CPU1_DS_PF_TBL[entry].FilterParams[j].FilterType = DS_UNUSED
    SCX_CPU1_DS_PF_TBL[entry].FilterParams[j].N_Value = DS_UNUSED
    SCX_CPU1_DS_PF_TBL[entry].FilterParams[j].X_Value = DS_UNUSED
    SCX_CPU1_DS_PF_TBL[entry].FilterParams[j].O_Value = DS_UNUSED
  enddo
enddo

endmnemonic = "SCX_CPU1_DS_PF_TBL[" & maxEntry & "].FilterParams[" & maxFilterParams & "].O_Value"

s create_tbl_file_from_cvt(hostCPU,filterAppid,"File Write Test Filter Table","ds_fwfilter.tbl",filterTblName,"SCX_CPU1_DS_PF_TBL_Description",endmnemonic)

%liv (log_procedure) = logging

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_ds_tbl2"
write ";*********************************************************************"
ENDPROC
