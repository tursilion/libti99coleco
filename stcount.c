// stcount function for compressed music player

// stcount - returns how many songs are in a pack
// inputs - pSong - pointer to song data
// returns - count (which is just the table pointers subtracted and divided)
unsigned char stcount(const void *pIn) {
	unsigned char *pSong = (unsigned char*)pIn;
	unsigned int i1,i2;

	// first pointer is pointer to stream tables
	// second pointer is pointer to note table
	// (note_table-stream_tables)/24 is number of songs
	i1 = (*(pSong++))<<8;
	i1 |= *(pSong++);
	i2 = (*(pSong++))<<8;
	i2 |= *(pSong);
	
	i2-=i1;		// get difference

#if 0
	// enable this if you'd rather pull in the division libraries
	i2 /= 24;
	return i2;
#else
	// slightly slower, but not likely by much
	i1 = 0;
	while (i2 >= 24) {
		i2-=24;
		i1++;
	}
	return i1;
#endif
}
