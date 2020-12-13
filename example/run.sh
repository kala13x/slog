make clean
make

valgrind --leak-check=full \
    --show-leak-kinds=all \
    --show-error-list=yes \
    --track-origins=yes \
    ./example
