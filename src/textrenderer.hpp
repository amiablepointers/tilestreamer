#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

class textrenderer {
	private:
		int fontsize;
		std::string font_filename;
		std::string text;

	public:
		textrenderer();
		~textrenderer();
		
		rgbimage *render() const;
		rgbimage *render_with_shadow() const;

		void setText(const std::string& text) {
			this->text = text; 
		};
		void setFontFilename(const std::string& fontfilename);
		void setFontSize(const int& fontsize) { 
			this->fontsize = fontsize; 
		};
};

#endif // TEXTRENDERER_H
