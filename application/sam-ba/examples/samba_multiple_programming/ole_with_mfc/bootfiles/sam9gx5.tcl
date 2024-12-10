# ----------------------------------------------------------------------------
#         ATMEL Microcontroller Software Support 
# ----------------------------------------------------------------------------
# Copyright (c) 2012, Atmel Corporation
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice,
# this list of conditions and the disclaimer below.
#
# Atmel's name may not be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
# DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

################################################################################
#  Main script: Load the demo in Serial Flash
################################################################################
global target
set bootstrapFile "bootfiles/example.bin"

## Download the binary file and verify the programming
puts "-I- === Initialize the SerialFlash access ==="
SERIALFLASH::Init 0
GENERIC::SendFile $bootstrapFile 0
set n [expr { int(10000000 * rand()) }]
set ReceivefileName "bootfiles/temp$n.bin"
set fileSize [file size $bootstrapFile]
GENERIC::ReceiveFile $ReceivefileName 0 $fileSize
set returnCompare [TCL_Compare $bootstrapFile $ReceivefileName]
file delete $ReceivefileName

if {$returnCompare == 1} {
        puts "-I- Sent file do not match!\n"
} else {
        puts "-I- Sent file match exactly!\n"
}

## Turn on a LED
TCL_Write_Int $target(handle) 0x40000 0xFFFFF600
TCL_Write_Int $target(handle) 0x40000 0xFFFFF610
