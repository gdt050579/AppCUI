target_sources(AppCUICore PRIVATE 
	BitmapLoader.cpp 
	Canvas.cpp 
	CharacterBuffer.cpp 
	Clip.cpp 
	CodePage.cpp
	Image.cpp 
	ProgressStatus.cpp 
	Rect.cpp 
	Renderer.cpp )

if (APPCUI_IMAGE_CODECS)
    target_sources(AppCUICore PRIVATE JPGLoader.cpp PNGLoader.cpp)
else()
    target_sources(AppCUICore PRIVATE ImageCodecsStub.cpp)
endif()
