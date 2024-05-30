FILE(GLOB Sources "*.cpp")
FILE(GLOB Headers "*.h")


add_library(ImageLoader ${Sources} ${Headers} )
SET_PROPERTY(TARGET ImageLoader PROPERTY FOLDER "Libraries")