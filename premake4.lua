solution "TD8_ARENA"
	configurations { "debug", "release" }
	platforms { "x64"}
	includedirs { ".", "src/", "./src/gkit", "./include", "/usr/include/opencv/", "/usr/include/opencv2/", "export/include/" }
	project_dir = path.getabsolute(".")
	
	configuration "debug"
		targetdir "bin/debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "release"
		targetdir "bin/release"
		flags { "OptimizeSpeed" }

	configuration "linux"
		buildoptions { "-mtune=native -march=native" }
		buildoptions { "-std=c++11" }
		buildoptions { "-W -Wall -Wextra -Wsign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable", "-pipe" }
		buildoptions { "-flto"}
		linkoptions  { "lib/x64/libLeap.so -Wl,-rpath=lib/x64"}
		linkoptions  { "-flto"}
    	linkoptions  { "-Lexport/lib/" }
		links        { "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_ml", "opencv_video", "opencv_features2d", "opencv_calib3d", "opencv_objdetect", "opencv_contrib", "opencv_legacy", "opencv_flann", "apicamera", "cameraUVC", "cameraFILE", "cameraOPENCV", "calibration", "glfw", "pthread", "X11", "Xrandr", "Xinerama", "Xi", "Xxf86vm", "Xcursor", "GLEW", "GL"}
    
    	linkoptions  { "-Xlinker", "-rpath=export/lib/" }

	configuration { "linux", "debug" }
		buildoptions { "-g"}
		linkoptions { "-g"}
		
	configuration { "linux", "release" }
		buildoptions { "-fopenmp" }
		linkoptions { "-fopenmp" }	


 -- description des fichiers communs
gkit_files = { project_dir .. "/src/gkit/*.cpp", project_dir .. "/src/gkit/*.h" }
td8_arena_files = { project_dir .. "/src/*.cpp", project_dir .. "/src/*.h*" }

project("TD8_ARENA")
	language "C++"
	kind "ConsoleApp"
	targetdir "bin/"
	files ( gkit_files )
	files ( td8_arena_files )




