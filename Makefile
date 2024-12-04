game: game.c
    gcc -o game main.c game.c object.c network.c -lSDL2 -lncurses -pthread

clean:
    rm game
    
#./game -s -a 192.168.122.1
#./game -c -a 192.168.122.1