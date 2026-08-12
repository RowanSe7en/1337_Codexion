#!/bin/sh

# --------------------easy-----------------------
# MUST burn out
# ./codexion 1 800 200 200 200 10 0 fifo
# ./codexion 5 2000 200 200 200 10 0 fifo
./codexion 5 2000 200 200 200 7 0 edf
# ./codexion 200 2000 200 200 200 7 0 edf

# --------------------Less easy-----------------------
# MUST burn out
# ./codexion 5 500 200 200 200 10 0 fifo

# --------------------Medium-----------------------

# ./codexion 5 3000 200 200 200 10 400 fifo
# ./codexion 5 3000 200 200 200 10 800 fifo
# ./codexion 5 3100 200 200 200 10 800 edf

#------------------------------------------------------------

# valgrind --tool=helgrind

# --------------------easy-----------------------
# MUST burn out
# valgrind --tool=helgrind ./codexion 1 800 200 200 200 10 0 fifo
# valgrind --tool=helgrind ./codexion 5 2000 200 200 200 10 0 fifo
# valgrind --tool=helgrind ./codexion 5 2000 200 200 200 7 0 edf

# --------------------Less easy-----------------------
# MUST burn out
# valgrind --tool=helgrind ./codexion 5 500 200 200 200 10 0 fifo

# --------------------Medium-----------------------

# valgrind --tool=helgrind ./codexion 5 3000 200 200 200 10 400 fifo
# valgrind --tool=helgrind ./codexion 5 3000 200 200 200 10 800 fifo
# valgrind --tool=helgrind ./codexion 5 3100 200 200 200 10 800 edf

#------------------------------------------------------------

# valgrind --tool=drd

# --------------------easy-----------------------
# MUST burn out
# valgrind --tool=drd ./codexion 1 800 200 200 200 10 0 fifo
# valgrind --tool=drd ./codexion 5 2000 200 200 200 10 0 fifo
# valgrind --tool=drd ./codexion 5 2000 200 200 200 7 0 edf

# --------------------Less easy-----------------------
# MUST burn out
# valgrind --tool=drd ./codexion 5 500 200 200 200 10 0 fifo

# --------------------Medium-----------------------

# valgrind --tool=drd ./codexion 5 3000 200 200 200 10 400 fifo
# valgrind --tool=drd ./codexion 5 3000 200 200 200 10 800 fifo
# valgrind --tool=drd ./codexion 5 3100 200 200 200 10 800 edf
