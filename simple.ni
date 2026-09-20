(sp SYSTEM_TABLE
    ^64 Signature
    ^32 Revision
    ^32 HeaderSize
    ^32 CRC32
    ^32 Reserved

    ^64 PFirmwareVendor
    ^32 FirmwareRevision
    ^32 Padding

    ^64 PConInHandle
    ^64 PsConIn
    ^64 PConOutHandle
    ^64 PsConOut
    ^64 PStdErrHandle
    ^64 PsStdErr

    ^64 PsRuntimeServices
    ^64 PsBootServices)

(sp BOOT_SERVICES
    ^64 Signature
    ^32 Revision
    ^32 HeaderSize
    ^32 CRC32
    ^32 Reserved

    # i didn't want to write the normal efi function names as it would take years

    ^64 something1
    ^64 something2
    ^64 something3
    ^64 something4
    ^64 something5
    ^64 something6
    ^64 something7
    ^64 something8
    ^64 something9
    ^64 something10
    ^64 something11
    ^64 something12
    ^64 something13
    ^64 something14
    ^64 something15
    ^64 something16
    ^64 something17
    ^64 something18
    ^64 something19
    ^64 something20
    ^64 something21
    ^64 something22
    ^64 something23
    ^64 something24
    ^64 something25
    ^64 something26
    ^64 something27
    ^64 something28

    ^64 FStall)

# Simple Text Output Protocol
(sp STOP
    ^64 FReset
    ^64 FOutputString)

(df main [^64 psImageHandle; ^64 psSystemTable]
    # print the firmware vendor
    (ca
	(rsea STOP FOutputString
	      (rsea SYSTEM_TABLE PsConOut psSystemTable))
	(rsea SYSTEM_TABLE PsConOut psSystemTable)
	(rsea SYSTEM_TABLE PFirmwareVendor psSystemTable))

    # wait 5 seconds (5000000 microseconds)
    (ca
	(rsea BOOT_SERVICES FStall
	      (rsea SYSTEM_TABLE PsBootServices psSystemTable))
	5000000)
    (ret))

(df printf_no_scam []
    (ret) # return 2 times to make sure I won't do too much work
    (ret))

# to make me not use 1000 syntaxes
# kinda inspired by efi docs
# sp					-> THIS_CASE
# func					-> ThisCase
# var					-> thisCase
# selem	(Struct element)		-> ThisCase
# vars meant to be used as addresses	-> prefix(p)
# aas (address as struct)		-> prefix(ps)
# vars / selems meant to be called	-> prefix(f)