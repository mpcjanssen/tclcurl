# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded curl 0.1 \
	    "[list load [file join $dir libtcl9curl0.1.so] [string totitle curl]];
    	      [list source [file join $dir curl.tcl]]"
} else {
    package ifneeded curl 0.1 \
	    "[list load [file join $dir libcurl0.1.so] [string totitle curl]]
    	      [list source [file join $dir curl.tcl]]"
}
