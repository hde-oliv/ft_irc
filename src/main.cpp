#include <cstdlib>

#include "Server.hpp"

int main( int argc, char *argv[] ) {
	if ( argc != 3 ) {
		return 1;
	}

	Server srv( argv[1], std::atoi( argv[2] ) );

	srv.startListening();
}
