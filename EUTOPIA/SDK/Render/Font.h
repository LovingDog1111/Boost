#pragma once
#include <string>
#include <MemoryUtil.h>
class Font {
	private:
		virtual void constructor();
		virtual void a1();
		virtual void a2();
		virtual void a3();
		virtual void a4();
		virtual void a5();
	public:
		virtual float getLineLength(std::string* str, float textSize, bool unknown);
		virtual float getLineHeight();
	};
	
	class FontRepos : public Font {
       public:
        CLASS_MEMBER(std::vector<std::shared_ptr<class Font>>, fontList, 0x40);

       public:
		   class Font* getSmoothFont() {
            return fontList[7].get();
		}
	};

	class FontHandler {
		public:
			CLASS_MEMBER(std::shared_ptr<FontRepos>, fontRepository, 0x18);
			CLASS_MEMBER(std::shared_ptr<Font>, defaultFont, 0x38);
			CLASS_MEMBER(unsigned int, fontId, 0x40);
			CLASS_MEMBER(bool, isDummyHandle, 0x48)
		};
