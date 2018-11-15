</$objtype/mkfile

TARG=writer10 reader10 brdstr10 pwriter10
HFILES=
OFILES=
BIN=$home/bin/$objtype
UPDATE=mkfile $HFILES $OFILES ${TARG:%=%.c}
</sys/src/cmd/mkmany
