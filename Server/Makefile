TARGET = Server

CC = gcc

$(TARGET): 
	$(CC) Server.c ServerFunctions.c -o $(TARGET)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET) $(args)