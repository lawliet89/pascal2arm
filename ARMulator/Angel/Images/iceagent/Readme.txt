This directory contains two files for the EmbeddedICE interface box. 
 
The first, iceagent.rom is a new version of the EmbeddedICE software "agent" 
compatible with the SDT2.1 default remote communication protocol, ADP. This 
file may be downloaded to the EmbeddedICE from the debugger using Load 
Agent, or programmed into ROM and installed in the EmbeddedICE. When using 
this software in the EmbeddedICE, you configure the ARM Debugger for Windows 
to connect to the target using the "Remote_A" option. The UNIX debugger 
"armsd" defaults to communication via ADP. 
 
The adpjtag.rom file allows the EmbeddedICE unit to be used to communicate 
with the Angel Debug Monitor on a target via the JTAG port. See the relevant 
Angel documentation on "ADP over JTAG". 
