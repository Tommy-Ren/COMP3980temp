game: game.c
    gcc -o game main.c game.c object.c network.c -lSDL2 -lncurses -pthread

clean:
    rm game
    
#./game -s -i kb|js|rd -n 192.168.1.104
#./game -s -i kb -n 192.168.122.1
#./game -c -i kb|js|rd -n 192.168.1.104
#./game -c -i kb -n 192.168.122.1