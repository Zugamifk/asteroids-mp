int gamestate_add_player(int id) {
	object *ship;
	ship = &(ships[id]);
	ship->state = 0;
}

int gamestate_encode( char **buffer, int *len ) {

}
