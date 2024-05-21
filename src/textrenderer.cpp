#include <iostream>
#include <string>
#include <sys/stat.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rgbimage.hpp"
#include "textrenderer.hpp"


textrenderer::textrenderer() {
    fontsize = 80;
	
    text = "";
}

void textrenderer::setFontFilename(const std::string& fontfilename) {
	struct stat buffer;   
	if (stat (fontfilename.c_str(), &buffer) == 0) { 
		this->font_filename = fontfilename;
	} else {
		std::cout << "File not found: " << fontfilename << std::endl;
	}
}

textrenderer::~textrenderer() {
	this->font_filename.erase();
}

void draw_bitmap(FT_Bitmap* ft_bitmap, int x, int y, rgbimage* img)
{
	uint8_t color[3];
	for (unsigned int j=0;j<ft_bitmap->rows;j++) {
		for (unsigned int k=0;k<ft_bitmap->width;k++) {
			unsigned char alpha = ft_bitmap->buffer[k+j*ft_bitmap->width];
			if (alpha>0) {
				memset(color, ft_bitmap->buffer[k+j*ft_bitmap->width], 3);
                img->putpixel(x+k, y+j, color);
			}
		}
	}
}

rgbimage *textrenderer::render() const
{
	FT_Library library;
	FT_Face face;

	FT_GlyphSlot slot;
	int pen_x, pen_y;

    int min_x = 10000;
    int max_x = 0;

    int min_y = 10000;
    int max_y = 0;

    int max_top = 0;

	if (font_filename.length() == 0) { return NULL; }
	
	int error = FT_Init_FreeType(&library);
	if (error) {
		printf("FT error");
	}

	error = FT_New_Face(library, font_filename.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		printf("FT error");
	} else if (error) {
		printf("FT error");
	}

	slot = face->glyph;
	FT_Set_Pixel_Sizes(face, fontsize, 0);

	pen_x = 0;
	pen_y = 0;

	for (unsigned int n=0;n<(text.length());n++) {
		FT_UInt  glyph_index;

		// retrieve glyph index from character code
		glyph_index = FT_Get_Char_Index(face, text[n]);

		// load glyph image into the slot (erase previous one)
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			continue;

		// convert to an anti-aliased bitmap
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
			continue;

		// now, draw to our target surface
        FT_Bitmap* ft_bitmap = &slot->bitmap;

        int next_x = pen_x + slot->bitmap_left + ft_bitmap->width;
        int next_y = pen_y - slot->bitmap_top + ft_bitmap->rows;

        int next_top = slot->bitmap_top;

		if ( next_x > max_x) {
			max_x = next_x;
		}
		if ( next_x < min_x) {
			min_x = next_x;
		}

		if ( next_y > max_y) {
			max_y = next_y;
		}
		if ( next_y < min_y) {
			min_y = next_y;
		}

		if ( next_top > max_top) {
			max_top = next_top;
		}

        // increment pen position
		pen_x += slot->advance.x >> 6;
		pen_y += slot->advance.y >> 6;
	}

    ///////////////////////////

	pen_x = 0;
	pen_y = 0;

	rgbimage *output = new rgbimage(max_x, max_y + max_top);

	for (unsigned int n=0;n<(text.length());n++) {
		FT_UInt  glyph_index;

		// retrieve glyph index from character code
		glyph_index = FT_Get_Char_Index(face, text[n]);

		// load glyph image into the slot (erase previous one)
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			continue;

		// convert to an anti-aliased bitmap
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
			continue;

		// now, draw to our target surface

        FT_Bitmap* ft_bitmap = &slot->bitmap;
		draw_bitmap( ft_bitmap,
						pen_x + slot->bitmap_left,
						pen_y - slot->bitmap_top + max_top,
						output);

		// increment pen position
		pen_x += slot->advance.x >> 6;
		pen_y += slot->advance.y >> 6;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return output;
}

rgbimage *textrenderer::render_with_shadow() const
{
	rgbimage *text = render();

	if (text == NULL) {
		return NULL;
	}

	rgbimage *blurtext = render();

	if (blurtext == NULL) {
		return NULL;
	}

    blurtext->expand(5);
    blurtext->gaussianblur(3.0);
    blurtext->addalpha(0);
    blurtext->visible_to_alpha();
    blurtext->setcolor(0,0,0);

	blurtext->paste(
		text, 
		(blurtext->get_w()-text->get_w())/2, 
		(blurtext->get_h()-text->get_h())/2
	);

    blurtext->autocrop();

	delete text;
	return blurtext;
}
