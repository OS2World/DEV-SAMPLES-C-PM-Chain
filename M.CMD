make %1.mak > %1.err
send "compile all done" to karln
type %1.err | more
