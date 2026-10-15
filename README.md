# core Flight System (cFS) Data Storage Application (DS)

## Introduction

The Data Storage application (DS) is a core Flight System (cFS) application 
that is a plug in to the Core Flight Executive (cFE) component of the cFS.  
The DS application is used for storing software bus messages in files. These 
files are generally stored on a storage device such as a solid state recorder 
but they could be stored on any file system. Another cFS application such as 
CFDP (CF) must be used in order to transfer the files created by DS from 
their onboard storage location to where they will be viewed and processed.

The DS application is written in C and depends on the cFS Operating System
Abstraction Layer (OSAL) and cFE components.  There is additional DS application
specific configuration information contained in the application user's guide.

Developer's guide information can be generated using Doxygen:
```
  make prep
  make -C build/docs/ds-usersguide ds-usersguide
```

## Software Required

cFS Framework (cFE, OSAL, PSP)

A demonstration bundle of the Core Flight System including the cFE, OSAL, and PSP can be obtained at https://github.com/nasa/cfs

For information about a mission ready cFS bundle, see: https://github.com/nasa/cFS#cfs-gov-mission-ready-version

## Known issues

See all [open issues](https://github.com/nasa/DS/issues) and closed to milestones later than this version.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
