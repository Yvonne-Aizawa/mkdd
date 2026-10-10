.include "macros.inc"

.text
.balign 4

.global TRKAccessFile
TRKAccessFile:
    twui r0, 0x0
    blr

.global TRKOpenFile
TRKOpenFile:
    twui r0, 0x0
    blr

.global TRKCloseFile
TRKCloseFile:
    twui r0, 0x0
    blr

.global TRKPositionFile
TRKPositionFile:
    twui r0, 0x0
    blr
