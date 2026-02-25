// 'prompt', 7x14px
static const unsigned char epd_bitmap_prompt [] PROGMEM = {
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 32)
static const int epd_bitmap_allArray_LEN = 1;
static const unsigned char* epd_bitmap_allArray[1] = {
	epd_bitmap_prompt
};
