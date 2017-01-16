solution "TD8_ARENA"
	configurations { "debug", "release" }
	platforms { "x64"}
	includedirs { ".", "src/", "./src/gkit", "./include", "/Shared/TP_VTDVR/LIRIS-VISION", "/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks", "/Locals/OpenCV-2.4.10/include", "/Locals/OpenCV-2.4.10/include/opencv", "/Locals/glfw/include", "/Locals/glm/include" }
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
		buildoptions { "lib/x64/libLeap.so -Wl,-rpath,lib/x64"}
		linkoptions  { "-flto"}
    linkoptions  { "/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build", "/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build", "/Locals/OpenCV-2.4.10/lib/", "/Locals/glfw/lib",  }
		links        { "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_ml", "opencv_video", "opencv_features2d", "opencv_calib3d", "opencv_objdetect", "opencv_contrib", "opencv_legacy", "opencv_flann", "apicamera", "cameraUVC", "cameraFILE", "cameraOPENCV", "calibration", "glfw3", "pthread", "X11", "Xrandr", "Xinerama", "Xi", "Xxf86vm", "Xcursor", "GLEW", "GL"}
    -- cette dernière ligne peut poser problème à cause des quotes:
    linkoptions  { "-Xlinker", "-rpath='/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build:/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build:'" }

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
	targetdir "bin/main"
	files ( gkit_files )
	files ( td8_arena_files )




