# ----------------------------------------------------------------------------
#         ATMEL Microcontroller Software Support 
# ----------------------------------------------------------------------------
# Copyright (c) 2011, Atmel Corporation
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
#  Main script: Load a example file in NAND Flash
################################################################################
set exampleFile	"../images/example.bin"

## Falshing binaries
puts "-I- === Initialize the NAND access ==="
NANDFLASH::Init

puts "-I- === Enable PMECC OS Parameters ==="
NANDFLASH::NandHeaderValue HEADER 0xc0c00405

puts "-I- === Erase all the NAND flash blocs and test the erasing ==="
NANDFLASH::EraseAllNandFlash

puts "-I- === Load the example image ==="
send_file {NandFlash} "$exampleFile" 0x00200000 0

puts "-I- === Compare the example image ==="
compare_file  {NandFlash} "$exampleFile" 0x00200000 0


puts "-I------------------------------------------------------"
puts "-I- Exemple : Nandflash script file correctly executed !"
puts "-I------------------------------------------------------"


