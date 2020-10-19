PROC scx_cpu1_ds_badfiletbls
;*******************************************************************************
;  Test Name:  ds_badfiletbls
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates seven CFS Data Storage (DS) Destination File 
;	Table load images each containing one of the possible seven content 
;	errors such that the table validation will fail and generate the proper
;	error event message. This table image is used in the ResetNoCDS test 
;	procedure.
;	
;  Prerequisite Conditions
;	None.
;
;  Assumptions and Constraints
;	None.
;
;  Change History
;	Date		   Name		Description
;	11/24/09	Walt Moleski	Inital implemetation.
;	12/08/10	Walt Moleski	Modified to use the table name specified
;					in the ds_platform_cfg.h file and added
;					a variable for the application name.
;	03/19/13	Walt Moleski	Added 2 new cases to test an invalid
;					sequence count for both types
;       01/31/17	Walt Moleski	Updated for DS 2.5.0.0 using CPU1 for
;					commanding and added a hostCPU variable
;					for the utility procs to connect to the
;					proper host IP address.
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
local DSAppName = "DS"
local fileTblName = DSAppName & "." & DS_DESTINATION_TBL_NAME
local hostCPU = "CPU3"

;; Setup the Destination File Table
;; The Description below is too long
SCX_CPU1_DS_DF_TBL_Description  = "Reset No Critical Data Store Test"

;; Entry 1 - Null Path
SCX_CPU1_DS_DF_TBL[0].Pathname  = ""
SCX_CPU1_DS_DF_TBL[0].Basename  = "seq1"
SCX_CPU1_DS_DF_TBL[0].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[0].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[0].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[0].FileSize = 1024
SCX_CPU1_DS_DF_TBL[0].FileAge = 600
SCX_CPU1_DS_DF_TBL[0].SeqCnt = 100

;; Entry 2 - Invalid FileNameType
SCX_CPU1_DS_DF_TBL[1].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[1].Basename  = "seq2"
SCX_CPU1_DS_DF_TBL[1].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[1].FileNameType = 3
SCX_CPU1_DS_DF_TBL[1].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[1].FileSize = 1024
SCX_CPU1_DS_DF_TBL[1].FileAge = 600
SCX_CPU1_DS_DF_TBL[1].SeqCnt = 200

;; Entry 3 - Invalid FileState
SCX_CPU1_DS_DF_TBL[2].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[2].Basename  = "seq3"
SCX_CPU1_DS_DF_TBL[2].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[2].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[2].FileState = 2
SCX_CPU1_DS_DF_TBL[2].FileSize = 1024
SCX_CPU1_DS_DF_TBL[2].FileAge = 600
SCX_CPU1_DS_DF_TBL[2].SeqCnt = 300

;; Entry 4 - Invalid File size (less than minimum)
SCX_CPU1_DS_DF_TBL[3].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[3].Basename  = "seq4"
SCX_CPU1_DS_DF_TBL[3].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[3].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[3].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[3].FileSize = 512
SCX_CPU1_DS_DF_TBL[3].FileAge = 600
SCX_CPU1_DS_DF_TBL[3].SeqCnt = 400

;; Entry 5 - Invalid File Age (less than minimum)
SCX_CPU1_DS_DF_TBL[4].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[4].Basename  = "time1"
SCX_CPU1_DS_DF_TBL[4].Extension = ""
SCX_CPU1_DS_DF_TBL[4].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[4].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[4].FileSize = 1024
SCX_CPU1_DS_DF_TBL[4].FileAge = 30
SCX_CPU1_DS_DF_TBL[4].SeqCnt = 0

;; Entry 6 - Invalid Extension text length
SCX_CPU1_DS_DF_TBL[5].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[5].Basename  = "time2"
SCX_CPU1_DS_DF_TBL[5].Extension = ".thisismorethan8chars"
SCX_CPU1_DS_DF_TBL[5].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[5].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[5].FileSize = 1024
SCX_CPU1_DS_DF_TBL[5].FileAge = 60
SCX_CPU1_DS_DF_TBL[5].SeqCnt = 0

;; Entry 7 - Invalid sequence count on a Time entry
SCX_CPU1_DS_DF_TBL[6].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[6].Basename  = "time3"
SCX_CPU1_DS_DF_TBL[6].Extension = ""
SCX_CPU1_DS_DF_TBL[6].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[6].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[6].FileSize = 1024
SCX_CPU1_DS_DF_TBL[6].FileAge = 60
SCX_CPU1_DS_DF_TBL[6].SeqCnt = DS_MAX_SEQUENCE_COUNT + 1

;; Entry 8
SCX_CPU1_DS_DF_TBL[7].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[7].Basename  = "time4"
SCX_CPU1_DS_DF_TBL[7].Extension = ""
SCX_CPU1_DS_DF_TBL[7].FileNameType = DS_BY_TIME
SCX_CPU1_DS_DF_TBL[7].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[7].FileSize = 1024
SCX_CPU1_DS_DF_TBL[7].FileAge = 60
SCX_CPU1_DS_DF_TBL[7].SeqCnt = 0

;; Entry 9 - Invalid Sequence Count
SCX_CPU1_DS_DF_TBL[8].Pathname  = "/ram/"
SCX_CPU1_DS_DF_TBL[8].Basename  = "seq5"
SCX_CPU1_DS_DF_TBL[8].Extension = ".dat"
SCX_CPU1_DS_DF_TBL[8].FileNameType = DS_BY_COUNT
SCX_CPU1_DS_DF_TBL[8].FileState = DS_ENABLED
SCX_CPU1_DS_DF_TBL[8].FileSize = 1024
SCX_CPU1_DS_DF_TBL[8].FileAge = 600
SCX_CPU1_DS_DF_TBL[8].SeqCnt = DS_MAX_SEQUENCE_COUNT + 1

local maxEntry = DS_DEST_FILE_CNT - 1
local endmnemonic = "SCX_CPU1_DS_DF_TBL[8].SeqCnt"

s create_tbl_file_from_cvt(hostCPU,fileAppid,"Invalid File Table Description","ds_badfile1.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

;; Make the Table Description valid
SCX_CPU1_DS_DF_TBL_Description  = "Reset No CDS Test"

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File Table Null Path", "ds_badfile2.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[0].Pathname  = "/ram/"

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File Table Name Type", "ds_badfile3.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[1].FileNameType = DS_BY_COUNT

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File State Type", "ds_badfile4.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[2].FileState = DS_ENABLED

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File Table Max Size", "ds_badfile5.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[3].FileSize = 1024

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File Table Max Age", "ds_badfile6.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[4].FileAge = 60

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid File Table Extension", "ds_badfile7.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[5].Extension = ""

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid Sequence Num1", "ds_badfile8.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

SCX_CPU1_DS_DF_TBL[6].SeqCnt = 0

s create_tbl_file_from_cvt(hostCPU, fileAppid, "Invalid Sequence Num2", "ds_badfile9.tbl",fileTblName,"SCX_CPU1_DS_DF_TBL_Description",endmnemonic)

%liv (log_procedure) = logging

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_ds_badfiletbls"
write ";*********************************************************************"
ENDPROC
