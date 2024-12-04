game: game.c
    gcc -o game main.c game.c object.c network.c -lncurses

clean:
    rm game
    
#./game -s -a 192.168.122.1
#./game -c -a 192.168.122.1