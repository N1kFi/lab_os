gcc parent.c -o parent \
gcc child.c -o child

./parent

strace ./parent
