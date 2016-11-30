all: http-server.c
	@make clean
	gcc -o http-server http-server.c
	@echo "server executable is ready to use\n"
	@echo "Server Usage: ./http-server [-p port-number] [-b base_directory]\n"
	@echo "\nIMPORTANT: Base directory is default set to webfiles folder present in this same folder.\n"
	@echo "Arguments are optional\n"

clean:
	rm -f http-server
	@echo "Executables are removed\n"
