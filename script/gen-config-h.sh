echo "#ifndef __RTE_CONFIG_H"
echo "#define __RTE_CONFIG_H"
grep CONFIG_ $1 |
grep -v '^[ \t]*#' |
sed 's,CONFIG_\(.*\)=y.*$,#undef \1\
#define \1 1,' |
sed 's,CONFIG_\(.*\)=n.*$,#undef \1,' |
sed 's,CONFIG_\(.*\)=\(.*\)$,#undef \1\
#define \1 \2,' |
sed 's,\# CONFIG_\(.*\) is not set$,#undef \1,'
echo "#endif /* __RTE_CONFIG_H */"
