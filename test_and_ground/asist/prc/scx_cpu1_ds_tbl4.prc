PROC scx_cpu1_ds_tbl4
;*******************************************************************************
;  Test Name:  ds_tbl4
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates the initial CFS Data Storage (DS) Destination
;	File and Packet Filter Table load image files. These files are currently
;	used by the MoveFile test procedure.
;	
;  Prerequisite Conditions
;	None.
;
;  Assumptions and Constraints
;	None.
;
;  Change History
;
;	Date		   Name		Description
;	01/03/11	Walt Moleski	Inital implemetation.
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
local seqFileMID = 0x0900
local timeFileMID = 0x0901
local oneFileMID = 0x0902
local xFileMID = 0x0903
local seqTimeFileMID = 0x0904
local timeSeqFileMID = 0x0905
local DSAppName = "DS"   
local fileTblName = DSAppName & "." & DS_DESTINATION_TBL_NAME
local filterTblName = DSAppName & "." & DS_FILTER_TBL_NAME
local hostCPU = "CPU3"

;; Setup the Destination File Table
SCX_CPU1_DS_DF_TBL_Description  = "Move File Test"

;; Entry 1
SCX_CPU1_DS_DF_TBL[0].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[0].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[0].Basename  = "seq1"
SCX_CPU1_DS_DF_TBL[0].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[0].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[0].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[0].FileSize = 1024
SCX_CPU1_DS_DF_TBL[0].FileAge = 100
SCX_CPU1_DS_DF_TBL[0].SeqCnt = 100

;; Entry 2
SCX_CPU1_DS_DF_TBL[1].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[1].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[1].Basename  = "seq2"
SCX_CPU1_DS_DF_TBL[1].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[1].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[1].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[1].FileSize = 1024
SCX_CPU1_DS_DF_TBL[1].FileAge = 100
SCX_CPU1_DS_DF_TBL[1].SeqCnt = 200

;; Entry 3
SCX_CPU1_DS_DF_TBL[2].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[2].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[2].Basename  = "seq3"
SCX_CPU1_DS_DF_TBL[2].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[2].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[2].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[2].FileSize = 1024
SCX_CPU1_DS_DF_TBL[2].FileAge = 100
SCX_CPU1_DS_DF_TBL[2].SeqCnt = 300

;; Entry 4
SCX_CPU1_DS_DF_TBL[3].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[3].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[3].Basename  = "seq4"
SCX_CPU1_DS_DF_TBL[3].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[3].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[3].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[3].FileSize = 1024
SCX_CPU1_DS_DF_TBL[3].FileAge = 100
SCX_CPU1_DS_DF_TBL[3].SeqCnt = 400

;; Entry 5
SCX_CPU1_DS_DF_TBL[4].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[4].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[4].Basename  = "time1"
SCX_CPU1_DS_DF_TBL[4].Extension = ""
SCX_CPU1_DS_DF_TBL[4].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[4].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[4].FileSize = 1024
SCX_CPU1_DS_DF_TBL[4].FileAge = 60
SCX_CPU1_DS_DF_TBL[4].SeqCnt = 0

;; Entry 6
SCX_CPU1_DS_DF_TBL[5].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[5].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[5].Basename  = "time2"
SCX_CPU1_DS_DF_TBL[5].Extension = ""
SCX_CPU1_DS_DF_TBL[5].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[5].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[5].FileSize = 1024
SCX_CPU1_DS_DF_TBL[5].FileAge = 60
SCX_CPU1_DS_DF_TBL[5].SeqCnt = 0

;; Entry 7
SCX_CPU1_DS_DF_TBL[6].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[6].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[6].Basename  = "time3"
SCX_CPU1_DS_DF_TBL[6].Extension = ""
SCX_CPU1_DS_DF_TBL[6].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[6].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[6].FileSize = 1024
SCX_CPU1_DS_DF_TBL[6].FileAge = 60
SCX_CPU1_DS_DF_TBL[6].SeqCnt = 0

;; Entry 8
SCX_CPU1_DS_DF_TBL[7].MoveDirName = "/ram/movedir/"
SCX_CPU1_DS_DF_TBL[7].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[7].Basename  = "time4"
SCX_CPU1_DS_DF_TBL[7].Extension = ""
SCX_CPU1_DS_DF_TBL[7].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[7].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[7].FileSize = 1024
SCX_CPU1_DS_DF_TBL[7].FileAge = 60
SCX_CPU1_DS_DF_TBL[7].SeqCnt = 0

local maxEntry = DS_DEST_FILE_CNT - 1

;; Clear the remaining entries in the table
;;for i = 8 to maxEntry do
;;  SCX_CPU1_DS_DF_TBL[i].Pathname  = ""
;;  SCX_CPU1_DS_DF_TBL[i].Basename  = ""
;;  SCX_CPU1_DS_DF_TBL[i].Extension = ""
;;  SCX_CPU1_DS_DF_TBL[i].FileNameType = 0
;;  SCX_CPU1_DS_DF_TBL[i].FileState = 0
;;  SCX_CPU1_DS_DF_TBL[i].FileSize = 0
;;  SCX_CPU1_DS_DF_TBL[i].FileAge = 0
;;  SCX_CPU1_DS_DF_TBL[i].SeqCnt = 0
;;enddo

;;local endmnemonic = "SCX_CPU1_DS_DF_TBL[" & maxEntry & "].SeqCnt"
local endmnemonic = "SCX_CPU1_DS_DF_TBL[7].SeqCnt"

s create_tbl_file_from_cvt(hostCPU,fileAppid,"Move File Test File Table","ds_movefile.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

;; Setup the Packet Filter Table
SCX_CPU1_DS_PF_TBL_Description  = "Move File Test"
;; Entry 1
SCX_CPU1_DS_PF_TBL[0].MessageID = seqFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].Index = 0
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[0].O_Value = 0
;; File 2
SCX_CPU1_DS_PF_TBL[0].FilterParams[1].Index = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[1].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[0].FilterParams[1].N_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[1].X_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[1].O_Value = 0
;; File 3
SCX_CPU1_DS_PF_TBL[0].FilterParams[2].Index = 2
SCX_CPU1_DS_PF_TBL[0].FilterParams[2].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[0].FilterParams[2].N_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[2].X_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[2].O_Value = 0
;; File 4
SCX_CPU1_DS_PF_TBL[0].FilterParams[3].Index = 3
SCX_CPU1_DS_PF_TBL[0].FilterParams[3].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[0].FilterParams[3].N_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[3].X_Value = 1
SCX_CPU1_DS_PF_TBL[0].FilterParams[3].O_Value = 0

;; Entry 2
SCX_CPU1_DS_PF_TBL[1].MessageID = timeFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].Index = 4
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].FilterType = DS_BY_TIME
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[0].O_Value = 0
;; File 2
SCX_CPU1_DS_PF_TBL[1].FilterParams[1].Index = 5
SCX_CPU1_DS_PF_TBL[1].FilterParams[1].FilterType = DS_BY_TIME
SCX_CPU1_DS_PF_TBL[1].FilterParams[1].N_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[1].X_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[1].O_Value = 0
;; File 3
SCX_CPU1_DS_PF_TBL[1].FilterParams[2].Index = 6
SCX_CPU1_DS_PF_TBL[1].FilterParams[2].FilterType = DS_BY_TIME
SCX_CPU1_DS_PF_TBL[1].FilterParams[2].N_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[2].X_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[2].O_Value = 0
;; File 4
SCX_CPU1_DS_PF_TBL[1].FilterParams[3].Index = 7
SCX_CPU1_DS_PF_TBL[1].FilterParams[3].FilterType = DS_BY_TIME
SCX_CPU1_DS_PF_TBL[1].FilterParams[3].N_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[3].X_Value = 1
SCX_CPU1_DS_PF_TBL[1].FilterParams[3].O_Value = 0

;; Entry 3
SCX_CPU1_DS_PF_TBL[2].MessageID = oneFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[2].FilterParams[0].Index = 0
SCX_CPU1_DS_PF_TBL[2].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[2].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[2].FilterParams[0].X_Value = 3
SCX_CPU1_DS_PF_TBL[2].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[2].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[2].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[2].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[2].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[2].FilterParams[j].O_Value = DS_UNUSED
enddo

;; Entry 4
SCX_CPU1_DS_PF_TBL[3].MessageID = xFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[3].FilterParams[0].Index = 1
SCX_CPU1_DS_PF_TBL[3].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[3].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[3].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[3].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[3].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[3].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[3].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[3].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[3].FilterParams[j].O_Value = DS_UNUSED
enddo

;; Entry 5
SCX_CPU1_DS_PF_TBL[4].MessageID = seqTimeFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[4].FilterParams[0].Index = 4
SCX_CPU1_DS_PF_TBL[4].FilterParams[0].FilterType = DS_BY_COUNT
SCX_CPU1_DS_PF_TBL[4].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[4].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[4].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[4].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[4].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[4].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[4].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[4].FilterParams[j].O_Value = DS_UNUSED
enddo

;; Entry 6
SCX_CPU1_DS_PF_TBL[5].MessageID = timeSeqFileMID
;; File 1
SCX_CPU1_DS_PF_TBL[5].FilterParams[0].Index = 1
SCX_CPU1_DS_PF_TBL[5].FilterParams[0].FilterType = DS_BY_TIME
SCX_CPU1_DS_PF_TBL[5].FilterParams[0].N_Value = 1
SCX_CPU1_DS_PF_TBL[5].FilterParams[0].X_Value = 1
SCX_CPU1_DS_PF_TBL[5].FilterParams[0].O_Value = 0
for j = 1 to DS_FILTERS_PER_PACKET-1 do
  SCX_CPU1_DS_PF_TBL[5].FilterParams[j].Index = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[5].FilterParams[j].FilterType = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[5].FilterParams[j].N_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[5].FilterParams[j].X_Value = DS_UNUSED
  SCX_CPU1_DS_PF_TBL[5].FilterParams[j].O_Value = DS_UNUSED
enddo

maxEntry = DS_PACKETS_IN_FILTER_TABLE - 1
local maxFilterParams = DS_FILTERS_PER_PACKET - 1

;; Loop for the remaining entries defined in the table
for entry = 6 to maxEntry do
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

s create_tbl_file_from_cvt(hostCPU,filterAppid,"Move File Test Filter Table","ds_movefilter.tbl",filterTblName,"SCX_CPU1_DS_PF_TBL_Description",endmnemonic)

%liv (log_procedure) = logging

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_ds_tbl4"
write ";*********************************************************************"
ENDPROC
